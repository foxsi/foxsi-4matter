#include "Foxsimile.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>

foxsimile::Responder::Responder(
    bool do_except, 
    std::map<std::vector<uint8_t>, std::vector<uint8_t>> new_response_lookup, 
    std::shared_ptr<SystemManager> new_system_manager, 
    std::shared_ptr<CommandDeck> new_deck, 
    boost::asio::io_context& context
    ):  response_lookup(new_response_lookup),
        system_manager(new_system_manager),
        deck(new_deck),
        // acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), new_system_manager->system.ethernet->port)),
        acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(new_system_manager->system.ethernet->address), new_system_manager->system.ethernet->port)),
        socket(context)
{
    response_strategy = response_strategies::lookup;
    except_on_bad_request = do_except;
    default_response = {0x00};
    static_latency_us = 1000;
    bytewise_latency_us = 10;

    frame_update_ms = std::chrono::milliseconds(500);

    frame_timer = new boost::asio::steady_timer(context, frame_update_ms);
    
    lookup = true;

    // if (system_manager->system.spacewire == nullptr) {
    //     throw "can't construct Responder without System::spacewire!";
    // }
    if (!system_manager->system.ethernet) {
        throw "can't construct Responder without System::ethernet!";
    }
    boost::asio::ip::address local_address = boost::asio::ip::make_address(system_manager->system.ethernet->address);
    unsigned short local_port = system_manager->system.ethernet->port;

    read_buffer.resize(0);
    read_buffer.resize(2048);

    // acceptor.accept(socket);
    acceptor.async_accept(
        socket, 
        boost::bind(
            &Responder::handle_accept, 
            this,
            boost::asio::placeholders::error
        )
    );
}

foxsimile::Responder::Responder(
    bool do_except, 
    std::string new_response_mmap_file, 
    std::shared_ptr<SystemManager> new_system_manager, 
    std::shared_ptr<CommandDeck> new_deck, 
    boost::asio::io_context &context
):  response_mmap_file(new_response_mmap_file),
    system_manager(new_system_manager),
    deck(new_deck),
    // acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), new_system_manager->system.ethernet->port)),
    acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(new_system_manager->system.ethernet->address), new_system_manager->system.ethernet->port)),
    socket(context) 
{
    response_strategy = response_strategies::mmap;
    except_on_bad_request = do_except;
    default_response = {0x00};
    static_latency_us = 1000;
    bytewise_latency_us = 10;

    frame_update_ms = std::chrono::milliseconds(500);

    frame_timer = new boost::asio::steady_timer(context, frame_update_ms);
    frame_timer->async_wait(boost::bind(&Responder::handle_frame_timer, this));
    
    lookup = false;

    // if (system_manager->system.spacewire == nullptr) {
    //     throw "can't construct Responder without System::spacewire!";
    // }
    if (!system_manager->system.ethernet) {
        throw "can't construct Responder without System::ethernet!";
    }
    boost::asio::ip::address local_address = boost::asio::ip::make_address(system_manager->system.ethernet->address);
    unsigned short local_port = system_manager->system.ethernet->port;

    read_buffer.resize(0);
    read_buffer.resize(2048);

    // read in mmap:
    std::cout << "\topening large mmap file...\n";
    std::basic_ifstream<char> file(response_mmap_file, std::ios::binary | std::ios::in);
    std::vector<uint8_t> pass((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    mmap = pass;
    file.close();

    // acceptor.accept(socket);
    acceptor.async_accept(
        socket, 
        boost::bind(
            &Responder::handle_accept, 
            this,
            boost::asio::placeholders::error
        )
    );
}

void foxsimile::Responder::async_receive() {
    // async read
    // callback to get_response

    socket.async_read_some(
        boost::asio::buffer(read_buffer),
        boost::bind(
            &Responder::await_full_buffer, 
            this,
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
    ));
}

void foxsimile::Responder::handle_accept(const boost::system::error_code &err) {
    if (!err) {
        std::cout << system_manager->system.name << " accepted!\n";
        async_receive();
        // frame_timer->async_wait(boost::bind(&Responder::handle_frame_timer, this));
    } else {
        std::cout << system_manager->system.name << " accept error :(\n";
    }
}

void foxsimile::Responder::await_full_buffer(const boost::system::error_code &err, std::size_t byte_count) {

    if (
        err == boost::asio::error::eof 
        || err == boost::asio::error::connection_aborted 
        || err == boost::asio::error::connection_reset) {
        std::cout << system_manager->system.name << " connection reset by host\n";
        end_session();
        return;
    }

    std::cout << system_manager->system.name << " received " << std::to_string(byte_count) << " bytes: 0x";
    for (size_t i = 0; i < byte_count; ++i) {
        std::cout << std::hex << std::to_string(read_buffer[i]) << " ";
    }
    std::cout << "\n";

    std::cout << system_manager->system.name << " accumulater entry: ";
    utilities::hex_print(message_accumulator);

    read_buffer.resize(byte_count);

    // store read value in accumulator:
    size_t current_accumulator_index = message_accumulator.size();
    message_accumulator.insert(message_accumulator.end(), read_buffer.begin(), read_buffer.end());

    std::cout << system_manager->system.name << " accumulator fill: ";
    utilities::hex_print(message_accumulator);
    std::cout << system_manager->system.name << " accumulator[11]: ";
    utilities::hex_print(message_accumulator[11]);
    std::cout << "\n";

    // check we can read 12 bytes in:
    if (message_accumulator.size() >= 12) {
        // split into header and footer, convert footer to a uint
        std::vector<uint8_t> head(message_accumulator.begin() + 8, message_accumulator.begin() + 12);
        std::vector<uint8_t> foot(message_accumulator.begin() + 12, message_accumulator.end());
        size_t purported_length = utilities::unsplat_from_4bytes(head);

        std::cout << system_manager->system.name << " head: ";
        utilities::hex_print(head);
        std::cout<< system_manager->system.name << " tail: ";
        utilities::hex_print(foot);

        // check the reported size is in the buffer
        if (foot.size() >= purported_length) {
            std::cout << system_manager->system.name << " length purported to be " << std::to_string(purported_length) << "\n";
            
            // pop message from message_accumulator front.
            std::vector<uint8_t> working_buffer(message_accumulator.begin(), message_accumulator.begin() + 12 + purported_length);
            message_accumulator.erase(message_accumulator.begin(), message_accumulator.begin() + purported_length);

            // trim ethernet header before handing to lookup?

            // send it out.
            send_response(get_response(working_buffer));
        }
    } else {
        if (message_accumulator.size() == 2) {
            if (message_accumulator[0] != 0x00) {
                std::cout << "probably for housekeeping.\n";
                std::vector<uint8_t> working_buffer(message_accumulator.begin(), message_accumulator.end());
                send_response(get_response(working_buffer));
            }
        }
    }

    // clear buffer
    message_accumulator.resize(0);
    read_buffer.resize(0);
    read_buffer.resize(2048);

    // listen again (either for more of this buffer, or to fill next buffer).
    async_receive();
}

void foxsimile::Responder::send_response(std::vector<uint8_t> response) {
    if (response.size() > 0) {
        // delay to test timeout:
        size_t delay_ms = 2;
        // utilities::hex_print(response);
        std::cout << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        // send response
        socket.send(boost::asio::buffer(response));
    } else {
        std::cout << "response is empty!\n";
    }
}

void foxsimile::Responder::end_session() {

    // wait for a new connection
    socket.close();
    acceptor.async_accept(
        socket, 
        boost::bind(
            &Responder::handle_accept, 
            this,
            boost::asio::placeholders::error
        )
    );
}

std::vector<uint8_t> foxsimile::Responder::get_response(std::vector<uint8_t> message) {
    std::vector<uint8_t> response;

    try {
        if (lookup) {
            response = response_lookup.at(message);
        } else {
            response = make_reply(message);
        }
    } catch(std::exception& e) {
        utilities::error_print("response threw exception " + std::string(e.what()) + "\n");
        if (except_on_bad_request) {
            throw e;
        }
    }
    return response;
}

bool foxsimile::Responder::add_response(std::vector<uint8_t> message, std::vector<uint8_t> response) {
    return response_lookup.insert(std::make_pair(message,response)).second;
}

void foxsimile::Responder::add_default_response(std::vector<uint8_t> message) {
    default_response = message;
}

void foxsimile::Responder::set_bad_request_behavior(bool do_except) {
    except_on_bad_request = do_except;
}

void foxsimile::Responder::handle_frame_timer() {
    try {
        for (uint8_t i = 0; i < 4; ++i) {
            std::string this_name = "cdte" + std::to_string(i + 1);
            System& this_system = deck->get_sys_for_name(this_name);
            foxsimile::mmap::advance_write_pointer(this_system, RING_BUFFER_TYPE_OPTIONS::PC, mmap);
        }
    } catch (std::exception& e) {
        utilities::error_print("timer caught: " + std::string(e.what()) + "\n");

    }

    frame_timer->expires_at(frame_timer->expiry() + frame_update_ms);
    frame_timer->async_wait(boost::bind(&Responder::handle_frame_timer, this));

}

std::vector<uint8_t> foxsimile::Responder::make_reply(std::vector<uint8_t> message) {
    std::vector<uint8_t> response;
    size_t eth_head_size = 12;

    if (message[0] != 0) {
        throw "can't foxsimile::Responder::make_reply(): message starts wrong";
    }
    
    size_t message_size = message.size();
    if (message_size < eth_head_size) {
        throw "can't foxsimile::Responder::make_reply(): message size way too short";
    }

    std::vector<uint8_t> tail_size_bytes(message.begin() + 7, message.begin() + eth_head_size - 1);
    size_t tail_size = utilities::unsplat_from_4bytes(tail_size_bytes);

    if (message_size - eth_head_size < tail_size) {
        throw "can't foxsimile::Responder::make_reply(): message too short";
    }

    std::cout << "trying parse\n";

    // in the this system's reference frame (for target and initiator)
    std::vector<uint8_t> tail(message.begin() + eth_head_size, message.end());
    
    std::cout << "parsing tail: ";
    utilities::hex_print(tail);
    std::cout << "\n";

    std::cout << "reply path address: ";
    utilities::hex_print(system_manager->system.spacewire->reply_path_address);
    std::cout << "\n";

    size_t target_path_address_size = system_manager->system.spacewire->reply_path_address.size();
    // std::vector<uint8_t> target_path_address(utilities::unsplat_from_4bytes(system_manager->system.spacewire->reply_path_address));
    std::vector<uint8_t> target_path_address(system_manager->system.spacewire->reply_path_address);

    // delete extra (padding to multiple of 4) nodes from the path address
    uint8_t first_node = 0;
    for (uint8_t i = 0; i < target_path_address_size; ++i) {
        // std::cout << "\ttarget_path_address[" << std::to_string(i) << "] = " << std::to_string(target_path_address[i]) << "\n";
        if (target_path_address.at(i) != 0x00) {
            first_node = i;
            break;
        }
    }
    target_path_address.erase(target_path_address.begin(), target_path_address.begin() + first_node);

    size_t instruction_offset_from_start = 2;
    size_t key_offset_from_start = 3;
    size_t reply_path_offset_from_start = 4;
    size_t initiator_logical_address_offset_from_reply_address = 0;
    size_t transaction_id_offset_from_reply_address = 1;
    size_t extended_address_offset_from_reply_address = 3;
    size_t address_offset_from_reply_address = 4;
    size_t data_length_offset_from_reply_address = 8;
    size_t data_offset_from_reply_address = 12;

    uint8_t instruction = tail.at(target_path_address.size() + instruction_offset_from_start);
    uint8_t key = tail.at(target_path_address.size() + key_offset_from_start);
    std::cout << "received instruction: " << std::to_string(instruction) << "\n";

    // checking instruction for how to respond:
    bool do_write = instruction & 0x20;    // check RMAP write (1) or read (0).
    bool do_reply = instruction & 0x08;
    bool do_verify = instruction & 0x10;
    uint8_t instruction_reply_size = 4*(instruction & 0x03);

    std::vector<uint8_t> explicit_reply_path_address(
        tail.begin() + reply_path_offset_from_start + target_path_address.size(), 
        tail.begin() + reply_path_offset_from_start + target_path_address.size() + instruction_reply_size
    );
    std::vector<uint8_t> reply_path_address(explicit_reply_path_address);

    // delete extra (padding to multiple of 4) nodes from the path address for response
    uint8_t reply_first_node = 0;
    for (uint8_t i = 0; i < reply_path_address.size(); ++i) {
        if (reply_path_address.at(i) != 0x00) {
            reply_first_node = i;
            break;
        }
    }
    reply_path_address.erase(reply_path_address.begin(), reply_path_address.begin() + reply_first_node);

    std::cout << "inferred target_path_address: ";
    utilities::hex_print(target_path_address);
    std::cout << "\n";
    std::cout << "inferred reply_path_address: ";
    utilities::hex_print(reply_path_address);
    std::cout << "\n";

    // extract other info from message
    uint8_t initiator_logical_address = tail.at(target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + initiator_logical_address_offset_from_reply_address);
    std::vector<uint8_t> transaction_id(
        tail.begin() + target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + transaction_id_offset_from_reply_address,
        tail.begin() + target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + transaction_id_offset_from_reply_address + 1
    );
    uint8_t extended_address = tail.at(target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + extended_address_offset_from_reply_address);
    std::vector<uint8_t> address_bytes(
        tail.begin() + target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + address_offset_from_reply_address,
        tail.begin() + target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + address_offset_from_reply_address + 4
    );
    std::vector<uint8_t> data_length_bytes(
        tail.begin() + target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + data_length_offset_from_reply_address,
        tail.begin() + target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + data_length_offset_from_reply_address + 3
    );

    size_t data_length = (data_length_bytes[0] << 16) & 0xff0000
                        | (data_length_bytes[1] << 8) & 0x00ff00
                        | (data_length_bytes[2] << 0) & 0x0000ff;
    size_t address = (address_bytes[0] << 24) & 0xff00'0000
                   | (address_bytes[1] << 16) & 0x00ff'0000
                   | (address_bytes[2] << 8)  & 0x0000'ff00
                   | (address_bytes[3] << 0)  & 0x0000'00ff;

    std::cout << "initiator logical address: " << std::to_string(initiator_logical_address) << "\n";
    std::cout << "requesting access to " << std::to_string(data_length) << " bytes from address ";
    utilities::hex_print(address_bytes);

    // now, access memory at the given `address`
    RING_BUFFER_TYPE_OPTIONS type = foxsimile::mmap::is_ring_buffer(*system_manager, address, data_length);
    std::cout << "found ring buffer type: " << RING_BUFFER_TYPE_OPTIONS_NAMES.at(type) << "\n";

    if (do_write) {
        std::cout << "got write command\n";
        // extract data to write:
        std::vector<uint8_t> data_bytes(
            tail.begin() + target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + data_offset_from_reply_address,
            tail.begin() + target_path_address.size() + reply_path_offset_from_start + instruction_reply_size + data_offset_from_reply_address + data_length
        );
        if (do_verify) {
            // todo: implement
            utilities::debug_print("verification not implemented!\n");
        }

        // write to mmap:
        for (size_t i = 0; i < data_length; ++i) {
            mmap.at(i + address) = data_bytes.at(i);
        }
        std::cout << "wrote data\n";
        if (do_reply) {
            utilities::error_print("don't know reply format!\n");
            return {};
        }
    } else {
        if (!do_reply) {
            utilities::error_print("got read command without reply!\n");
            return {};
        }
        std::cout << "got read command\n";

        // query mmap:
        std::vector<uint8_t> read_data(mmap.begin() + address, mmap.begin() + address + data_length);
        std::cout << "read data from memory\n";
        
        // build reply packet
        std::vector<uint8_t> ether_header;
        std::vector<uint8_t> spw_tailer;
        std::vector<uint8_t> packet;

        spw_tailer.push_back(initiator_logical_address);
        spw_tailer.push_back(0x01);     // RMAP
        spw_tailer.push_back((instruction & 0x03) | 0x08);
        spw_tailer.push_back(0x00);     // placeholder for reply status code, see ECSS-E-SD-50-52C5 section 5.6
        spw_tailer.push_back(system_manager->system.spacewire->source_logical_address);
        spw_tailer.push_back(transaction_id[0]);
        spw_tailer.push_back(transaction_id[1]);
        spw_tailer.push_back(0x00);
        spw_tailer.insert(spw_tailer.end(), data_length_bytes.begin(), data_length_bytes.end());
        uint8_t tailer_crc = system_manager->system.spacewire->crc(spw_tailer);
        spw_tailer.push_back(tailer_crc);
        uint8_t data_crc = system_manager->system.spacewire->crc(read_data);
        spw_tailer.insert(spw_tailer.end(), read_data.begin(), read_data.end());
        spw_tailer.push_back(data_crc);
        packet.insert(packet.end(), spw_tailer.begin(), spw_tailer.end());

        std::vector<uint8_t> spw_size_bytes = utilities::splat_to_nbytes(4, packet.size());
        ether_header.resize(8);
        ether_header.insert(ether_header.end(), spw_size_bytes.begin(), spw_size_bytes.end());
        packet.insert(packet.begin(), ether_header.begin(), ether_header.end());

        std::cout << "sending response: ";
        utilities::spw_print(packet, nullptr);
        std::cout << "\n";

        return packet;
    }

    // checklist: 
    //      target logical address
    //      key
    //      protocol id
    // verifylist:
    //      header and data crcs

// debug
    response = {0x01, 0x01, 0x01, 0x01};

    // todo: move [] access to .at(), and wrap in try/except
    return response;
}

RING_BUFFER_TYPE_OPTIONS foxsimile::mmap::is_ring_buffer(SystemManager& sys_man, size_t address, size_t offset) {
    // assumes convex (contiguous) ring buffer area.
    size_t first = address;
    size_t last = address + offset;

    if (sys_man.system.name.find("cdte1") != std::string::npos) {
        if (first >= 0x0040'0000 && last < 0x022a'2df0) {
            return RING_BUFFER_TYPE_OPTIONS::PC;
        }
    } else if (sys_man.system.name.find("cdte2") != std::string::npos) {
        if (first >= 0x0230'0000 && last < 0x041a'2df0) {
            return RING_BUFFER_TYPE_OPTIONS::PC;
        }
    } else if (sys_man.system.name.find("cdte3") != std::string::npos) {
        if (first >= 0x0420'0000 && last < 0x060a'2df0) {
            return RING_BUFFER_TYPE_OPTIONS::PC;
        }
    } else if (sys_man.system.name.find("cdte4") != std::string::npos) {
        if (first >= 0x0610'0000 && last < 0x07fa'2df0) {
            return RING_BUFFER_TYPE_OPTIONS::PC;
        }
    } else if (sys_man.system.name.find("cmos") != std::string::npos) {
        if (first >= 0x1000 && last < 0x0440'1000) {
            return RING_BUFFER_TYPE_OPTIONS::QL;
        }
        if (first >= 0x0440'1000 && last < 0x07a0'1000) {
            return RING_BUFFER_TYPE_OPTIONS::PC;
        }
    }
    return RING_BUFFER_TYPE_OPTIONS::NONE;
}

void foxsimile::mmap::advance_write_pointer(System& system, RING_BUFFER_TYPE_OPTIONS type, std::vector<uint8_t>& mmap) {
    // check this is system with a ring buffer, and set stride accordingly.
    
    uint32_t stride = 0;
    uint32_t mod_size = 0;
    uint32_t start = 0;
    if (system.name.find("cdte") != std::string::npos) {
        stride = system.ring_params.at(type).frame_size_bytes;
        start = system.ring_params.at(type).start_address;
        mod_size = 980*stride; // 0x01ea'2df0
    } else if (system.name.find("cmos") != std::string::npos) {
        if (type == RING_BUFFER_TYPE_OPTIONS::PC) {
            stride = 0x1b'0000;
            start = system.ring_params.at(type).start_address;
            mod_size = 32*stride;
        } else if (type == RING_BUFFER_TYPE_OPTIONS ::QL) {
            stride = 0x22'0000;
            start = system.ring_params.at(type).start_address;
            mod_size = 32*stride;
        }
    } else {
        throw "Trying to advance write pointer for wrong system!";
    }

    size_t write_pointer_size = system.ring_params[type].write_pointer_width_bytes;
    uint32_t current_write_pointer_start = system.ring_params[type].write_pointer_address;
    std::vector<uint8_t> current_write_pointer_bytes(mmap.begin() + current_write_pointer_start, mmap.begin() + current_write_pointer_start + write_pointer_size);
    
    uint32_t current_write_pointer = utilities::unsplat_from_4bytes(current_write_pointer_bytes);
    uint32_t new_write_pointer = stride + current_write_pointer;
    if (new_write_pointer > mod_size + start) {
        new_write_pointer = start;
    }
    std::vector<uint8_t> new_write_pointer_bytes = utilities::splat_to_nbytes(4, new_write_pointer);
    for (size_t i = 0; i < write_pointer_size; ++i) {
        mmap.at(i + current_write_pointer_start) = new_write_pointer_bytes.at(i);
    }
}

void foxsimile::mmap::advance_write_pointer(SystemManager& sys_man, RING_BUFFER_TYPE_OPTIONS type, std::vector<uint8_t>& mmap) {

    foxsimile::mmap::advance_write_pointer(sys_man.system, type, mmap);
}
