#include "Subsystem.h"

CdTe::CdTe(
    SUBSYSTEM_ORDER sys_name, 
    STATE_ORDER initial_state, 
    std::string local_ip, 
    unsigned short local_port, 
    std::string remote_ip, 
    unsigned short remote_port, 
    boost::asio::ip::tcp::endpoint& ground, 
    boost::asio::io_context& io_context)
    : TCPInterface::TCPInterface(
        local_ip, 
        local_port, 
        remote_ip, 
        remote_port, 
        io_context) {
    
    state = state;
    name = name;

    ground_endpoint = ground;
}

CdTe::~CdTe() {
    TCPInterface::~TCPInterface();

    // do whatever to end TCP connection
}

void CdTe::forward_to_ground() {
    // receive from remote_endpoint
    // send to ground_endpoint
}