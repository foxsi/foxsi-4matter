#include "Subsystem.h"

// construct from Parameters.h
PepperMill::PepperMill(
    boost::asio::io_context& context
):  
    local_udp_sock(context), 
    local_tcp_sock(context)
{
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


// Ground::Ground(
//     std::map<STATE_ORDER, double>& durations, 
//     std::map<std::string, std::string>& flags, 
//     std::string local_ip, 
//     unsigned short local_port,
//     std::string remote_ip, 
//     unsigned short remote_port, 
//     boost::asio::io_context& io_context
// ):  UDPInterface(local_ip, local_port, remote_ip, remote_port, io_context),
//     durations(durations),
//     flags(flags)
// {

// }

// Ground::Ground(
//     Ground& ground, 
//     boost::asio::io_context& io_context
// ):  UDPInterface(ground.local_ip, ground.local_port, ground.remote_ip, ground.remote_port)
// {

// }


// to resolve this, need multiple dispatch: define this function separately for each Subsystem, even if they share an interface.
// int Ground::send(char buff[RECV_BUFF_LEN], Housekeeping& caller) {
//     local_socket.async_send_to(
//         boost::asio::buffer(buff, RECV_BUFF_LEN), 
//         remote_endpoint,
//         [this, caller, buff](boost::system::error_code ec, std::size_t bytes_received) {
//             if(!ec && bytes_received > 0) {
//                 caller.recv(buff);
//             } else {
//                 std::cerr << ec.what();
//                 throw "couldn't repeat to ground\n";
//             }
//         }
//     );
// }



// Housekeeping::Housekeeping(
//     SUBSYSTEM_ORDER out_name, 
//     STATE_ORDER out_state,
//     Ground& in_ground,
//     std::map<STATE_ORDER, double>& durations, 
//     std::map<std::string, std::string>& flags,  
//     std::string local_ip, 
//     unsigned short local_port, 
//     std::string remote_ip, 
//     unsigned short remote_port,
//     boost::asio::io_context& io_context
// ):  UDPInterface(local_ip, local_port, remote_ip, remote_port, io_context), 
//     ground(in_ground, io_context), 
//     durations(durations), 
//     flags(flags) 
// {
//     is_active = false;
//     name = out_name;
//     state = out_state;
// }

// int Housekeeping::recv(std::vector<std::vector<char>>& q) {
//     char buff[RECV_BUFF_LEN];
//     local_socket.async_receive_from(
//         boost::asio::buffer(buff, RECV_BUFF_LEN),
//         remote_endpoint,
//         [this, buff, q](boost::system::error_code ec, std::size_t bytes_received) {
//             if(!ec && bytes_received > 0) {
//                 this->ground.send(buff, this);
//             } else {
//                 std::cout << ec.what();
//                 Housekeeping::recv(q);
//             }
//         }
//     );
//     return 0;
// }



// CdTe::CdTe(
//     SUBSYSTEM_ORDER sys_name, 
//     STATE_ORDER initial_state, 
//     std::string local_ip, 
//     unsigned short local_port, 
//     std::string remote_ip, 
//     unsigned short remote_port, 
//     boost::asio::ip::tcp::endpoint& ground, 
//     boost::asio::io_context& io_context
// )
//     : TCPInterface::TCPInterface
//     (
//         local_ip, 
//         local_port, 
//         remote_ip, 
//         remote_port, 
//         io_context
//     ) 
// {
    
//     state = state;
//     name = name;

//     ground_endpoint = ground;
// }

// CdTe::~CdTe() {
//     TCPInterface::~TCPInterface();

//     // do whatever to end TCP connection
// }

// void CdTe::forward_to_ground() {
//     // receive from remote_endpoint
//     // send to ground_endpoint
// }