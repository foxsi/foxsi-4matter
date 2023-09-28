#include "TransportLayer.h"
#include "Ticker.h"
#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <string>

int main() {
    
    // for gse-based testing; flip these if running on formatter processor
    std::string local_ip = "192.168.1.108";
    std::string ground_ip = "192.168.1.8";

    // number ports for local receiving, remote TCP, and remote UDP.
    unsigned short local_ground_port = 10000;
    unsigned short remote_ground_port = 10000;
    unsigned short remote_subsys_port = 9999;

    // create the Boost io_context to handle async events
    boost::asio::io_context context;

    // durations of each state (currently UNUSED)
    std::map<STATE_ORDER, double> durations{
        {STATE_ORDER::CMD_SEND, 1.0},
        {STATE_ORDER::DATA_REQ, 1.0},
        {STATE_ORDER::DATA_RECV, 1.0},
        {STATE_ORDER::IDLE, 1.0}
    };

    // currently UNUSED (not even sure what I wanted to do with this...)
    std::map<std::string, std::string> flags{
        {"flag0", "no"}
    };

    TransportLayerMachine frmtr(
        local_ip,               // IP address of this computer
        ground_ip,              // IP of the remote TCP computer (to listen to)
        ground_ip,              // IP of the remote UDP computer (to send to)
        local_ground_port,      // port number on this computer to listen for TCP on
        remote_ground_port,     // port number on the remote TCP computer
        remote_subsys_port,     // port number on the remote UDP computer (to send to)
        context
    );

    frmtr.recv_tcp_fwd_udp();   // the io_context will asynchronously liten for TCP packets and forward them out to the UDP endpoint
    frmtr.recv_udp_fwd_tcp();   // ...and vice-versa
    context.run();

    return 0;
}
