#include "LineInterface.h"
#include "TransportLayer.h"
#include "Fragmenter.h"
#include "RingBufferInterface.h"
#include "Metronome.h"
#include "Commanding.h"
#include "Systems.h"
#include "Buffers.h"
#include "Parameters.h"
#include <boost/asio.hpp>
#include <unordered_map>
#include <iostream>
#include <queue>
#include <memory>
#include "moodycamel/concurrentqueue.h"

int main(int argc, char* argv[]) {

    boost::asio::io_context context;
    LineInterface lif(argc, argv, context);

    boost::asio::ip::udp::endpoint local_udp_endpoint;
    boost::asio::ip::tcp::endpoint local_tcp_endpoint;
    boost::asio::ip::udp::endpoint remote_udp_endpoint;
    boost::asio::ip::tcp::endpoint remote_tcp_endpoint;

    bool tcpb = 0;
    bool udpb = 0;

    auto deck = std::make_shared<CommandDeck>(lif.get_command_deck());

    uint8_t gse_hex = 0x00;
    uint8_t spmu_hex = 0x08;

    std::cout << "found " << lif.unique_endpoints.size() << " unique endpoints\n";
    std::cout << "found " << lif.local_endpoints.size() << " local endpoints\n";

    local_udp_endpoint.address(boost::asio::ip::make_address(lif.local_address));
    // local_udp_endpoint.port(lif.lookup_endpoints[gse_hex]->port);
    local_udp_endpoint.port(deck->get_sys_for_code(gse_hex).ethernet->port);
    local_tcp_endpoint.address(boost::asio::ip::make_address(lif.local_address));
    // local_tcp_endpoint.port(lif.lookup_endpoints[spmu_hex]->port);
    local_tcp_endpoint.port(deck->get_sys_for_code(spmu_hex).ethernet->port);
    // remote_udp_endpoint.address(boost::asio::ip::make_address(lif.lookup_endpoints[gse_hex]->address));
    remote_udp_endpoint.address(boost::asio::ip::make_address(deck->get_sys_for_code(gse_hex).ethernet->address));
    // remote_udp_endpoint.port(lif.lookup_endpoints[gse_hex]->port);
    remote_udp_endpoint.port(deck->get_sys_for_code(gse_hex).ethernet->port);
    // remote_tcp_endpoint.address(boost::asio::ip::make_address(lif.lookup_endpoints[spmu_hex]->address));
    remote_udp_endpoint.address(boost::asio::ip::make_address(deck->get_sys_for_code(spmu_hex).ethernet->address));
    // remote_tcp_endpoint.port(lif.lookup_endpoints[spmu_hex]->port);
    remote_udp_endpoint.port(deck->get_sys_for_code(spmu_hex).ethernet->port);

    

    deck->print();

    // todo: add these to foxsi4-commands/systems.json for the GSE Ethernet interfaces (take minimum of all options):
    size_t frag_header_size = 6;
    size_t frag_packet_size = 100;
    Fragmenter frag(frag_packet_size, frag_header_size);

    std::cout << "initialized fragmenter\n";

    // todo: add these to foxsi4-commands/systems.json for each system with a ring buffer.
    uint32_t cdte_ring_start_addr = 0x00400000;
    size_t cdte_ring_size = 32124400;
    // size_t cdte_ring_block_size = 32780;
    // testing more, smaller blocks:
    size_t cdte_ring_block_size = 2000;
    size_t cdte_read_block_count = 17;

    uint32_t cmos_ring_start_addr = 0x00001000;
    // size_t cmos_ring_size = 0x041e0000; // total ring size
    size_t cmos_ring_size = 0x001e0014;
    // size_t cmos_ring_block_size = 0x00220000;
    size_t cmos_ring_block_size = 2000;
    size_t cmos_read_block_count = 984;

    RingBufferInterface cdte_rbif = RingBufferInterface(cdte_ring_start_addr, cdte_ring_size, cdte_ring_block_size, cdte_read_block_count);

    RingBufferInterface cmos_rbif = RingBufferInterface(cmos_ring_start_addr, cmos_ring_size, cmos_ring_block_size, cmos_read_block_count);

    std::cout << "initialized ring buffer interface\n";

    std::unordered_map<uint8_t, RingBufferInterface> rbif_map;
    rbif_map[0x08] = cdte_rbif;
    rbif_map[0x09] = cdte_rbif;
    rbif_map[0x0a] = cdte_rbif;
    rbif_map[0x0b] = cdte_rbif;
    rbif_map[0x0c] = cdte_rbif;
    rbif_map[0x0e] = cmos_rbif;
    rbif_map[0x0f] = cmos_rbif;

    std::cout << "added ring buffer interface to map\n";

    auto temp_map = std::make_shared<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>>();
    auto temp_dbuf = std::make_shared<moodycamel::ConcurrentQueue<DownlinkBufferElement>>();

    TransportLayerMachine frmtr(
        local_udp_endpoint,
        local_tcp_endpoint,
        remote_udp_endpoint,
        remote_tcp_endpoint,
        temp_map,
        temp_dbuf,
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

    // add the packet fragmenter:
    frmtr.add_fragmenter(frag);

    // add the ring buffer interface:
    frmtr.add_ring_buffer_interface(rbif_map);

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

    // Metronome metronome(
    //     lif.times.period_seconds,
    //     context
    // );
    

    context.run();
    return 0;
}