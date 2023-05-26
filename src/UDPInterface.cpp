#include "UDPInterface.h"

UDPInterface::UDPInterface(std::string local_ip, unsigned short local_port, std::string remote_ip, unsigned short remote_port, boost::asio::io_context& io_context): local_socket(io_context) {
    local_address = boost::asio::ip::make_address(local_ip);
    remote_address = boost::asio::ip::make_address(remote_ip);

    local_endpoint = boost::asio::ip::udp::endpoint(local_address, local_port);
    remote_endpoint = boost::asio::ip::udp::endpoint(remote_address, remote_port);

    local_socket.open(boost::asio::ip::udp::v4());
    local_socket.bind(local_endpoint);
}

// UDPInterface::~UDPInterface() {
//     // do something to kill ongoing I/O and close socket?   
// }

int UDPInterface::recv(uint8_t* addr, uint8_t* buffer) {
    // build a SpW message using addr
    // wrap socket.recv_from(), recv_from remote_endpoint

    // eventually make buffer shared
    return 0;
}

int UDPInterface::async_recv(uint8_t* addr, uint8_t* buffer) {
    return 0;
}


int UDPInterface::send(uint8_t* addr, uint8_t* buffer) {
    return 0;
}

int UDPInterface::async_send(uint8_t* addr, uint8_t* buffer) {
    return 0;
}

// int recv(std::queue<std::vector<uint8_t>>& q) {
//     return 0;
// }
int recv(std::vector<std::vector<uint8_t>>& q) {
    return 0;
}
int send(std::queue<std::vector<uint8_t>>& q) {
    return 0;
}
// int send(std::vector<std::vector<uint8_t>>& q) {
//     return 0;
// }

// void UDPInterface::async_send(uint8_t* buffer, std::size_t length) {
//     // local_socket.send_to(buffer, remote_endpoint);
// }