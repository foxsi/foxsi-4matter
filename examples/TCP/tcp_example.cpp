
#include "TCPInterface.h"
#include <boost/asio.hpp>
#include <iostream>

int main() {
    boost::asio::io_context context;

    boost::asio::ip::address address(boost::asio::ip::make_address("192.168.1.8"));
    boost::asio::ip::tcp::endpoint endpoint(address, 9000);
    TCPServer server(endpoint, context);
    context.run();

    return 0;
}