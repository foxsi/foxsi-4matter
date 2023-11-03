// #include "LineInterface.h"
// #include "TransportLayer.h"
// #include "Metronome.h"
// #include "Commanding.h"
// #include "Parameters.h"
// #include <boost/asio.hpp>
// #include <unordered_map>
// #include <iostream>

int main(int argc, char* argv[]) {

    // boost::asio::io_context context;
    // LineInterface lif(argc, argv, context);

    // boost::asio::ip::udp::endpoint local_udp_endpoint;
    // boost::asio::ip::tcp::endpoint local_tcp_endpoint;
    // boost::asio::ip::udp::endpoint remote_udp_endpoint;
    // boost::asio::ip::tcp::endpoint remote_tcp_endpoint;

    // bool tcpb = 0;
    // bool udpb = 0;

    // local_udp_endpoint.address(boost::asio::ip::make_address(lif.local_address));
    // local_udp_endpoint.port(lif.endpoints["gse"].port);
    // remote_tcp_endpoint.address(boost::asio::ip::make_address(lif.endpoints["spmu"].address));
    // remote_tcp_endpoint.port(lif.endpoints["spmu"].port);


    // // TransportLayerMachine mill(
    // //     local_udp_endpoint,
    // //     local_tcp_endpoint,
    // //     remote_udp_endpoint,
    // //     remote_tcp_endpoint,
    // //     context
    // // );

    // TransportLayerMachine frmtr(
    //     lif.local_address,              // IP address of this computer
    //     lif.endpoints["spmu"].address,  // IP of the remote TCP computer (to listen to)
    //     lif.endpoints["gse"].address,   // IP of the remote UDP computer (to send to)
    //     lif.endpoints["spmu"].port,     // port number on this computer to listen for TCP on
    //     lif.endpoints["spmu"].port,     // port number on the remote TCP computer
    //     lif.endpoints["gse"].port,      // port number on the remote UDP computer (to send to)
    //     context
    // );

    // frmtr.recv_tcp_fwd_udp();
    // frmtr.recv_udp_fwd_tcp();

    // // Metronome metronome(
    // //     lif.times.period_seconds,
    // //     context
    // // );
    

    // context.run();
    return 0;
}