#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include "AbstractState.h"
#include "UDPInterface.h"
#include "TCPInterface.h"
#include "UARTInterface.h"
#include "Parameters.h"
// #include <boost/asio.hpp>
#include <vector>
#include <map>
#include <queue>

// forward declarations so these can reference each other:
// class Housekeeping;
// class Ground;

// /*  Concept
//     Housekeeping, CdTe, CMOS etc run in sequence, triggered by Metronome.
//     Ground::send() runs in parallel, interrupted by the ::send()s of the others.
//     Ground::send() multicasts to GSE and EVTM.
//     <Subsystem>::send()s forward commands and data requests to each subsystem, 
//     directly (UART to Timepix) or via TCP/IP to SPMU-001/SpW for the others.

//     Decide if Housekeeping (plenum board) will be UDP or TCP
// */

// class Ground: public UDPInterface::UDPInterface {
//     public:
//         // Ground will be used from real subsystem methods---don't track its state.

//         // SUBSYSTEM_ORDER name;
//         // STATE_ORDER state;
        
//         std::map<STATE_ORDER, double>& durations;
//         std::map<std::string, std::string>& flags;
//         bool is_active; // gets from Ticker

//     public:
//         Ground(
//             std::map<STATE_ORDER, double>& durations, 
//             std::map<std::string, std::string>& flags, 
//             std::string local_ip, 
//             unsigned short local_port,
//             std::string remote_ip, 
//             unsigned short remote_port, 
//             boost::asio::io_context& io_context
//         );

//         Ground(
//             Ground& ground, 
//             boost::asio::io_context& io_context
//         );

//         // inherit the UDPInterface
//         // send, recv, etc
//         // @todo multicast send

//         // in implementation of this, call the parent-defined recv method and pipe the results into the queue argument.
//         int recv(std::vector<std::vector<char>>& q);
//         // similarly here.
//         int send(std::queue<std::vector<char>>& q);
//         int send(char buff[RECV_BUFF_LEN], Housekeeping& caller);
//             // Ground should recv from remote into <command queue> and send to remote from <downlink queue>
// };


// class Housekeeping: public UDPInterface::UDPInterface {
//     public:
//         SUBSYSTEM_ORDER name;
//         STATE_ORDER state;

//         Ground& ground;

//         std::map<STATE_ORDER, double>& durations;
//         std::map<std::string, std::string>& flags;
//         bool is_active; // gets from Ticker

//     public:
//         Housekeeping(
//             SUBSYSTEM_ORDER out_name, 
//             STATE_ORDER out_state, 
//             Ground& in_ground,
//             std::map<STATE_ORDER, double>& durations, 
//             std::map<std::string, std::string>& flags, 
//             std::string local_ip, 
//             unsigned short local_port,
//             std::string remote_ip, 
//             unsigned short remote_port, 
//             boost::asio::io_context& io_context
//         );

//         int recv(std::vector<std::vector<char>>& q);
//         int send(std::queue<std::vector<char>>& q);

// };

class PepperMill {
    public:
        boost::asio::ip::udp::socket local_udp_sock;
        boost::asio::ip::tcp::socket local_tcp_sock;

        boost::asio::ip::udp::endpoint remote_udp_endpoint;
        boost::asio::ip::tcp::endpoint remote_tcp_endpoint;

        std::vector<char> share_data;
        std::queue<char> ground_pipe;

        STATE_ORDER active_state;
        SUBSYSTEM_ORDER active_subsys;

    public:
        PepperMill(boost::asio::io_context& context);
        PepperMill(
            std::string local_ip,
            std::string remote_tcp_ip,
            std::string remote_udp_ip,
            unsigned short local_port,
            unsigned short remote_tcp_port,
            unsigned short remote_udp_port,
            boost::asio::io_context& context
        );

        void update(SUBSYSTEM_ORDER new_subsys, STATE_ORDER new_state);

        void handle_recv();
        void recv_tcp_fwd_udp();
        void send_tcp();
        void send_udp();
};

class CdTe: public TCPInterface::TCPInterface {
    public:
        SUBSYSTEM_ORDER name;
        boost::asio::ip::tcp::endpoint ground_endpoint;

        CdTe(
            SUBSYSTEM_ORDER sys_name, 
            STATE_ORDER initial_state, 
            std::string local_ip, 
            unsigned short local_port, 
            std::string remote_ip, 
            unsigned short remote_port, 
            boost::asio::ip::tcp::endpoint& ground, 
            boost::asio::io_context& io_context);
        ~CdTe();

        void forward_to_ground();
};

#endif