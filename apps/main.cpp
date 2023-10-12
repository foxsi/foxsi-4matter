#include "LineInterface.h"
#include "Buffers.h"
// #include "Metronome.h"
#include "Circle.h"
#include "Parameters.h"

#include "moodycamel/concurrentqueue.h"
#include <unordered_map>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <queue>


int main(int argc, char** argv) {

    boost::asio::io_context context;
    LineInterface lif(argc, argv, context);
    CommandDeck deck = lif.get_command_deck();

    System& cdte1 = deck.get_sys_for_name("cdte1");
    System& cdte2 = deck.get_sys_for_name("cdte2");
    System& cdte3 = deck.get_sys_for_name("cdte3");
    System& cdte4 = deck.get_sys_for_name("cdte4");
    System& cmos1 = deck.get_sys_for_name("cmos1");
    System& cmos2 = deck.get_sys_for_name("cmos2");

    // test:
    // std::unordered_map<System, uint8_t> test;
    // test.insert(std::make_pair(cdte1, 0x02));

    PacketFramer pf_cdte1(cdte1, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cdte2(cdte2, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cdte3(cdte3, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cdte4(cdte4, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cmos1_ql(cmos1, RING_BUFFER_TYPE_OPTIONS::QL);
    PacketFramer pf_cmos1_pc(cmos1, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cmos2_ql(cmos2, RING_BUFFER_TYPE_OPTIONS::QL);
    PacketFramer pf_cmos2_pc(cmos2, RING_BUFFER_TYPE_OPTIONS::PC);

    FramePacketizer fp_cdte1(cdte1, deck.get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte2(cdte2, deck.get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte3(cdte3, deck.get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte4(cdte4, deck.get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cmos1_ql(cmos1, deck.get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::QL);
    FramePacketizer fp_cmos1_pc(cmos1, deck.get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cmos2_ql(cmos2, deck.get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::QL);
    FramePacketizer fp_cmos2_pc(cmos2, deck.get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);

    std::queue<UplinkBufferElement> cdte1_uplink_queue;
    std::queue<UplinkBufferElement> cdte2_uplink_queue;
    std::queue<UplinkBufferElement> cdte3_uplink_queue;
    std::queue<UplinkBufferElement> cdte4_uplink_queue;
    std::queue<UplinkBufferElement> cmos1_uplink_queue;
    std::queue<UplinkBufferElement> cmos2_uplink_queue;

    auto cdte1_manager = std::make_shared<SystemManager>(cdte1, cdte1_uplink_queue);
    auto cdte2_manager = std::make_shared<SystemManager>(cdte2, cdte2_uplink_queue);
    auto cdte3_manager = std::make_shared<SystemManager>(cdte3, cdte3_uplink_queue);
    auto cdte4_manager = std::make_shared<SystemManager>(cdte4, cdte4_uplink_queue);
    auto cmos1_manager = std::make_shared<SystemManager>(cmos1, cmos1_uplink_queue);
    auto cmos2_manager = std::make_shared<SystemManager>(cmos2, cmos2_uplink_queue);

    cdte1_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cdte1);
    cdte1_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cdte1);
    cdte1_manager->add_timing(&lif.lookup_timing[cdte1]);
    cdte2_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cdte2);
    cdte2_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cdte2);
    cdte2_manager->add_timing(&lif.lookup_timing[cdte2]);
    cdte3_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cdte3);
    cdte3_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cdte3);
    cdte3_manager->add_timing(&lif.lookup_timing[cdte3]);
    cdte4_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cdte4);
    cdte4_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cdte4);
    cdte4_manager->add_timing(&lif.lookup_timing[cdte4]);

    cmos1_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cmos1_pc);
    cmos1_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::QL, &fp_cmos1_ql);
    cmos1_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cmos1_pc);
    cmos1_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::QL, &pf_cmos1_ql);
    cmos1_manager->add_timing(&lif.lookup_timing[cmos1]);
    cmos2_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cmos2_pc);
    cmos2_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::QL, &fp_cmos2_ql);
    cmos2_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cmos2_pc);
    cmos2_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::QL, &pf_cmos2_ql);
    cmos2_manager->add_timing(&lif.lookup_timing[cmos2]);

    std::vector<std::shared_ptr<SystemManager>> order;
    order.emplace_back(std::move(cdte1_manager));
    order.emplace_back(std::move(cmos1_manager));

    std::cout << "endpoints: \n";
    boost::asio::ip::udp::endpoint local_udp_end(
        boost::asio::ip::make_address(lif.local_address),
        deck.get_sys_for_name("gse").ethernet->port
    );
    boost::asio::ip::tcp::endpoint local_tcp_end(
        boost::asio::ip::make_address(lif.local_address),
        deck.get_sys_for_name("cdte1").ethernet->port
    );
    boost::asio::ip::udp::endpoint remote_udp_end(
        boost::asio::ip::make_address(deck.get_sys_for_name("gse").ethernet->address),
        deck.get_sys_for_name("gse").ethernet->port
    );
    boost::asio::ip::tcp::endpoint remote_tcp_end(
        boost::asio::ip::make_address(deck.get_sys_for_name("cdte1").ethernet->address),
        deck.get_sys_for_name("cdte1").ethernet->port
    );
    
    std::cout << "uplink: \n";
    auto new_uplink_buffer = std::make_shared<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>>();
    auto new_downlink_buffer = std::make_shared<moodycamel::ConcurrentQueue<DownlinkBufferElement>>();


    std::cout << "machine: \n";
    auto machine = std::make_shared<TransportLayerMachine>(
        local_udp_end,
        local_tcp_end,
        remote_udp_end,
        local_tcp_end,
        new_uplink_buffer,
        new_downlink_buffer,
        context
    );

    Circle loop(
        1.0,
        order,
        deck,
        machine,
        context
    );

    // loop.slowmo_gain = 10;

    std::cout <<"done\n";
    context.run();
    std::cout <<"doner\n";

    return 0;
}