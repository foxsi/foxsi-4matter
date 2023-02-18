#ifndef TCPINTERFACE_H
#define TCPINTERFACE_H

#include "AbstractSerial.h"
#include <boost/asio.hpp>

class TCPInterface: AbstractSerial {
    public:
        boost::asio::ip::address local_address;
        unsigned short local_port;
        boost::asio::ip::address remote_address;
        unsigned short remote_port;

        boost::asio::ip::tcp::endpoint local_endpoint;
        boost::asio::ip::tcp::endpoint remote_endpoint;

        boost::asio::ip::tcp::socket local_socket;
        
        // add share memory interface attribute

        TCPInterface(std::string local_ip, unsigned short local_port, std::string remote_ip, unsigned short remote_port, boost::asio::io_context& io_context);

        ~TCPInterface();

        int recv(uint8_t* addr, char* buffer);
        int async_recv(uint8_t* addr, char* buffer);

        int send(uint8_t* addr, char* buffer);
        int async_send(uint8_t* addr, char* buffer);
};

#endif