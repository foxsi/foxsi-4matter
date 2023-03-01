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

#endif