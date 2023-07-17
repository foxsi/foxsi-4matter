#include "LineInterface.h"
#include "Subsystem.h"
#include "Fragmenter.h"
#include "Metronome.h"
#include "Commanding.h"
#include "Parameters.h"
#include <boost/asio.hpp>
#include <unordered_map>
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

    uint8_t gse_hex = 0x00;
    uint8_t spmu_hex = 0x08;

    std::cout << "found " << lif.unique_endpoints.size() << " unique endpoints\n";
    std::cout << "found " << lif.local_endpoints.size() << " local endpoints\n";

    local_udp_endpoint.address(boost::asio::ip::make_address(lif.local_address));
    local_udp_endpoint.port(lif.lookup_endpoints[gse_hex]->port);
    local_tcp_endpoint.address(boost::asio::ip::make_address(lif.local_address));
    local_tcp_endpoint.port(lif.lookup_endpoints[spmu_hex]->port);
    remote_udp_endpoint.address(boost::asio::ip::make_address(lif.lookup_endpoints[gse_hex]->address));
    remote_udp_endpoint.port(lif.lookup_endpoints[gse_hex]->port);
    remote_tcp_endpoint.address(boost::asio::ip::make_address(lif.lookup_endpoints[spmu_hex]->address));
    remote_tcp_endpoint.port(lif.lookup_endpoints[spmu_hex]->port);

    CommandDeck deck = lif.get_command_deck();

    deck.print();

    size_t frag_header_size = 6;
    size_t frag_packet_size = 100;
    Fragmenter frag(frag_packet_size, frag_header_size);

    TransportLayerMachine frmtr(
        local_udp_endpoint,
        local_tcp_endpoint,
        remote_udp_endpoint,
        remote_tcp_endpoint,
        context
    );

    std::cout << "initialized TransportLayerMachine\n";

    // TransportLayerMachine frmtr(
    //     lif.local_address,              // IP address of this computer
    //     lif.endpoints["spmu"].address,  // IP of the remote TCP computer (to listen to)
    //     lif.endpoints["gse"].address,   // IP of the remote UDP computer (to send to)
    //     lif.endpoints["spmu"].port,     // port number on this computer to listen for TCP on
    //     lif.endpoints["spmu"].port,     // port number on the remote TCP computer
    //     lif.endpoints["gse"].port,      // port number on the remote UDP computer (to send to)
    //     context
    // );

    // TransportLayerMachine frmtr(
    //     lif.local_address,              // IP address of this computer
    //     lif.endpoints["spmu"].address,  // IP of the remote TCP computer (to listen to)
    //     lif.endpoints["gse"].address,   // IP of the remote UDP computer (to send to)
    //     lif.endpoints["gse"].port,     // port number on this computer to listen for TCP or UDP on
    //     lif.endpoints["spmu"].port,     // port number on the remote TCP computer
    //     lif.endpoints["gse"].port,      // port number on the remote UDP computer (to send to)
    //     context
    // );

    // TransportLayerMachine frmtr(
    //     "192.168.1.8",      // IP address of this computer
    //     "192.168.1.100",    // IP of the remote TCP computer (to listen to)
    //     "192.168.1.18",    // IP of the remote UDP computer (to send to)
    //     9999,     // port number on this computer to listen for TCP or UDP on
    //     10030,     // port number on the remote TCP computer
    //     9998,      // port number on the remote UDP computer (to send to)
    //     context
    // );

    // add the command deck:
    frmtr.add_commands(deck);

    // add the fragmenter to the deck:
    frmtr.add_fragmenter(frag);

    // display configuration:
    std::cout << "network setup:" << "\n";
    
    std::cout << "\tlocal TCP socket endpoint:\t\t" << frmtr.local_tcp_sock.local_endpoint().address().to_string() << ":" << std::to_string(frmtr.local_tcp_sock.local_endpoint().port()) << "\n";
    std::cout << "\tremote socket-bound TCP endpoint:\t" << frmtr.local_tcp_sock.remote_endpoint().address().to_string() << ":" << std::to_string(frmtr.local_tcp_sock.remote_endpoint().port()) << "\n";
    std::cout << "\tremote stored TCP endpoint:\t\t" << frmtr.remote_tcp_endpoint.address().to_string() << ":" << std::to_string(frmtr.remote_tcp_endpoint.port()) << "\n";
    std::cout << "\tlocal UDP socket endpoint:\t\t" << frmtr.local_udp_sock.local_endpoint().address().to_string() << ":" << std::to_string(frmtr.local_udp_sock.local_endpoint().port()) << "\n";
    std::cout << "\tremote stored UDP endpoint:\t\t" << frmtr.remote_udp_endpoint.address().to_string() << ":" << std::to_string(frmtr.remote_udp_endpoint.port()) << "\n";
    
    // give the io_context work to do:
    frmtr.recv_udp_fwd_tcp_cmd();
    frmtr.recv_tcp_fwd_udp();


    // while(1) {
    //     frmtr.print_udp_basic();
    // }
    
    

    // Metronome metronome(
    //     lif.times.period_seconds,
    //     context
    // );
    

    context.run();
    return 0;
}