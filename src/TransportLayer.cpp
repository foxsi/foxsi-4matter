#include "TransportLayer.h"
#include "Utilities.h"
// #include "DataLinkLayer.h"

#include <boost/bind.hpp>   // boost::bind (for async handler to class members)
#include <algorithm>        // std::fill
#include <iomanip>
#include <stdexcept>
#include <utility>
#include <thread>
#include <chrono>

// construct from Parameters.h macro constants
TransportLayerMachine::TransportLayerMachine(
    std::shared_ptr<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>> new_uplink_buffer, 
    std::shared_ptr<moodycamel::ConcurrentQueue<DownlinkBufferElement>> new_downlink_buffer,
    boost::asio::io_context& context
):  
    io_context(context),
    local_udp_sock(context), 
    local_tcp_sock(context),
    local_tcp_housekeeping_sock(context),
    uplink_buffer(new_uplink_buffer),
    downlink_buffer(new_downlink_buffer),
    local_uart_port(context),
    uplink_uart_port(context)
{
    active_subsys = SUBSYSTEM_ORDER::HOUSEKEEPING;
    active_state = STATE_ORDER::IDLE;

    // commands = CommandDeck();
    
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
    local_tcp_sock.open(boost::asio::ip::tcp::v4());

    set_socket_options();

    local_udp_sock.bind(local_udp_endpoint);
    local_tcp_sock.bind(local_tcp_endpoint);
    local_tcp_sock.connect(remote_tcp_endpoint);
}

// construct from existing boost asio endpoints
TransportLayerMachine::TransportLayerMachine(
    boost::asio::ip::udp::endpoint local_udp_end,
    boost::asio::ip::tcp::endpoint local_tcp_end,
    boost::asio::ip::tcp::endpoint local_tcp_housekeeping_end,
    boost::asio::ip::udp::endpoint remote_udp_end,
    boost::asio::ip::tcp::endpoint remote_tcp_end,
    boost::asio::ip::tcp::endpoint remote_tcp_housekeeping_end,
    std::shared_ptr<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>> new_uplink_buffer, 
    std::shared_ptr<moodycamel::ConcurrentQueue<DownlinkBufferElement>> new_downlink_buffer,
    boost::asio::io_context& context
): 
    io_context(context),
    local_udp_sock(context),
    local_tcp_sock(context),
    local_tcp_housekeeping_sock(context),
    uplink_buffer(new_uplink_buffer),
    downlink_buffer(new_downlink_buffer),
    local_uart_port(context),
    uplink_uart_port(context)
{
    remote_udp_endpoint = remote_udp_end;
    remote_tcp_endpoint = remote_tcp_end;
    remote_tcp_housekeeping_endpoint = remote_tcp_housekeeping_end;
    
    active_subsys = SUBSYSTEM_ORDER::HOUSEKEEPING;
    active_state = STATE_ORDER::IDLE;
    
    // commands = CommandDeck();
    
    std::vector<uint8_t> tmp_vec(config::buffer::RECV_BUFF_LEN, '\0');
    downlink_buff = tmp_vec;
    uplink_buff = tmp_vec;
    
    std::vector<uint8_t> tmp_cmd = {};
    command_pipe = tmp_cmd;

    local_udp_sock.open(boost::asio::ip::udp::v4());
    local_tcp_sock.open(boost::asio::ip::tcp::v4());
    local_tcp_housekeeping_sock.open(boost::asio::ip::tcp::v4());
    
    set_socket_options();

    local_udp_sock.bind(local_udp_end);
    
    local_tcp_sock.bind(local_tcp_end);
    local_tcp_sock.connect(remote_tcp_endpoint);

    local_tcp_housekeeping_sock.bind(local_tcp_housekeeping_end);
    local_tcp_housekeeping_sock.connect(remote_tcp_housekeeping_endpoint);
}

TransportLayerMachine::TransportLayerMachine(
    boost::asio::ip::udp::endpoint local_udp_end, 
    boost::asio::ip::tcp::endpoint local_tcp_end, 
    boost::asio::ip::tcp::endpoint local_tcp_housekeeping_end, 
    boost::asio::ip::udp::endpoint remote_udp_end, 
    boost::asio::ip::tcp::endpoint remote_tcp_end, 
    boost::asio::ip::tcp::endpoint remote_tcp_housekeeping_end, 
    std::shared_ptr<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>> new_uplink_buffer, std::shared_ptr<moodycamel::ConcurrentQueue<DownlinkBufferElement>> new_downlink_buffer, 
    std::shared_ptr<UART> local_uart, 
    std::shared_ptr<UART> uplink_uart, 
    boost::asio::io_context &context
): 
    io_context(context),
    local_udp_sock(context),
    local_tcp_sock(context),
    local_tcp_housekeeping_sock(context),
    uplink_buffer(new_uplink_buffer),
    downlink_buffer(new_downlink_buffer),
    local_uart_port(context),
    uplink_uart_port(context)
{
    remote_udp_endpoint = remote_udp_end;
    remote_tcp_endpoint = remote_tcp_end;
    remote_tcp_housekeeping_endpoint = remote_tcp_housekeeping_end;
    
    active_subsys = SUBSYSTEM_ORDER::HOUSEKEEPING;
    active_state = STATE_ORDER::IDLE;
    
    // commands = CommandDeck();
    
    std::vector<uint8_t> tmp_vec(config::buffer::RECV_BUFF_LEN, '\0');
    downlink_buff = tmp_vec;
    uplink_buff = tmp_vec;
    
    std::vector<uint8_t> tmp_cmd = {};
    command_pipe = tmp_cmd;

    local_udp_sock.open(boost::asio::ip::udp::v4());
    local_udp_sock.bind(local_udp_end);

    local_tcp_sock.open(boost::asio::ip::tcp::v4());
    local_tcp_housekeeping_sock.open(boost::asio::ip::tcp::v4());
    
    set_socket_options();
    
    local_tcp_sock.bind(local_tcp_end);
    local_tcp_sock.connect(remote_tcp_endpoint);

    local_tcp_housekeeping_sock.bind(local_tcp_housekeeping_end);
    local_tcp_housekeeping_sock.connect(remote_tcp_housekeeping_endpoint);

    std::cout << "opening serial ports...\n";
    try {
        utilities::debug_print("\topening " + local_uart->tty_path + "\n");
        local_uart_port.open(local_uart->tty_path);
        try {
            set_local_serial_options(local_uart);
        } catch (std::exception& e) {
            utilities::error_print("failed to set local serial port options!: " + std::string(e.what()) + "\n");
        }
    } catch (std::exception& e) {
        utilities::error_print("failed to open local serial port!: " + std::string(e.what()) + "\n");
    }

    try {
        uplink_uart_port.open(uplink_uart->tty_path);
        try {
            utilities::debug_print("\topening " + uplink_uart->tty_path + "\n");
            set_uplink_serial_options(uplink_uart);
        } catch (std::exception& e) {
            utilities::error_print("failed to set uplink serial port options!: " + std::string(e.what()) + "\n");
        }
    } catch (std::exception& e) {
        utilities::error_print("failed to open uplink serial port!: " + std::string(e.what()) + "\n");
    }
}

void TransportLayerMachine::add_commands(std::shared_ptr<CommandDeck> new_commands) {
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
    // todo: update all these. Maybe even foxsi4-commands/dma/*/ring_buffer_interface.json should include an array of frame read commands for each applicable system.
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
        utilities::debug_print("\tcommand is not for frame read\n");
    }
    return false;
}

void TransportLayerMachine::set_socket_options() {
    boost::asio::socket_base::reuse_address reuse_addr_option(true);
    std::cout << "trying to set ::reuse_address\n";
    
    local_udp_sock.set_option(reuse_addr_option);
    local_tcp_sock.set_option(reuse_addr_option);
    local_tcp_housekeeping_sock.set_option(reuse_addr_option);
}

void TransportLayerMachine::set_local_serial_options(std::shared_ptr<UART> port) {
    local_uart_port.set_option(boost::asio::serial_port_base::baud_rate(port->baud_rate));
    local_uart_port.set_option(boost::asio::serial_port_base::character_size(port->data_bits));

    if (port->parity == 0) {
        local_uart_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    } else if (port->parity == 1) {
        local_uart_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::odd));
    } else if (port->parity == 2) {
        local_uart_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::even));
    } else {
        utilities::error_print("unacceptable parity option for serial port!\n");
    }

    if (port->stop_bits == 1) {
        local_uart_port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    } else if (port->stop_bits == 2) {
        local_uart_port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::two));
    } else {
        utilities::error_print("unacceptable stop bits option for serial port!\n");
    }
}

void TransportLayerMachine::set_uplink_serial_options(std::shared_ptr<UART> port) {
    uplink_uart_port.set_option(boost::asio::serial_port_base::baud_rate(port->baud_rate));
    uplink_uart_port.set_option(boost::asio::serial_port_base::character_size(port->data_bits));

    if (port->parity == 0) {
        uplink_uart_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    } else if (port->parity == 1) {
        uplink_uart_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::odd));
    } else if (port->parity == 2) {
        uplink_uart_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::even));
    } else {
        utilities::error_print("unacceptable parity option for serial port!\n");
    }

    if (port->stop_bits == 1) {
        uplink_uart_port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    } else if (port->stop_bits == 2) {
        uplink_uart_port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::two));
    } else {
        utilities::error_print("unacceptable stop bits option for serial port!\n");
    }
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
    std::vector<uint8_t> reply_data = get_reply_data(downlink_buff, 0x08);
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

void TransportLayerMachine::send_uart() {
    std::cout << "in send_uart\n";

    // const std::vector<uint8_t> FRAME1 = {0x03, 0x02, 0x05, 0x07};
    // uart.vsettings(5, FRAME1.size()); // can set up the port with this info or change when needed

    // // send the frame information
    // uart.send(FRAME1);    
}

void TransportLayerMachine::recv_uart() {
    std::cout << "in rec_uart\n";

    // define a vector to be used as a buffer 
    // randomly choose 4
    // std::vector<uint8_t> buffer(4);

    // // send buffer to the reading port method
    // uart.recv(buffer);    
}

// todo: rewrite this with Buffer.h stuff
void TransportLayerMachine::handle_cmd() {
    // todo: save pointers in these buffers (cmd and uplink) to manage multiple sequential reads/writes safely. Maybe with vector of vectors
    std::cout << "in handle_cmd()\n";

    uint8_t uplink_buff_sys = uplink_buff[0];
    uint8_t uplink_buff_cmd = uplink_buff[1];

    bool is_frame_read_cmd = check_frame_read_cmd(uplink_buff_sys, uplink_buff_cmd);

    std::cout << "sys: \t" << std::hex << (int)uplink_buff_sys << "\n";
    std::cout << "cmd: \t" << std::hex << (int)uplink_buff_cmd << "\n";

    std::vector<uint8_t> output_cmd = commands->get_command_bytes_for_sys_for_code(uplink_buff_sys, uplink_buff_cmd);

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
                std::vector<uint8_t> ring_read_cmd = commands->get_read_command_from_template(uplink_buff_sys, uplink_buff_cmd, ring_read_addr, ring_len);

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
                std::vector<uint8_t> ring_read_cmd = commands->get_read_command_from_template(uplink_buff_sys, uplink_buff_cmd, ring_read_addr, ring_len);

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

size_t TransportLayerMachine::read(boost::asio::ip::tcp::socket &socket, std::vector<uint8_t> &buffer, SystemManager &sys_man) {
    size_t retry_count = 0;
    bool did_read = false;
    while (retry_count < sys_man.timing->retry_max_count && !did_read) {
        std::vector<uint8_t> reply = TransportLayerMachine::sync_tcp_read(socket, buffer.size(), std::chrono::milliseconds(sys_man.timing->timeout_millis));
        if (reply.size() == 0) {
            utilities::error_print("TransportLayerMachine::read() attempt "  + std::to_string(retry_count) + " failed.\n");
        } else {
            buffer = reply;
            return buffer.size();
        }
        ++retry_count;
    }
    utilities::error_print("All TransportLayerMachine::read() attempts failed!\n");
    return 0;
}

size_t TransportLayerMachine::read_some(boost::asio::ip::tcp::socket &socket, std::vector<uint8_t> &buffer, SystemManager &sys_man) {
    size_t retry_count = 0;
    bool did_read = false;
    while (retry_count < sys_man.timing->retry_max_count && !did_read) {
        std::vector<uint8_t> reply = TransportLayerMachine::sync_tcp_read_some(socket, std::chrono::milliseconds(sys_man.timing->timeout_millis));
        if (reply.size() == 0) {
            utilities::error_print("TransportLayerMachine::read() attempt "  + std::to_string(retry_count) + " failed.\n");
        } else {
            buffer = reply;
            return buffer.size();
        }
        ++retry_count;
    }
    utilities::error_print("All TransportLayerMachine::read() attempts failed!\n");
    return 0;
}

size_t TransportLayerMachine::read(boost::asio::serial_port &port, std::vector<uint8_t> &buffer, SystemManager &sys_man) {
    size_t retry_count = 0;
    bool did_read = false;
    while (retry_count < sys_man.timing->retry_max_count && !did_read) {
        std::vector<uint8_t> reply = TransportLayerMachine::sync_uart_read(port, buffer.size(), std::chrono::milliseconds(sys_man.timing->timeout_millis));
        if (reply.size() == 0) {
            utilities::error_print("TransportLayerMachine::read() attempt "  + std::to_string(retry_count) + " failed.\n");
        } else {
            buffer = reply;
            return buffer.size();
        }
        ++retry_count;
    }
    utilities::error_print("All TransportLayerMachine::read() attempts failed!\n");
    return 0;
}

size_t TransportLayerMachine::read_udp(boost::asio::ip::udp::socket &socket, std::vector<uint8_t> &buffer, SystemManager &sys_man) {
    size_t retry_count = 0;
    bool did_read = false;
    while (retry_count < sys_man.timing->retry_max_count && !did_read) {
        std::vector<uint8_t> reply = TransportLayerMachine::sync_udp_read(socket, buffer.size(), std::chrono::milliseconds(sys_man.timing->timeout_millis));
        if (reply.size() == 0) {
            // utilities::error_print("TransportLayerMachine::read_udp() attempt "  + std::to_string(retry_count) + " failed.\n");
        } else {
            buffer = reply;
            return buffer.size();
        }
        ++retry_count;
    }
    // utilities::error_print("All TransportLayerMachine::read() attempts failed!\n");
    return 0;
}

std::vector<uint8_t> TransportLayerMachine::sync_tcp_read(size_t receive_size, std::chrono::milliseconds timeout_ms) {
    tcp_local_receive_swap.resize(receive_size);
    boost::system::error_code ec;

    for (size_t i = 0; i < 3; ++i) {
        local_tcp_sock.async_receive(
            boost::asio::buffer(tcp_local_receive_swap),
            boost::bind(
                &TransportLayerMachine::sync_tcp_read_handler,
                boost::placeholders::_1, 
                boost::placeholders::_2, 
                &ec, 
                &receive_size
            )
        );

        // Run the operation until it completes, or until the timeout.
        bool timed_out = TransportLayerMachine::run_tcp_context(timeout_ms);
        if (!timed_out) {
            break;
        } else {
            utilities::error_print("read attempt " + std::to_string(i) + " timed out! trying again.\n");
        }
    }
    std::vector<uint8_t> swap_copy(tcp_local_receive_swap);
    swap_copy.resize(receive_size);
    tcp_local_receive_swap.resize(0);

    return swap_copy;
}

std::vector<uint8_t> TransportLayerMachine::sync_tcp_read(boost::asio::ip::tcp::socket& socket, size_t receive_size, std::chrono::milliseconds timeout_ms) {
    boost::system::error_code err;
    tcp_local_receive_swap.resize(receive_size);

    boost::asio::async_read(
        socket,
        boost::asio::buffer(tcp_local_receive_swap),
        boost::bind(
            &TransportLayerMachine::sync_tcp_read_handler,
            boost::placeholders::_1, 
            boost::placeholders::_2, 
            &err, 
            &receive_size
        )
    );
    bool timed_out = TransportLayerMachine::run_tcp_context(timeout_ms);

    if (timed_out) {
        return {};
    } else {
        std::vector<uint8_t> swap_copy(tcp_local_receive_swap);
        swap_copy.resize(receive_size);
        tcp_local_receive_swap.resize(0);
        return swap_copy;
    }
}

std::vector<uint8_t> TransportLayerMachine::sync_tcp_read_some(boost::asio::ip::tcp::socket &socket, std::chrono::milliseconds timeout_ms) {
    boost::system::error_code err;
    tcp_local_receive_swap.resize(4096);
    size_t receive_size;

    socket.async_read_some(
        boost::asio::buffer(tcp_local_receive_swap),
        boost::bind(
            &TransportLayerMachine::sync_tcp_read_handler,
            boost::placeholders::_1, 
            boost::placeholders::_2, 
            &err, 
            &receive_size
        )
    );
    bool timed_out = TransportLayerMachine::run_tcp_context(timeout_ms);

    if (timed_out) {
        return {};
    } else {
        std::vector<uint8_t> swap_copy(tcp_local_receive_swap);
        swap_copy.resize(receive_size);
        tcp_local_receive_swap.resize(0);
        return swap_copy;
    }
}

std::vector<uint8_t> TransportLayerMachine::sync_uart_read(boost::asio::serial_port &port, size_t receive_size, std::chrono::milliseconds timeout_ms) {
    boost::system::error_code err;
    utilities::debug_print("will resize uart buffer to " + std::to_string(receive_size) + "\n");
    uart_local_receive_swap.resize(receive_size);

    utilities::debug_print("starting async uart read\n");

    boost::asio::async_read(
        port,
        boost::asio::buffer(uart_local_receive_swap),
        boost::bind(
            &TransportLayerMachine::sync_uart_read_handler,
            boost::placeholders::_1, 
            boost::placeholders::_2, 
            &err, 
            &receive_size
        )
    );
    bool timed_out = TransportLayerMachine::run_uart_context(timeout_ms);

    if (timed_out) {
        utilities::error_print("uart read timed out!\n");
        return {};
    } else {
        std::vector<uint8_t> swap_copy(uart_local_receive_swap);
        swap_copy.resize(receive_size);
        uart_local_receive_swap.resize(0);
        return swap_copy;
    }
}

std::vector<uint8_t> TransportLayerMachine::sync_udp_read(boost::asio::ip::udp::socket &socket, size_t receive_size, std::chrono::milliseconds timeout_ms) {
    boost::system::error_code err;
    udp_local_receive_swap.resize(receive_size);

    socket.async_receive_from(
        boost::asio::buffer(udp_local_receive_swap),
        remote_udp_endpoint,
        boost::bind(
            &TransportLayerMachine::sync_tcp_read_handler,
            boost::placeholders::_1, 
            boost::placeholders::_2, 
            &err, 
            &receive_size
        )
    );
    bool timed_out = TransportLayerMachine::run_udp_context(timeout_ms);

    if (timed_out) {
        return {};
    } else {
        utilities::debug_print("udp received!\n");
        std::vector<uint8_t> swap_copy(udp_local_receive_swap);
        swap_copy.resize(receive_size);
        udp_local_receive_swap.resize(0);
        return swap_copy;
    }
}

void TransportLayerMachine::sync_tcp_read_handler(const boost::system::error_code &ec, std::size_t length, boost::system::error_code *out_ec, std::size_t *out_length)
{
    *out_ec = ec;
    *out_length = length;
}

void TransportLayerMachine::sync_uart_read_handler(const boost::system::error_code &ec, std::size_t length, boost::system::error_code *out_ec, std::size_t *out_length) {
    *out_ec = ec;
    *out_length = length;
}

bool TransportLayerMachine::run_udp_context(std::chrono::milliseconds timeout_ms)
{
    io_context.restart();
    io_context.run_for(timeout_ms);
    if (!io_context.stopped()) {
        local_udp_sock.cancel();
        io_context.run();
        return true;
    }
    return false;
}

bool TransportLayerMachine::run_tcp_context(std::chrono::milliseconds timeout_ms)
{
    io_context.restart();
    io_context.run_for(timeout_ms);
    if (!io_context.stopped()) {
        local_tcp_sock.cancel();
        local_tcp_housekeeping_sock.cancel();
        io_context.run();
        return true;
    }
    return false;
}

bool TransportLayerMachine::run_uart_context(std::chrono::milliseconds timeout_ms) {
    io_context.restart();
    io_context.run_for(timeout_ms);
    if (!io_context.stopped()) {
        local_uart_port.cancel();
        io_context.run();
        return true;
    }
    return false;
}

void TransportLayerMachine::run_tcp_context(SystemManager &sys_man) {
    // will need some logic to pick the right io_context.
    io_context.restart();

    for (uint32_t i = 0; i < sys_man.timing->retry_max_count; ++i) {
        io_context.run_for(std::chrono::milliseconds(sys_man.timing->timeout_millis));
        if (!io_context.stopped()) {
            utilities::error_print("Attempt " + std::to_string(i) + " timed out in TransportLayerMachine for " + sys_man.system.name + "!");
        }
    }
}

size_t TransportLayerMachine::sync_remote_buffer_transaction(SystemManager &sys_man, RING_BUFFER_TYPE_OPTIONS buffer_type, size_t prior_write_pointer) {
    utilities::debug_print("in sync_remote_buffer_transaction() for " + sys_man.system.name + "\n");

    if(!sys_man.system.spacewire) {
        utilities::error_print("require spacewire interface to read ring buffer.\n");
        return prior_write_pointer;
    }

    // checking if the ring buffer parameters contains the desired buffer_type
    if(sys_man.system.ring_params.size() < static_cast<uint8_t>(buffer_type)) {
        utilities::error_print("System::ring_params does not contain the requested buffer type.\n");
        return prior_write_pointer;
    }
    
    // select the ring buffer parameter object to use for this interaction
    RingBufferParameters ring_params(sys_man.system.ring_params[buffer_type]);
    utilities::debug_print("\tcan access ring buffer parameters\n");

    // get write address width
    uint8_t write_pointer_width = ring_params.write_pointer_width_bytes;
    // get write address as byte stream
    std::vector<uint8_t> write_pointer_bytes(utilities::splat_to_nbytes(write_pointer_width, ring_params.write_pointer_address));
    utilities::debug_print("\twrite pointer width: " + std::to_string(write_pointer_width) + "\n");

    // packet to read write point:
    std::vector<uint8_t> write_pointer_request_packet(commands->get_read_command_for_sys_at_address(sys_man.system.hex, write_pointer_bytes, write_pointer_width));
    utilities::debug_print("\tsending read command: ");
    utilities::spw_print(write_pointer_request_packet, sys_man.system.spacewire);
    
    // RMAP read request the write pointer:
    local_tcp_sock.send(boost::asio::buffer(write_pointer_request_packet));
    utilities::debug_print("\trequested remote write pointer\n");

    // RMAP read reply:
    std::vector<uint8_t> last_reply;
    last_reply.resize(4096);
    size_t reply_len = TransportLayerMachine::read_some(local_tcp_sock, last_reply, sys_man);
    utilities::debug_print("\tgot remote write pointer, reply length " + std::to_string(reply_len) + "\n");
    
    // wrap the reply vector to the correct length:
    last_reply.resize(reply_len);

    // if read length is too short:
    if (reply_len < 26) {
        utilities::error_print("received only " + std::to_string(reply_len) + " of data (26 requred):\n\t");
        utilities::hex_print(last_reply);
        return prior_write_pointer;
    } else {
        utilities::spw_print(last_reply, nullptr);
    }

    // extract the data field from the reply:
    std::vector<uint8_t> last_write_pointer_bytes(get_reply_data(last_reply, sys_man.system.hex));
    if(last_write_pointer_bytes.size() != 4) {
        utilities::error_print("got bad write pointer length!\n");
    }
    utilities::debug_print("\textracted write pointer from reply: ");
    utilities::hex_print(last_write_pointer_bytes);

    // if CMOS, swap endianness of reply:
    if(sys_man.system.hex == commands->get_sys_code_for_name("cmos1") || sys_man.system.hex == commands->get_sys_code_for_name("cmos2")) {
        last_write_pointer_bytes = utilities::swap_endian4(last_write_pointer_bytes);
        utilities::debug_print("\tfound cmos, swapped endianness\n");
    }
    // later, use EVTM? Or a union object?
    FramePacketizer* fp(sys_man.get_frame_packetizer(buffer_type));
    utilities::debug_print("\tgot pointer to fp: " + fp->to_string());
    PacketFramer* pf(sys_man.get_packet_framer(buffer_type));
    utilities::debug_print("\tgot pointer to pf: " + pf->to_string());

    size_t last_write_pointer(utilities::unsplat_from_4bytes(last_write_pointer_bytes));

    // confirm write pointer in ring buffer
    // todo: this is not a valid criterion for cmos, which does not use all of its buffer area for frames.

    // if (last_write_pointer < ring_params.start_address || last_write_pointer > ring_params.start_address + ring_params.frames_per_ring*ring_params.frame_size_bytes) {
    //     utilities::error_print("write pointer " + std::to_string(last_write_pointer) + " outside ring buffer for system with ");
    //     utilities::error_print(ring_params.to_string());
    //     return prior_write_pointer;
    // }

    // check if we are reading a duplicate frame (interlock with Circle::manager_systems() context)
    if (last_write_pointer == prior_write_pointer) {
        utilities::debug_print("write pointer has not advanced, skipping\n");
        return prior_write_pointer;
    }

    // log this time for frame time measurement:
    auto frame_start_time = std::chrono::high_resolution_clock::now();
    size_t packet_counter = 0;
    while (!pf->check_frame_done()) {
        
        // update the address to read from:
        size_t this_start_address = last_write_pointer + packet_counter*sys_man.system.spacewire->max_payload_size;
        
        write_pointer_bytes = utilities::splat_to_nbytes(write_pointer_width, this_start_address);
        // make a new RMAP read command for that address:
        std::vector<uint8_t> buffer_read_command(commands->get_read_command_for_sys_at_address(sys_man.system.hex, write_pointer_bytes, sys_man.system.ethernet->max_payload_size));
        
        // start RTT timer
        auto rtt_start_time = std::chrono::high_resolution_clock::now();
        // send the read command:
        local_tcp_sock.send(boost::asio::buffer(buffer_read_command));

        // receive the command response:
        // want 1825 bytes back for CdTe (runtime constant per foxsi4-commands/systems.json)
        size_t expected_size = sys_man.system.spacewire->static_footer_size + sys_man.system.spacewire->static_header_size + sys_man.system.ethernet->max_payload_size;
        utilities::debug_print("\t\twaiting to receive " + std::to_string(expected_size) + " from system\n");
        std::vector<uint8_t> last_buffer_reply(expected_size);

        // read, using timeout/retry properties in `sys_man`
        reply_len = TransportLayerMachine::read(local_tcp_sock, last_buffer_reply, sys_man);

        // error if incorrect length is read
        last_buffer_reply.resize(reply_len);
        if (reply_len != expected_size) {
            utilities::error_print("expected " + std::to_string(expected_size) + " bytes but received " + std::to_string(reply_len) + "\n");
            return prior_write_pointer;
        }
        
        // log raw data to prelogger file for debug
        size_t remaining_size = std::min(ring_params.frame_size_bytes - pf->get_frame().size(), reply_len - sys_man.system.spacewire->static_header_size - sys_man.system.spacewire->static_footer_size);
        
        std::vector<uint8_t> trace_vec(last_buffer_reply.begin() + sys_man.system.spacewire->static_header_size, last_buffer_reply.end() - sys_man.system.spacewire->static_footer_size);
        trace_vec.resize(remaining_size);
        utilities::trace_prelog(trace_vec);

        // log RTT timer to logger file for debug
        utilities::debug_log("rtt read time (ms): ");
        utilities::debug_log(std::to_string(std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - rtt_start_time).count()));

        // push that response onto the frame:
        pf->push_to_frame(last_buffer_reply);

        ++packet_counter;
    }
    // write to log
    utilities::debug_log("frame read time (ms): ");
    utilities::debug_log(std::to_string(std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - frame_start_time).count()));

    // hand the complete frame to the packetizer
    fp->set_frame(pf->get_frame());
    utilities::debug_print("\tset frame packetizer frame\n");
    utilities::debug_print("\tpacket framer frame.size(): " + std::to_string(pf->get_frame().size()) + "\n");
    
    // write to log
    utilities::debug_log("PacketFramer::frame: ");
    utilities::trace_log(pf->get_frame());

    // push the frame onto the downlink queue
    while (!fp->frame_emptied()) {
        // fp.pop_buffer_element()
        DownlinkBufferElement this_dbe(fp->pop_buffer_element());
        downlink_buffer->enqueue(this_dbe);
        // utilities::debug_print("\t\tqueued packet\n");
    }
    
    // clear the old frame to use the PacketFramer again.
    pf->clear_frame();

    utilities::debug_print("\tpushed ring buffer data to downlink buffer\n");
    return last_write_pointer;
}

void TransportLayerMachine::sync_tcp_send_buffer_commands_to_system(SystemManager &sys_man) {
    utilities::debug_print("in sync_tcp_send_buffer_commands_to_system()\n");
    Command command;
    System system;
    std::vector<uint8_t> varargs;

    bool uplink_available;
    bool system_unavailable;
    UplinkBufferElement element;
    moodycamel::ConcurrentQueue<UplinkBufferElement> this_system_queue;

    try {
        // uplink_available = (uplink_buffer->at(sys_man.system)).try_dequeue(element);
        uplink_buffer->at(sys_man.system);
    } catch (std::out_of_range& e) {
        system_unavailable = true;
        utilities::error_print("uplink buffer does not contain system, returning\n");
        return;
    }
    
    uplink_available = uplink_buffer->at(sys_man.system).try_dequeue(element);

    if (!uplink_available) {
        utilities::debug_print("no commands in queue\n");
        return;
    }
    command = *element.get_command();
    system = *element.get_system();
    varargs = element.get_varargs();


    if (sys_man.system != system) {
        utilities::error_print("got wrong buffer for system!\n");
    }

    if (check_frame_read_cmd(system.hex, command.hex)) {
        utilities::debug_print("got frame read command. Not implemented yet\n");
        // todo: call sync_remote_buffer_transaction().
    } else {
        // todo: possibly wrap this in a try block. Or make CommandDeck::get_command_bytes_for_sys_for_code() robust to missed keys.
        std::vector<uint8_t> send_packet(commands->get_command_bytes_for_sys_for_code(system.hex, command.hex));
        utilities::debug_print("got command for system. Sending...\n");
        // utilities::hex_print(send_packet);
        utilities::spw_print(send_packet, sys_man.system.spacewire);
        // todo: make this work for housekeeping
        local_tcp_sock.send(boost::asio::buffer(send_packet));
    }
}

std::vector<uint8_t> TransportLayerMachine::sync_tcp_housekeeping_transaction(std::vector<uint8_t> data_to_send) {
    utilities::debug_print("sending " + std::to_string(data_to_send.size()) + " bytes: ");
    utilities::hex_print(data_to_send);
    std::vector<uint8_t> reply;
    // todo: no magic numbers
    reply.resize(1024);
    local_tcp_housekeeping_sock.send(boost::asio::buffer(data_to_send));
    size_t reply_len = local_tcp_housekeeping_sock.read_some(boost::asio::buffer(reply));
    reply.resize(reply_len);

    utilities::debug_print("received " + std::to_string(reply_len) + " bytes: ");
    utilities::hex_print(reply);

    // async_udp_send_downlink_buffer();

    return reply;
}

void TransportLayerMachine::sync_tcp_housekeeping_send(std::vector<uint8_t> data_to_send) {
    utilities::debug_print("transmitting to housekeeping_system\n");
    local_tcp_housekeeping_sock.send(boost::asio::buffer(data_to_send));
}

void TransportLayerMachine::sync_udp_receive_to_uplink_buffer(SystemManager &uplink_sys_man) {
    utilities::debug_print("in sync_udp_receive_to_uplink_buffer()\n");

    std::vector<uint8_t> reply(2);
    for (size_t count = 0; count < 8; ++count) {
        size_t reply_size = TransportLayerMachine::read_udp(local_udp_sock, reply, uplink_sys_man);
        if (reply_size == 0) {
            // utilities::error_print("got no uplink command\n");
            return;
        } else {
            utilities::debug_print("received uplink: ");
            utilities::hex_print(reply);
            utilities::debug_print("\n");
            // try to find queue for command
            uint8_t sys_code = reply.at(0);
            try{
                UplinkBufferElement new_uplink(reply, *commands);

                (uplink_buffer->at(commands->get_sys_for_code(sys_code))).enqueue(new_uplink);
            } catch (std::out_of_range& e) {
                // todo: log the error.
                utilities::error_print("could not add uplink command to queue!\n"); 
                return;
            }
            utilities::debug_print("\tstored uplink commands\n");
        }
    }
}

// todo: see if this works.
void TransportLayerMachine::async_udp_receive_to_uplink_buffer() {
    utilities::debug_print("in async_udp_receive_to_uplink_buffer()\n");
    uplink_swap.resize(config::buffer::RECV_BUFF_LEN);
    uplink_swap.resize(2);
    // std::vector<uint8_t> local_buffer(config::buffer::RECV_BUFF_LEN);
    local_udp_sock.async_receive_from(
        boost::asio::buffer(uplink_swap),
        remote_udp_endpoint,
        boost::bind(
            &TransportLayerMachine::async_udp_receive_push_to_uplink_buffer, 
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred
        )
    );
}

void TransportLayerMachine::async_udp_send_downlink_buffer() {
    utilities::debug_print("in TransportLayerMachine::async_udp_send_downlink_buffer()\n");

    DownlinkBufferElement dbe;
    bool has_data = true;
    while(has_data) {
        has_data = downlink_buffer->try_dequeue(dbe);
        if (has_data) {
            std::vector<uint8_t> packet = dbe.get_packet();

            utilities::debug_print("in downlink buffer, found ");
            utilities::hex_print(packet);
            utilities::debug_print("\nsending.\n");

            local_udp_sock.send_to(boost::asio::buffer(packet), remote_udp_endpoint);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // todo: something to limit datarate
}

bool TransportLayerMachine::sync_udp_send_all_downlink_buffer() {
    utilities::debug_print("in TransportLayerMachine::sync_udp_send_downlink_buffer()\n");

    // todo: replace this with non-dummy setup that adds enough max_packet_size to avoid errors when popping queue:
    DownlinkBufferElement dbe(&(commands->get_sys_for_name("uplink")), 2000);
    bool has_data = downlink_buffer->try_dequeue(dbe);

    while (has_data && dbe.get_system().hex != 0x05) {
        std::vector<uint8_t> packet = dbe.get_packet();
        // utilities::debug_print("sending to gse: ");
        // utilities::hex_print(packet);

        local_udp_sock.send_to(
            boost::asio::buffer(packet), 
            remote_udp_endpoint
        );

        has_data = downlink_buffer->try_dequeue(dbe);
    }
    return has_data;
}

std::vector<uint8_t> TransportLayerMachine::sync_tcp_send_command_for_sys(SystemManager sys_man, Command cmd) {
    std::vector<uint8_t> packet = commands->get_command_bytes_for_sys_for_code(sys_man.system.hex, cmd.hex);

    std::vector<uint8_t> reply(4096);

    utilities::debug_print("in sync_tcp_send_command_for_sys(), sending ");
    if (sys_man.system.type == COMMAND_TYPE_OPTIONS::SPW) {
        utilities::spw_print(packet, sys_man.system.spacewire);
    } else {
        utilities::hex_print(packet);
    }

    local_tcp_sock.send(boost::asio::buffer(packet));
    utilities::debug_print("in sync_tcp_send_command_for_sys(), sent request\n");

    if (cmd.read) {
        utilities::debug_print("waiting for response\n");
        size_t expected_size = cmd.get_spw_reply_length();
        reply.resize(expected_size);
        // todo: determine if ::read_some is correct here. 
        // size_t reply_len = TransportLayerMachine::read(local_tcp_sock, reply, sys_man);
        size_t reply_len = TransportLayerMachine::read_some(local_tcp_sock, reply, sys_man);
        if (reply_len == expected_size) {
            utilities::debug_print("got response: ");
        } else {
            utilities::error_print("got wrong reply size! Expected " + std::to_string(expected_size) + " and received " + std::to_string(reply_len) + "\n");
        }
        utilities::hex_print(reply);
        reply.resize(reply_len);
    } else {
        reply.resize(0);
    }
    return reply;
}

std::vector<uint8_t> TransportLayerMachine::sync_tcp_send_command_for_housekeeping_sys(SystemManager hk_man, Command cmd) {
    std::vector<uint8_t> packet = commands->get_command_bytes_for_sys_for_code(hk_man.system.hex, cmd.hex);

    std::vector<uint8_t> reply(4096);

    utilities::debug_print("in sync_tcp_send_command_for_housekeeping_sys(), sending ");
    if (hk_man.system.type != COMMAND_TYPE_OPTIONS::ETHERNET) {
        utilities::error_print("cannot send non-Ethernet command!");
    }

    local_tcp_housekeeping_sock.send(boost::asio::buffer(packet));
    utilities::debug_print("in sync_tcp_send_command_for_housekeeping_sys(), sent request\n");

    if (cmd.read) {
        utilities::debug_print("waiting for response\n");
        size_t expected_size = cmd.get_eth_reply_length();
        reply.resize(expected_size);
        // size_t reply_len = TransportLayerMachine::read(local_tcp_housekeeping_sock, reply, hk_man);
        size_t reply_len = TransportLayerMachine::read(local_tcp_housekeeping_sock, reply, hk_man);
        if (reply_len == expected_size) {
            utilities::debug_print("got response: ");
        } else {
            utilities::error_print("got wrong reply size! Expected " + std::to_string(expected_size) + " and received " + std::to_string(reply_len) + "\n");
        }
        utilities::hex_print(reply);
        reply.resize(reply_len);
    } else {
        reply.resize(0);
    }
    return reply;
}

std::vector<uint8_t> TransportLayerMachine::sync_uart_send_command_for_sys(SystemManager sys_man, Command cmd) {
    std::vector<uint8_t> packet = commands->get_command_bytes_for_sys_for_code(sys_man.system.hex, cmd.hex);

    std::vector<uint8_t> reply(4096);

    utilities::debug_print("in sync_uart_send_command_for_sys(), sending " + utilities::bytes_to_string(packet) + "\n");
    if (sys_man.system.type != COMMAND_TYPE_OPTIONS::UART) {
        utilities::error_print("cannot send non-UART command!");
    }

    local_uart_port.write_some(boost::asio::buffer(packet));
    utilities::debug_print("in sync_uart_send_command_for_sys(), sent request\n");

    if (cmd.read) {
        utilities::debug_print("waiting for response\n");
        size_t expected_size = cmd.get_uart_reply_length();
        reply.resize(expected_size);
        size_t reply_len = TransportLayerMachine::read(local_uart_port, reply, sys_man);
        if (reply_len > 0) {
            utilities::debug_print("got response: ");
            utilities::hex_print(reply);
        }
        reply.resize(reply_len);
    } else {
        reply.resize(0);
    }
    return reply;
}

void TransportLayerMachine::async_udp_receive_push_to_uplink_buffer(const boost::system::error_code& err, std::size_t byte_count) {
    utilities::debug_print("trying to push to uplink buffer\n");

    if (uplink_swap.empty()) {
        utilities::error_print("TransportLayerMachine received 0 bytes of UDP data!\n");
        return;
    }
    // trim extra buffer:
    uplink_swap.resize(byte_count);
    utilities::debug_print("received uplink command: ");
    utilities::hex_print(uplink_swap);

    uint8_t sys_code = uplink_swap[0];
    if (commands->get_sys_name_for_code(sys_code) == "formatter") {
        // don't queue command, act on it immediately.
        
    } else {
        // try to find queue for command
        try{
            UplinkBufferElement new_uplink(uplink_swap, *commands);

            (uplink_buffer->at(commands->get_sys_for_code(sys_code))).enqueue(new_uplink);
        } catch (std::out_of_range& e) {
            // todo: log the error.
            utilities::error_print("could not add uplink command to queue!\n");
        }
    }
    utilities::debug_print("\n");

    async_udp_receive_to_uplink_buffer();
}

void TransportLayerMachine::await_loop_begin() {
    
    utilities::debug_print("waiting for uplink to begin loop...\n");

    // How does this interact with the async uplink (currently not working?
    // Eventually this should be absorbed into async_udp_receive_to_uplink_buffer()  (intercept command and don't queue.)
    std::vector<uint8_t> uplink_msg(8);
    
    uint8_t sys = 0x00;
    uint8_t cmd = 0x00;

    // wait for system formatter and command 0x01
    while (sys != 0x01 && cmd != 0x0f) {
        local_udp_sock.receive_from(boost::asio::buffer(uplink_msg), remote_udp_endpoint);
        sys = uplink_msg[0];
        cmd = uplink_msg[1];
    }
}

std::vector<uint8_t> TransportLayerMachine::get_reply_data(std::vector<uint8_t> spw_reply, System& sys) {

    size_t ether_prefix_length = 12; // using SPMU-001, this is always true
    size_t target_path_address_length = 0; // path address is removed by the time we receive reply.

    // utilities::debug_print("\tpulling reply from ");
    // utilities::hex_print(spw_reply);
    // utilities::debug_print("\n");

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

    if (ether_prefix_length + target_path_address_length + data_length_start_offset + data_length_length + 1 > spw_reply.size()) {
        utilities::error_print("message too short to parse!\n");
        return {};
    }

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
    System& sys_obj = commands->get_sys_for_code(sys);
    
    return get_reply_data(spw_reply, sys_obj);
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