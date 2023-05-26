#include "TCPInterface.h"
#include <memory>

TCPSession::TCPSession(boost::asio::ip::tcp::socket socket): 
    local_socket(std::move(socket)) {

}

void TCPSession::read() {
    auto self(shared_from_this());
    local_socket.async_read_some(
        boost::asio::buffer(data, RECV_BUFF_LEN), 
        [this,self](boost::system::error_code ec, std::size_t length) {
            if(!ec) {
                TCPSession::write(length);

            }
        }
    );
}

void TCPSession::write(std::size_t length) {
    auto self(shared_from_this());
    boost::asio::async_write(
        local_socket, 
        boost::asio::buffer(data, RECV_BUFF_LEN), 
        [this,self](boost::system::error_code ec, std::size_t length) {
            if(!ec) {
                TCPSession::read();
            }
        }
    );
}



TCPServer::TCPServer(boost::asio::ip::tcp::endpoint endpoint, boost::asio::io_context& io_context): acceptor(io_context, endpoint) {
    accept(io_context);
}

void TCPServer::accept(boost::asio::io_context& io_context) {
    acceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if(!ec) {
            std::make_shared<TCPSession>(std::move(socket))->read();
        }
        }
    );
}

TCPInterface::TCPInterface(
    std::string local_ip, 
    unsigned short local_port, 
    std::string remote_ip, 
    unsigned short remote_port, 
    boost::asio::io_context& io_context
    ): 
    local_socket(io_context)//,
    // strand(boost::asio::make_strand(local_socket.get_executor())),
    // recv_deadline(strand),
    // send_deadline(strand)
    {
    local_address = boost::asio::ip::make_address(local_ip);
    remote_address = boost::asio::ip::make_address(remote_ip);

    local_endpoint = boost::asio::ip::tcp::endpoint(local_address, local_port);
    remote_endpoint = boost::asio::ip::tcp::endpoint(remote_address, remote_port);

    // the tricks for UDP don't work here.
    // See this example: https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp
    // Need to make an acceptor, then another thread for async send/receive.

    local_socket.open(boost::asio::ip::tcp::v4());
    // confirm TCP connect flow:
    local_socket.connect(remote_endpoint);
    TCPInterface::recv();
}

int TCPInterface::recv(uint8_t* addr, uint8_t* buffer) {

}

int TCPInterface::async_recv(uint8_t* addr, uint8_t* buffer) {
    
    return 0;
}

int TCPInterface::send(uint8_t* addr, uint8_t* buffer) {

}

int TCPInterface::async_send(uint8_t* addr, uint8_t* buffer) {
    
    return 0;
}

void TCPInterface::recv() {
    const std::string msg_str = "hello, it's the formatter\n";
    const uint8_t* msg = reinterpret_cast<const uint8_t*>(&msg_str[0]);
    std::size_t len = msg_str.length();
    uint8_t recv_buff[RECV_BUFF_LEN];

    // memset(recvbuf, 0, sizeof(recvbuf));

    local_socket.receive(boost::asio::buffer(recv_buff, RECV_BUFF_LEN));

    std::cout << recv_buff << "\n";
    TCPInterface::send(msg, len);
}

void TCPInterface::send(const uint8_t* buffer, std::size_t len) {
    std::cout << "sending" << "\n";
    local_socket.send(boost::asio::buffer(buffer, len));
    TCPInterface::recv();
}