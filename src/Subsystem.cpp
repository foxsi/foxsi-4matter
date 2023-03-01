#include "Subsystem.h"

#include <boost/bind.hpp>   // boost::bind (for async handler to class members)
#include <algorithm>        // std::fill

// construct from Parameters.h
PepperMill::PepperMill(
    boost::asio::io_context& context
):  
    local_udp_sock(context), 
    local_tcp_sock(context)
{
    active_subsys = HOUSEKEEPING;
    active_state = IDLE;
    
    std::vector<char> tmp_vec(RECV_BUFF_LEN, '\0');
    share_data = tmp_vec;
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
    share_data = tmp_vec;

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


void PepperMill::recv_tcp_fwd_udp() {
    std::cout << "in recv_tcp_fwd_udp()\n";

    // read incoming TCP data...
    local_tcp_sock.async_read_some(
        boost::asio::buffer(share_data),
        boost::bind(&PepperMill::send_udp, this)
    );
}

void PepperMill::send_udp() {
    std::cout << "in send_udp\n";

    // forward the buffer share_data over UDP...
    local_udp_sock.async_send_to(
        boost::asio::buffer(share_data),
        remote_udp_endpoint,
        boost::bind(&PepperMill::recv_tcp_fwd_udp, this)
    );

    // clear the buffer share_data...
    std::fill(share_data.begin(), share_data.end(), '\0');
}
