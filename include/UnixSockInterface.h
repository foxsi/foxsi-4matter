#ifndef UNIXSOCKINTERFACE_H
#define UNIXSOCKINTERFACE_H

#include "AbstractSerial.h"
#include <boost/asio.hpp>
#include <string>

/* This is for datagram-oriented sockets. Prefer to send/receive full packet at once. */
class UnixDomainSocketInterface: AbstractSerial {
    public:
        std::string endpoint_name;
        
        boost::asio::local::datagram_protocol::socket local_socket;
        boost::asio::local::datagram_protocol::socket remote_socket;

        UnixDomainSocketInterface(std::string endpoint, boost::asio::io_context& context);
        ~UnixDomainSocketInterface();

        int recv(uint8_t* addr, uint8_t* buffer);
        int async_recv(uint8_t* addr, uint8_t* buffer);

        int send(uint8_t* addr, uint8_t* data);
        int async_send(uint8_t* addr, uint8_t* data);

        // for test
        void recv();
        void send(uint8_t* buff, std::size_t len);
};

#endif