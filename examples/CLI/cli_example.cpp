#include "LineInterface.h"
#include "Subsystem.h"
#include <boost/asio.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    boost::asio::io_context context;
    LineInterface lif(argc, argv, context);

    boost::asio::ip::udp::endpoint local_udp_endpoint;
    boost::asio::ip::tcp::endpoint local_tcp_endpoint;
    boost::asio::ip::udp::endpoint remote_udp_endpoint;
    boost::asio::ip::tcp::endpoint remote_tcp_endpoint;

    bool tcpb = 0;
    bool udpb = 0;
    // loop over all peripherals

    for(int i = 0; i<lif.endpoints.size(); i++) {
        if(lif.endpoints[i].protocol.compare("udp") == 0) {
            local_udp_endpoint.address(boost::asio::ip::make_address(lif.endpoints[i].address));
            local_udp_endpoint.port(lif.endpoints[i].port);
        } else if (lif.endpoints[i].protocol.compare("tcp") == 0) {
            local_tcp_endpoint.address(boost::asio::ip::make_address(lif.endpoints[i].address));
            local_tcp_endpoint.port(lif.endpoints[i].port);
        } else {
            std::cout << "undefined protocol found\n";
        }
    }

    PepperMill mill(
        local_udp_endpoint,
        local_tcp_endpoint,
        remote_udp_endpoint,
        remote_tcp_endpoint,
        context
    );
    

    context.run();
    return 0;
}