#include "Foxsimile.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

foxsimile::Responder::Responder(
    bool do_except, 
    std::map<std::vector<uint8_t>, std::vector<uint8_t>> new_response_lookup, 
    std::shared_ptr<SystemManager> new_system_manager, std::shared_ptr<CommandDeck> new_deck, 
    boost::asio::io_context& context
    ):  response_lookup(new_response_lookup),
        system_manager(new_system_manager),
        deck(new_deck),
        // acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), new_system_manager->system.ethernet->port)),
        acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(new_system_manager->system.ethernet->address), new_system_manager->system.ethernet->port)),
        socket(context)
{
    except_on_bad_request = do_except;
    default_response = {0x00};
    static_latency_us = 1000;
    bytewise_latency_us = 10;
    
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

    std::cout << system_manager->system.name << " received " << std::to_string(byte_count) << " bytes: ";
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
            std::vector<uint8_t> working_buffer(message_accumulator.begin(), message_accumulator.begin() + purported_length);

            message_accumulator.erase(message_accumulator.begin(), message_accumulator.begin() + purported_length);

            // trim ethernet header before handing to lookup?

            // send it out.
            send_response(get_response(working_buffer));
        }
    }

    // clear buffer
    read_buffer.resize(0);
    read_buffer.resize(2048);

    // listen again (either for more of this buffer, or to fill next buffer).
    async_receive();
}

void foxsimile::Responder::send_response(std::vector<uint8_t> response) {
    // delay to test timeout:
    size_t delay_ms = 10000;
    std::cout << "in " << std::to_string(delay_ms) << " ms, will send  response ";
    utilities::hex_print(response);
    std::cout << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    // send response
    socket.send(boost::asio::buffer(response));
}

void foxsimile::Responder::end_session() {

    // wait for a new connection
    socket.close();
    acceptor.accept(socket);
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
    std::vector<uint8_t> tail(message.begin() + eth_head_size - 1, message.end());
    size_t target_path_address_size = system_manager->system.spacewire->reply_path_address.size();
    std::vector<uint8_t> target_path_address(utilities::unsplat_from_4bytes(system_manager->system.spacewire->reply_path_address));

    // delete extra (padding to multiple of 4) nodes from the path address
    bool node_before = false;
    uint8_t first_node = 0x00;
    for (uint8_t i = 0; i < target_path_address_size; ++i) {
        if (target_path_address[i] != 0x00) {
            if (!node_before) {
                first_node = i;
            }
        }
    }
    target_path_address.erase(target_path_address.begin(), target_path_address.begin() + first_node);

    size_t reply_path_address_size = system_manager->system.spacewire->target_path_address.size();
    // round path address length to nearest multiple of 4
    if (reply_path_address_size % 4 != 0) {
        reply_path_address_size += 4 - (reply_path_address_size % 4);
    }

    uint8_t instruction = tail[target_path_address.size() + 2];
    std::cout << "received instruction: " << instruction << "\n";

    // uint8_t queried_logical_address = tail;
    // uint8_t queried_instruction = tail[3];

    // check system_manager->system->spacewire.reply_path_address.size() for index to start of message

// debug
    response = {0x01, 0x01, 0x01, 0x01};

    return response;
}
