#include "Subsystem.h"

#include <boost/bind.hpp>   // boost::bind (for async handler to class members)
#include <algorithm>        // std::fill

// construct from Parameters.h macro constants
PepperMill::PepperMill(
    boost::asio::io_context& context
):  
    local_udp_sock(context), 
    local_tcp_sock(context)
{
    active_subsys = HOUSEKEEPING;
    active_state = IDLE;
    
    std::vector<char> tmp_vec(RECV_BUFF_LEN, '\0');
    downlink_buff = tmp_vec;
    uplink_buff = tmp_vec;
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
PepperMill::PepperMill(
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
    active_subsys = HOUSEKEEPING;
    active_state = IDLE;
    
    std::vector<char> tmp_vec(RECV_BUFF_LEN, '\0');
    downlink_buff = tmp_vec;
    uplink_buff = tmp_vec;

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
PepperMill::PepperMill(
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
    
    active_subsys = HOUSEKEEPING;
    active_state = IDLE;
    
    std::vector<char> tmp_vec(RECV_BUFF_LEN, '\0');
    downlink_buff = tmp_vec;
    uplink_buff = tmp_vec;

    local_udp_sock.open(boost::asio::ip::udp::v4());
    local_udp_sock.bind(local_udp_end);

    local_tcp_sock.open(boost::asio::ip::tcp::v4());
    local_tcp_sock.bind(local_tcp_end);
    local_tcp_sock.connect(remote_tcp_endpoint);
}


void PepperMill::recv_tcp_fwd_udp() {
    std::cout << "in recv_tcp_fwd_udp()\n";

    // read incoming TCP data...
    local_tcp_sock.async_read_some(
        boost::asio::buffer(downlink_buff),         // read data into the downlink buffer
        boost::bind(&PepperMill::send_udp, this)    // callback to send_udp to forward the data after it has been received
    );
}

void PepperMill::recv_udp_fwd_tcp() {
    std::cout << "in recv_udp_fwd_tcp()\n";

    // read incoming UDP data...
    local_udp_sock.async_receive_from(
        boost::asio::buffer(uplink_buff),           // read data into the uplink buffer
        remote_udp_endpoint,                        // receive data from the UDP endpoint
        boost::bind(&PepperMill::send_tcp, this)    // callback to send_tcp to forward the data after it has been received
    );
}

void PepperMill::send_udp() {
    std::cout << "in send_udp\n";

    // forward the buffer downlink_buff over UDP...
    local_udp_sock.async_send_to(
        boost::asio::buffer(downlink_buff),                 // send out the contents of downlink_buff
        remote_udp_endpoint,                                // send to the UDP endpoint
        boost::bind(&PepperMill::recv_tcp_fwd_udp, this)    // callback to recv_tcp_fwd_udp after send to continue listening
    );

    // clear the downlink buffer
    std::fill(downlink_buff.begin(), downlink_buff.end(), '\0');
}

void PepperMill::send_tcp() {
    std::cout << "in send_tcp\n";

    // forward the buffer uplink_buff over TCP...
    local_tcp_sock.async_send(
        boost::asio::buffer(uplink_buff),                   // send out the contents of uplink_buff
        boost::bind(&PepperMill::recv_udp_fwd_tcp, this)    // callback to recv_udp_fwd_tcp after send to continue listening
    );

    // clear the buffer uplink_buff...
    std::fill(uplink_buff.begin(), uplink_buff.end(), '\0');
}
