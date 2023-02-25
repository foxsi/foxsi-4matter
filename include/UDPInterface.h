#ifndef UDPINTERFACE_H
#define UDPINTERFACE_H

#include "AbstractSerial.h"
#include <boost/asio.hpp>

class UDPInterface: AbstractSerial {
    public:
        boost::asio::ip::address local_address;
        unsigned short local_port;
        boost::asio::ip::address remote_address;
        unsigned short remote_port;

        boost::asio::ip::udp::endpoint local_endpoint;
        boost::asio::ip::udp::endpoint remote_endpoint;

        boost::asio::ip::udp::socket local_socket;
        
        // add share memory interface attribute

        UDPInterface(std::string local_ip, unsigned short local_port, std::string remote_ip, unsigned short remote_port, boost::asio::io_context& io_context);

        // ~UDPInterface();

        int recv(uint8_t* addr, char* buffer);
        int async_recv(uint8_t* addr, char* buffer);

        int send(uint8_t* addr, char* buffer);
        int async_send(uint8_t* addr, char* buffer);

        void async_send(char* buffer, std::size_t length);
};

#endif