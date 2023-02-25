#ifndef TCPINTERFACE_H
#define TCPINTERFACE_H

#include "Parameters.h"
#include "AbstractSerial.h"
#include "UDPInterface.h"
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/bind/bind.hpp>
#include <fstream>
#include <deque>

// see https://stackoverflow.com/questions/66875712/how-to-create-boostasio-server-that-listen-to-two-different-ports

// like TCP Session
class TCPSession: public std::enable_shared_from_this<TCPSession> {
    public:
        boost::asio::ip::tcp::socket local_socket;
        char data[RECV_BUFF_LEN];
        // UDPInterface udpif;

        TCPSession(boost::asio::ip::tcp::socket socket);
        void read();
        void write(std::size_t length);
};

class TCPServer {
    public:
        boost::asio::ip::tcp::acceptor acceptor;

        TCPServer(boost::asio::ip::tcp::endpoint endpoint, boost::asio::io_context& io_context);

        void accept(boost::asio::io_context& io_context);
};

// like TCP Server
class TCPInterface {
    public:
        boost::asio::ip::address local_address;
        unsigned short local_port;
        boost::asio::ip::address remote_address;
        unsigned short remote_port;

        boost::asio::ip::tcp::endpoint local_endpoint;
        boost::asio::ip::tcp::endpoint remote_endpoint;

        boost::asio::ip::tcp::socket local_socket;

        // boost::asio::ip::tcp::acceptor acceptor;

        // boost::asio::strand<boost::asio::ip::tcp::socket::executor_type> strand;
        // boost::asio::steady_timer recv_deadline;
        // boost::asio::steady_timer send_deadline;
        
        // add share memory interface attribute

        TCPInterface(
            std::string local_ip, 
            unsigned short local_port, 
            std::string remote_ip, 
            unsigned short remote_port, 
            boost::asio::io_context& io_context
        );

        // ~TCPInterface();

        // void accept();

        int recv(uint8_t* addr, char* buffer);
        int async_recv(uint8_t* addr, char* buffer);

        int send(uint8_t* addr, char* buffer);
        int async_send(uint8_t* addr, char* buffer);

        void recv();
        void send(const char* buffer, std::size_t len);
};



#endif