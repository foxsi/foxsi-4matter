#include "Subsystem.h"
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
    
    std::vector<uint8_t> tmp_vec(RECV_BUFF_LEN, '\0');
    downlink_buff = tmp_vec;
    uplink_buff = tmp_vec;

    std::vector<uint8_t> tmp_cmd = {};
    command_pipe = tmp_cmd;
    ground_pipe.push('0');

    boost::asio::ip::address local_addr = boost::asio::ip::make_address(LOCAL_IP);
    boost::asio::ip::address remote_udp_addr = boost::asio::ip::make_address(GSE_IP);
    boost::asio::ip::address remote_tcp_addr = boost::asio::ip::make_address(SPMU_IP);
    boost::asio::ip::udp::endpoint local_udp_endpoint(local_addr, LOCAL_PORT);
    boost::asio::ip::tcp::endpoint local_tcp_endpoint(local_addr, LOCAL_PORT);

    remote_udp_endpoint = boost::asio::ip::udp::endpoint(remote_udp_addr, GSE_PORT);
    remote_tcp_endpoint = boost::asio::ip::tcp::endpoint(remote_tcp_addr, SPMU_PORT);

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
    
    std::vector<uint8_t> tmp_vec(RECV_BUFF_LEN, '\0');
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
    
    std::vector<uint8_t> tmp_vec(RECV_BUFF_LEN, '\0');
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
        debug_print("checking for frame read of an unimplemented system!\n");
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
    hex_print(filtered);
    std::cout << "\n";

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

    bool is_frame_read_cmd = check_frame_read_cmd(uplink_buff_sys, uplink_buff_cmd);

    std::cout << "sys: \t" << std::hex << (int)uplink_buff_sys << "\n";
    std::cout << "cmd: \t" << std::hex << (int)uplink_buff_cmd << "\n";

    std::vector<uint8_t> output_cmd = commands.get_command_bytes_for_sys_for_code(uplink_buff_sys, uplink_buff_cmd);

    command_pipe.insert(command_pipe.end(), output_cmd.begin(), output_cmd.end());
    // command_pipe.push_back('\0');

    std::cout << "transmitting:\t";
    hex_print(output_cmd);
    std::cout << "to " << local_tcp_sock.remote_endpoint().address().to_string() << ":" << std::to_string(local_tcp_sock.remote_endpoint().port()) << "\n";

    if(is_frame_read_cmd) {
        std::cout << "\tfound frame read command, handling transaction\n";
        local_tcp_sock.send(boost::asio::buffer(output_cmd));

        std::vector<uint8_t> reply;
        reply.resize(1024);
        size_t reply_len = local_tcp_sock.read_some(boost::asio::buffer(reply));

        // std::vector<uint8_t> reply(reply_buff, reply_buff + 1024);
        
        std::cout << "got reply of length 0x" << reply.size() << " with reported size 0x" << reply_len << ":\t";
        hex_print(reply);
        std::cout << "\n";

        if(reply_len < 1) {
            std::cout << "\tgot no response from read command.\n";
            return;
        }

        // todo: verify reply length > 0 before proceeding to avoid index outside the vector.
        
        // data in SpW reply packet starts 12 B into the packet (after path address), and ends with CRC.
        // todo: replace with Parameters.h-defined values.
        size_t ether_offset_from_start = 12;
        size_t spw_offset_from_start = 12;
        size_t offset_from_end = 1;
        std::vector<uint8_t> remote_wr_ptr(reply.begin() + spw_offset_from_start + ether_offset_from_start, reply.end() - offset_from_end);

        // update the ring buffer interface for this system
        // todo: resolve question for IPMU team---does the returned write pointer include required offset? or do I need to add it?
        std::vector<uint32_t> spw_data = ring_buffers[uplink_buff_sys].get_spw_data(unsplat_from_4bytes(remote_wr_ptr));

        // check if ring buffer wraps around (2nd piece of buffer is zero-length):
        if(spw_data[3] == 0) {
            // send only one command.
            debug_print("\treading from non-wrapped ring buffer region.\n");
            // populate template command:
            std::vector<uint8_t> ring_addr = splat_to_nbytes(4, spw_data[0]);
            size_t ring_len = spw_data[1];
            std::vector<uint8_t> ring_read_cmd = commands.get_read_command_from_template(uplink_buff_sys, uplink_buff_cmd, ring_addr, ring_len);
            local_tcp_sock.async_send(
                boost::asio::buffer(ring_read_cmd),                   // send out the contents of uplink_buff
                boost::bind(&TransportLayerMachine::recv_udp_fwd_tcp_cmd, this)    // callback to recv_udp_fwd_tcp after send to continue listening
            );

        } else {
            // send two read commands.
            debug_print("\treading from wrapped ring buffer region.\n");
            // populate two template commands:
            std::vector<uint8_t> ring_addr1 = splat_to_nbytes(4, spw_data[0]);
            size_t ring_len1 = spw_data[1];
            std::vector<uint8_t> ring_read_cmd1 = commands.get_read_command_from_template(uplink_buff_sys, uplink_buff_cmd, ring_addr1, ring_len1);
            std::vector<uint8_t> ring_addr2 = splat_to_nbytes(4, spw_data[2]);
            size_t ring_len2 = spw_data[3];
            std::vector<uint8_t> ring_read_cmd2 = commands.get_read_command_from_template(uplink_buff_sys, uplink_buff_cmd, ring_addr2, ring_len2);
            
            local_tcp_sock.async_send(
                boost::asio::buffer(ring_read_cmd1),                   // send out the contents of uplink_buff
                boost::bind(&TransportLayerMachine::recv_udp_fwd_tcp_cmd, this)    // callback to recv_udp_fwd_tcp after send to continue listening
            );

            local_tcp_sock.async_send(
                boost::asio::buffer(ring_read_cmd2),                   // send out the contents of uplink_buff
                boost::bind(&TransportLayerMachine::recv_udp_fwd_tcp_cmd, this)    // callback to recv_udp_fwd_tcp after send to continue listening
            );
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

    // TODO: CHANGE THIS SO WE GET THE WRITE POINTER ADDRESS FOR SYSTEM
    std::vector<uint8_t> output_cmd = commands.get_command_bytes_for_sys_for_code(uplink_buff_sys, uplink_buff_cmd);

    // read the last write pointer
    local_tcp_sock.send(boost::asio::buffer(output_cmd));
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