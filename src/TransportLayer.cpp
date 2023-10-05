#include "TransportLayer.h"
#include "Utilities.h"

#include <boost/bind.hpp>   // boost::bind (for async handler to class members)
#include <algorithm>        // std::fill
#include <iomanip>

// construct from Parameters.h macro constants
TransportLayerMachine::TransportLayerMachine(
    boost::asio::io_context& context
):  
    local_udp_sock(context), 
    local_tcp_sock(context)
{
    active_subsys = SUBSYSTEM_ORDER::HOUSEKEEPING;
    active_state = STATE_ORDER::IDLE;

    commands = CommandDeck();
    
    std::vector<uint8_t> tmp_vec(config::buffer::RECV_BUFF_LEN, '\0');
    downlink_buff = tmp_vec;
    uplink_buff = tmp_vec;

    std::vector<uint8_t> tmp_cmd = {};
    command_pipe = tmp_cmd;
    ground_pipe.push('0');

    boost::asio::ip::address local_addr = boost::asio::ip::make_address(config::ethernet::LOCAL_IP);
    boost::asio::ip::address remote_udp_addr = boost::asio::ip::make_address(config::ethernet::GSE_IP);
    boost::asio::ip::address remote_tcp_addr = boost::asio::ip::make_address(config::ethernet::SPMU_IP);
    boost::asio::ip::udp::endpoint local_udp_endpoint(local_addr, config::ethernet::LOCAL_PORT);
    boost::asio::ip::tcp::endpoint local_tcp_endpoint(local_addr, config::ethernet::LOCAL_PORT);

    remote_udp_endpoint = boost::asio::ip::udp::endpoint(remote_udp_addr, config::ethernet::GSE_PORT);
    remote_tcp_endpoint = boost::asio::ip::tcp::endpoint(remote_tcp_addr, config::ethernet::SPMU_PORT);

    local_udp_sock.open(boost::asio::ip::udp::v4());
    local_udp_sock.bind(local_udp_endpoint);

    local_tcp_sock.open(boost::asio::ip::tcp::v4());
    local_tcp_sock.bind(local_tcp_endpoint);
    local_tcp_sock.connect(remote_tcp_endpoint);
}

// construct from IP address:port pairs
TransportLayerMachine::TransportLayerMachine(
    std::string local_ip,
    std::string remote_tcp_ip,
    std::string remote_udp_ip,
    unsigned short local_port,
    unsigned short remote_tcp_port,
    unsigned short remote_udp_port,
    boost::asio::io_context& context
): 
    local_udp_sock(context),
    local_tcp_sock(context)
{
    active_subsys = SUBSYSTEM_ORDER::HOUSEKEEPING;
    active_state = STATE_ORDER::IDLE;

    commands = CommandDeck();
    
    std::vector<uint8_t> tmp_vec(config::buffer::RECV_BUFF_LEN, '\0');
    downlink_buff = tmp_vec;
    uplink_buff = tmp_vec;
    
    std::vector<uint8_t> tmp_cmd = {};
    command_pipe = tmp_cmd;

    boost::asio::ip::address local_addr = boost::asio::ip::make_address(local_ip);
    boost::asio::ip::address remote_udp_addr = boost::asio::ip::make_address(remote_udp_ip);
    boost::asio::ip::address remote_tcp_addr = boost::asio::ip::make_address(remote_tcp_ip);
    
    boost::asio::ip::udp::endpoint local_udp_endpoint(local_addr, local_port);
    boost::asio::ip::tcp::endpoint local_tcp_endpoint(local_addr, local_port);

    remote_udp_endpoint = boost::asio::ip::udp::endpoint(remote_udp_addr, remote_udp_port);
    remote_tcp_endpoint = boost::asio::ip::tcp::endpoint(remote_tcp_addr, remote_tcp_port);

    local_udp_sock.open(boost::asio::ip::udp::v4());
    local_udp_sock.bind(local_udp_endpoint);

    local_tcp_sock.open(boost::asio::ip::tcp::v4());
    local_tcp_sock.bind(local_tcp_endpoint);
    local_tcp_sock.connect(remote_tcp_endpoint);
}


// construct from existing boost asio endpoints
TransportLayerMachine::TransportLayerMachine(
    boost::asio::ip::udp::endpoint local_udp_end,
    boost::asio::ip::tcp::endpoint local_tcp_end,
    boost::asio::ip::udp::endpoint remote_udp_end,
    boost::asio::ip::tcp::endpoint remote_tcp_end,
    boost::asio::io_context& context
): 
    local_udp_sock(context),
    local_tcp_sock(context)
{
    remote_udp_endpoint = remote_udp_end;
    remote_tcp_endpoint = remote_tcp_end;
    
    active_subsys = SUBSYSTEM_ORDER::HOUSEKEEPING;
    active_state = STATE_ORDER::IDLE;
    
    commands = CommandDeck();
    
    std::vector<uint8_t> tmp_vec(config::buffer::RECV_BUFF_LEN, '\0');
    downlink_buff = tmp_vec;
    uplink_buff = tmp_vec;
    
    std::vector<uint8_t> tmp_cmd = {};
    command_pipe = tmp_cmd;

    local_udp_sock.open(boost::asio::ip::udp::v4());
    local_udp_sock.bind(local_udp_end);

    local_tcp_sock.open(boost::asio::ip::tcp::v4());
    local_tcp_sock.bind(local_tcp_end);
    local_tcp_sock.connect(remote_tcp_endpoint);
}

void TransportLayerMachine::add_commands(CommandDeck& new_commands) {
    commands = new_commands;
}

void TransportLayerMachine::add_ring_buffer_interface(std::unordered_map<uint8_t, RingBufferInterface> new_ring_buffers) {
    ring_buffers = new_ring_buffers;
}

void TransportLayerMachine::add_fragmenter(Fragmenter new_fragmenter) {
    fragmenter = new_fragmenter;
}

void TransportLayerMachine::add_fragmenter(size_t fragment_size, size_t header_size) {
    fragmenter = Fragmenter(fragment_size, header_size);
}

/* -- utility -------------------------------------------- */

bool TransportLayerMachine::check_frame_read_cmd(uint8_t sys, uint8_t cmd) {
    if(sys == 0x0e) {                       // cmos1
        if(cmd == static_cast<uint8_t>(RING_READ_CMD::CMOS_1) || cmd == 0x9f) {
            return true;
        }
    } else if(sys == 0x0f) {                // cmos2
        if(cmd == static_cast<uint8_t>(RING_READ_CMD::CMOS_2) || cmd == 0x9f) {
            return true;
        }
    } else if(sys == 0x09) {                // cdte1
        if(cmd == static_cast<uint8_t>(RING_READ_CMD::CDTE_1)) {
            return true;
        }
    } else if(sys == 0x0a) {                // cdte2
        if(cmd == static_cast<uint8_t>(RING_READ_CMD::CDTE_2)) {
            return true;
        }
    } else if(sys == 0x0b) {                // cdte3
        if(cmd == static_cast<uint8_t>(RING_READ_CMD::CDTE_3)) {
            return true;
        }
    } else if(sys == 0x0c) {                // cdte4
        if(cmd == static_cast<uint8_t>(RING_READ_CMD::CDTE_4)) {
            return true;
        }
    } else {
        utilities::debug_print("checking for frame read of an unimplemented system!\n");
    }
    return false;
}

/* -- network I/F ---------------------------------------- */

void TransportLayerMachine::recv_tcp_fwd_udp() {
    std::cout << "in recv_tcp_fwd_udp()\n";

    // read incoming TCP data...
    local_tcp_sock.async_read_some(
        boost::asio::buffer(downlink_buff),         // read data into the downlink buffer
        boost::bind(
            &TransportLayerMachine::send_udp, 
            this,
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
        )    // callback to send_udp to forward the data after it has been received
    );
}

void TransportLayerMachine::recv_udp_fwd_tcp() {
    std::cout << "in recv_udp_fwd_tcp()\n";

    // read incoming UDP data...
    local_udp_sock.async_receive_from(
        boost::asio::buffer(uplink_buff),           // read data into the uplink buffer
        remote_udp_endpoint,                        // receive data from the UDP endpoint
        boost::bind(&TransportLayerMachine::send_tcp, this)    // callback to send_tcp to forward the data after it has been received
    );
}

void TransportLayerMachine::send_udp(const boost::system::error_code& err, std::size_t byte_count) {
    std::cout << "in send_udp\n";

    // std::vector<uint8_t> filtered = downlink_buff;
    std::vector<uint8_t> filtered(downlink_buff.begin(), downlink_buff.begin() + byte_count);
    // std::vector<uint8_t> filtered;
    // std::copy_if(downlink_buff.begin(), downlink_buff.end(), std::back_inserter(filtered), [](uint8_t i){return i != 0x00;});
    std::cout << "received " << byte_count << " bytes:\n";
    utilities::hex_print(filtered);
    std::cout << "\n";

    utilities::debug_print("trying spw data isolation: ");
    // todo: REPLACE HARDCODED 0x08 with lookup CdTe DE
    std::vector<uint8_t> reply_data = TransportLayerMachine::get_reply_data(downlink_buff, 0x08);
    utilities::hex_print(reply_data);

    // fragment the filtered buffer
    // prepend <sys> code to the buffer
    // send on UDP.
    // forward the buffer downlink_buff over UDP...
    local_udp_sock.async_send_to(
        boost::asio::buffer(downlink_buff),                 // send out the contents of downlink_buff
        remote_udp_endpoint,                                // send to the UDP endpoint
        boost::bind(&TransportLayerMachine::recv_tcp_fwd_udp, this)    // callback to recv_tcp_fwd_udp after send to continue listening
    );

    // clear the downlink buffer
    std::fill(downlink_buff.begin(), downlink_buff.end(), '\0');
}

void TransportLayerMachine::send_tcp() {
    std::cout << "in send_tcp\n";

    // forward the buffer uplink_buff over TCP...
    local_tcp_sock.async_send(
        boost::asio::buffer(uplink_buff),                   // send out the contents of uplink_buff
        boost::bind(&TransportLayerMachine::recv_udp_fwd_tcp, this)    // callback to recv_udp_fwd_tcp after send to continue listening
    );

    // clear the buffer uplink_buff...
    std::fill(uplink_buff.begin(), uplink_buff.end(), '\0');
}

void TransportLayerMachine::recv_udp_fwd_tcp_cmd() {
    std::cout << "in recv_udp_fwd_tcp_cmd()\n";

    // read incoming UDP data...
    local_udp_sock.async_receive_from(
        boost::asio::buffer(uplink_buff),           // read data into the uplink buffer
        remote_udp_endpoint,                        // receive data from the UDP endpoint
        boost::bind(&TransportLayerMachine::handle_cmd, this)    // callback to send_tcp to forward the data after it has been received
    );
}

void TransportLayerMachine::handle_cmd() {
    // todo: save pointers in these buffers (cmd and uplink) to manage multiple sequential reads/writes safely. Maybe with vector of vectors
    std::cout << "in handle_cmd()\n";

    uint8_t uplink_buff_sys = uplink_buff[0];
    uint8_t uplink_buff_cmd = uplink_buff[1];

    // update
    bool is_frame_read_cmd = check_frame_read_cmd(uplink_buff_sys, uplink_buff_cmd);

    std::cout << "sys: \t" << std::hex << (int)uplink_buff_sys << "\n";
    std::cout << "cmd: \t" << std::hex << (int)uplink_buff_cmd << "\n";

    std::vector<uint8_t> output_cmd = commands.get_command_bytes_for_sys_for_code(uplink_buff_sys, uplink_buff_cmd);

    command_pipe.insert(command_pipe.end(), output_cmd.begin(), output_cmd.end());
    // command_pipe.push_back('\0');

    std::cout << "transmitting:\t";
    utilities::hex_print(output_cmd);
    std::cout << "to " << local_tcp_sock.remote_endpoint().address().to_string() << ":" << std::to_string(local_tcp_sock.remote_endpoint().port()) << "\n";

    if(is_frame_read_cmd) {
        std::cout << "\tfound frame read command, handling transaction\n";
        local_tcp_sock.send(boost::asio::buffer(output_cmd));

        std::vector<uint8_t> reply;
        reply.resize(4096);
        size_t reply_len = local_tcp_sock.read_some(boost::asio::buffer(reply));

        // wrap the reply vector to the correct length
        reply.resize(reply_len);
        
        std::cout << "got reply of length 0x" << reply.size() << " with reported size 0x" << reply_len << ":\t";
        utilities::hex_print(reply);
        std::cout << "\n";

        if(reply_len < 1) {
            std::cout << "\tgot no response from read command.\n";
            return;
        }

        // todo: verify reply length > 0 before proceeding to avoid index outside the vector.
        
        std::vector<uint8_t> remote_wr_ptr = TransportLayerMachine::get_reply_data(reply, uplink_buff_sys);
        if(remote_wr_ptr.size() != 4) {
            utilities::error_print("got bad write pointer length!\n");
        }

        // todo: REPLACE HARDCODED:
        if(uplink_buff_sys == 0x0f || uplink_buff_sys == 0x0e) {
            remote_wr_ptr = utilities::swap_endian4(remote_wr_ptr);
        }

        // todo: handle this in a sustainable way
        std::vector<uint32_t> spw_data;
        spw_data.push_back(utilities::unsplat_from_4bytes(remote_wr_ptr));
        spw_data.push_back(ring_buffers[uplink_buff_sys].get_block_size());
        spw_data.push_back(0x00);

        // buffer to hold replies:
        std::vector<uint8_t> reply_stack;

        size_t blocks = ring_buffers[uplink_buff_sys].get_read_count_blocks();
        size_t block_size = ring_buffers[uplink_buff_sys].get_block_size();

        utilities::debug_print("\t\tgot " + std::to_string(blocks) + " blocks of size " + std::to_string(block_size) + "\n");

        // check if ring buffer wraps around (2nd piece of buffer is zero-length):
        if(spw_data[3] == 0) {
            // send only one command.
            utilities::debug_print("\treading from non-wrapped ring buffer region.\n");
            utilities::debug_print("\tspw_data[0]: " + std::to_string(spw_data[0]));

            for(size_t i = 0; i < blocks; ++i) {
                utilities::debug_print("\t reading block " + std::to_string(i) + " of " + std::to_string(blocks - 1) + "\n");

                // advance the read address
                std::vector<uint8_t> ring_read_addr = utilities::splat_to_nbytes(4, spw_data[0] + i*block_size);

                // build a new read command from template
                size_t ring_len = spw_data[1];
                std::vector<uint8_t> ring_read_cmd = commands.get_read_command_from_template(uplink_buff_sys, uplink_buff_cmd, ring_read_addr, ring_len);

                utilities::debug_print("\treading from address " + std::to_string(spw_data[0] + i*block_size) + "\n");

                // send the read command
                utilities::debug_print("\tsending read command: \n");
                utilities::hex_print(ring_read_cmd);
                local_tcp_sock.send(boost::asio::buffer(ring_read_cmd));
                
                utilities::debug_print("\twaiting for reply\n");
                // get the response
                std::vector<uint8_t> last_reply_packet;
                last_reply_packet.resize(4096);
                size_t reply_len = local_tcp_sock.read_some(boost::asio::buffer(last_reply_packet));
                utilities::debug_print("\tgot reply of length " + std::to_string(reply_len) + "\n");
                utilities::debug_print("\treply:\n");
                utilities::hex_print(last_reply_packet);

                // extract data field from reply
                std::vector<uint8_t> last_reply_data;
                if(i == 0) {
                    // SPMU-001 doesn't seem to prepend any header for subsequent packets? So just strip off SpW and Ether headers for the first packet.
                    last_reply_data = TransportLayerMachine::get_reply_data(last_reply_packet, uplink_buff_sys);
                } else {
                    last_reply_data = last_reply_packet;
                }

                utilities::debug_print("\tadding to reply stack\n");
                // append new data to buffer
                reply_stack.insert(reply_stack.begin(), last_reply_data.begin(), last_reply_data.end());
            }

            utilities::debug_print("\nHERE IS THE RING BUFFER OUTPUT:\n");
            utilities::hex_print(reply_stack);

            TransportLayerMachine::recv_udp_fwd_tcp_cmd();

        } else {
            // this is copy-paste of the above case. todo: use your brain. write same stuff as a separate method. differentiate if its needed.

            // send two read commands.
            utilities::debug_print("\treading from wrapped ring buffer region.\n");
            utilities::error_print("\tI haven't really implemented this yet :{}\n");
            utilities::debug_print("\tspw_data[0]: " + std::to_string(spw_data[0]));

            /********************** JUST PASTED FROM ABOVE **********************/
            for(size_t i = 0; i < blocks; ++i) {
                utilities::debug_print("\treading block " + std::to_string(i) + " of " + std::to_string(blocks - 1) + "\n");

                // advance the read address
                std::vector<uint8_t> ring_read_addr = utilities::splat_to_nbytes(4, spw_data[0] + i*block_size);

                // build a new read command from template
                size_t ring_len = spw_data[1];
                std::vector<uint8_t> ring_read_cmd = commands.get_read_command_from_template(uplink_buff_sys, uplink_buff_cmd, ring_read_addr, ring_len);

                utilities::debug_print("\treading from address " + std::to_string(spw_data[0] + i*block_size) + "\n");

                // send the read command
                utilities::debug_print("\tsending read command: \n");
                utilities::hex_print(ring_read_cmd);
                local_tcp_sock.send(boost::asio::buffer(ring_read_cmd));
                
                utilities::debug_print("\twaiting for reply\n");
                // get the response
                std::vector<uint8_t> last_reply_packet;
                size_t reply_len = local_tcp_sock.read_some(boost::asio::buffer(last_reply_packet));
                utilities::debug_print("\tgot reply of length " + std::to_string(reply_len) + "\n");

                // extract data field from reply
                std::vector<uint8_t> last_reply_data = TransportLayerMachine::get_reply_data(last_reply_packet, uplink_buff_sys);

                // append new data to buffer
                reply_stack.insert(reply_stack.begin(), last_reply_data.begin(), last_reply_data.end());
            }

            utilities::debug_print("\nHERE IS THE RING BUFFER OUTPUT:\n");
            utilities::hex_print(reply_stack);

            TransportLayerMachine::recv_udp_fwd_tcp_cmd();
        }

    } else {
        std::cout << "\tfound non-frame read command, sending\n";
        local_tcp_sock.async_send(
            boost::asio::buffer(output_cmd),                   // send out the contents of uplink_buff
            boost::bind(&TransportLayerMachine::recv_udp_fwd_tcp_cmd, this)    // callback to recv_udp_fwd_tcp after send to continue listening
        );
    }

    // clear the buffer uplink_buff...
    std::fill(uplink_buff.begin(), uplink_buff.end(), '\0');
    // clear the buffer command_pipe...
    std::fill(command_pipe.begin(), command_pipe.end(), '\0');
}

void TransportLayerMachine::handle_remote_buffer_transaction() {
    std::cout << "in handle_remote_buffer_transaction()\n";
    uint8_t uplink_buff_sys = uplink_buff[0];
    uint8_t uplink_buff_cmd = uplink_buff[1];

    std::cout << "sys: \t" << std::hex << (int)uplink_buff_sys << "\n";
    std::cout << "cmd: \t" << std::hex << (int)uplink_buff_cmd << "\n";

    std::vector<uint8_t> output_cmd = commands.get_command_bytes_for_sys_for_code(uplink_buff_sys, uplink_buff_cmd);

    // read the last write pointer
    local_tcp_sock.send(boost::asio::buffer(output_cmd));
}

std::vector<uint8_t> TransportLayerMachine::get_reply_data(std::vector<uint8_t> spw_reply, System& sys) {

    size_t ether_prefix_length = 12; // using SPMU-001, this is always true
    size_t target_path_address_length = 0; // path address is removed by the time we receive reply.

    // size_t target_path_address_length = sys.target_path_address.size() - 1;
    /**
     * 1B initiator logical address
     * 1B protocol id
     * 1B instruction
     * 1B status
     * 1B target logical address
     * 2B transaction id
     * 1B reserved
     * 3B data length
     * 1B header CRC
     */

    // now extract data based on data length field
    size_t data_length_start_offset = 9;
    size_t data_length_length = 3;

    utilities::debug_print("\tlast header access: " + std::to_string(ether_prefix_length + target_path_address_length + data_length_start_offset + data_length_length) + "\n");

    std::vector<uint8_t> data_length_vec(
        spw_reply.begin() 
            + ether_prefix_length 
            + target_path_address_length 
            + data_length_start_offset
            - 1, 
        spw_reply.begin() 
            + ether_prefix_length
            + target_path_address_length
            + data_length_start_offset
            + data_length_length
            - 1
    );

    // pre-pad `data_length_vec` with zero to use with `unsplat_from_4bytes()`
    std::vector<uint8_t> zero_prefix(1); 
    zero_prefix[0] = 0x00;
    data_length_vec.insert(data_length_vec.begin(), zero_prefix.begin(), zero_prefix.end());

    utilities::debug_print("\tvector data length field result: \n");
    for(auto& el: data_length_vec) {
        utilities::debug_print("\t\t" + std::to_string(el) + "\n");
    }

    uint32_t data_length = utilities::unsplat_from_4bytes(data_length_vec);
    
    utilities::debug_print("\n\tconverted data length field result: ");
    utilities::debug_print("\n\t\t" + std::to_string(data_length) + "\n");

    // now read rest of spw_reply based on data_length
    std::vector<uint8_t> reply_data(
        spw_reply.begin()
            + ether_prefix_length
            + target_path_address_length
            + data_length_start_offset
            + data_length_length,
        spw_reply.begin()
            + ether_prefix_length
            + target_path_address_length
            + data_length_start_offset
            + data_length_length
            + data_length
    );

    return reply_data;
}

std::vector<uint8_t> TransportLayerMachine::get_reply_data(std::vector<uint8_t> spw_reply, uint8_t sys) {
    System& sys_obj = commands.get_sys_for_code(sys);
    
    return TransportLayerMachine::get_reply_data(spw_reply, sys_obj);
}

void TransportLayerMachine::print_udp_basic() {
    std::cout << "in print_udp_basic()\n";

    std::cout << "connected to " << local_udp_sock.local_endpoint().address().to_string() << ":" << std::to_string(local_udp_sock.local_endpoint().port()) << "\n";

    char local_buffer[1000];

    local_udp_sock.receive_from(
        boost::asio::buffer(local_buffer),
        remote_udp_endpoint
    );

    std::string recv_txt(local_buffer);
    std::cout << "received " << recv_txt <<"\n";

    local_buffer[0] = '\0';
}