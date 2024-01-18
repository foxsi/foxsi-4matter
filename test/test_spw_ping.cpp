#include "LineInterface.h"
#include "TransportLayer.h"
#include "Utilities.h"

#include <boost/asio.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    boost::asio::io_context context;

    LineInterface lif(argc, argv, context);
    auto deck = std::make_shared<CommandDeck>(lif.get_command_deck());

    System& housekeeping = deck->get_sys_for_name("housekeeping");
    System& cdtede = deck->get_sys_for_name("cdtede");
    System& cdte1 = deck->get_sys_for_name("cdte1");
    System& cdte2 = deck->get_sys_for_name("cdte2");
    System& cdte3 = deck->get_sys_for_name("cdte3");
    System& cdte4 = deck->get_sys_for_name("cdte4");
    System& cmos1 = deck->get_sys_for_name("cmos1");
    System& cmos2 = deck->get_sys_for_name("cmos2");

    PacketFramer pf_cdte1(cdte1, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cdte2(cdte2, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cdte3(cdte3, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cdte4(cdte4, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cmos1_ql(cmos1, RING_BUFFER_TYPE_OPTIONS::QL);
    PacketFramer pf_cmos1_pc(cmos1, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cmos2_ql(cmos2, RING_BUFFER_TYPE_OPTIONS::QL);
    PacketFramer pf_cmos2_pc(cmos2, RING_BUFFER_TYPE_OPTIONS::PC);

    FramePacketizer fp_cdte1(cdte1, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte2(cdte2, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte3(cdte3, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte4(cdte4, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cmos1_ql(cmos1, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::QL);
    FramePacketizer fp_cmos1_pc(cmos1, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cmos2_ql(cmos2, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::QL);
    FramePacketizer fp_cmos2_pc(cmos2, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);

    std::queue<UplinkBufferElement> housekeeping_uplink_queue;
    std::queue<UplinkBufferElement> cdtede_uplink_queue;
    std::queue<UplinkBufferElement> cdte1_uplink_queue;
    std::queue<UplinkBufferElement> cdte2_uplink_queue;
    std::queue<UplinkBufferElement> cdte3_uplink_queue;
    std::queue<UplinkBufferElement> cdte4_uplink_queue;
    std::queue<UplinkBufferElement> cmos1_uplink_queue;
    std::queue<UplinkBufferElement> cmos2_uplink_queue;

    auto housekeeping_manager = std::make_shared<SystemManager>(housekeeping, housekeeping_uplink_queue);
    auto cdtede_manager = std::make_shared<SystemManager>(cdtede, cdtede_uplink_queue);
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

    std::cout << "endpoints: \n";
    boost::asio::ip::udp::endpoint local_udp_end(
        boost::asio::ip::make_address(lif.local_address),
        deck->get_sys_for_name("gse").ethernet->port
    );
    boost::asio::ip::tcp::endpoint local_tcp_end(
        boost::asio::ip::make_address(lif.local_address),
        deck->get_sys_for_name("cdte1").ethernet->port
    );
    boost::asio::ip::tcp::endpoint local_tcp_housekeeping_end(
        boost::asio::ip::make_address(lif.local_address),
        deck->get_sys_for_name("housekeeping").ethernet->port
    );
    boost::asio::ip::udp::endpoint remote_udp_end(
        boost::asio::ip::make_address(deck->get_sys_for_name("gse").ethernet->address),
        deck->get_sys_for_name("gse").ethernet->port
    );
    boost::asio::ip::tcp::endpoint remote_tcp_end(
        boost::asio::ip::make_address(deck->get_sys_for_name("cdte1").ethernet->address),
        deck->get_sys_for_name("cdte1").ethernet->port
    );
    boost::asio::ip::tcp::endpoint remote_tcp_housekeeping_end(
        boost::asio::ip::make_address(deck->get_sys_for_name("housekeeping").ethernet->address),
        deck->get_sys_for_name("housekeeping").ethernet->port
    );


    std::cout << "uplink: \n";
    auto new_uplink_buffer = std::make_shared<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>>();
    // add keys to the uplink buffer map:
    (*new_uplink_buffer)[deck->get_sys_for_name("housekeeping")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cdtede")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cdte1")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cdte2")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cdte3")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cdte4")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cmos1")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cmos2")];
    
    std::cout << "downlink: \n";
    auto new_downlink_buffer = std::make_shared<moodycamel::ConcurrentQueue<DownlinkBufferElement>>();

    std::cout << "machine: \n";
    auto machine = std::make_shared<TransportLayerMachine>(
        local_udp_end,
        local_tcp_end,
        local_tcp_housekeeping_end,
        remote_udp_end,
        remote_tcp_end,
        remote_tcp_housekeeping_end,
        new_uplink_buffer,
        new_downlink_buffer,
        context
    );
    machine->add_commands(deck);

    // testing:
    System test_system = deck->get_sys_for_name(lif.get_test_system_name());
    std::queue<UplinkBufferElement> q;
    SystemManager test_system_manager(test_system, q);

    uint8_t test_command = 0x00;
    if (lif.get_test_system_name().find("cmos") != std::string::npos) {
        test_command = 0xa8;
    } else if (lif.get_test_system_name().find("cdte") != std::string::npos) {
        test_command = 0x8a;
    } else {
        utilities::error_print("don't have debug command for requested system " + lif.get_test_system_name() + "!\n");
    }

    std::cout << "testing system " << test_system.name << " with command " << std::to_string(test_command) << "\n";
    std::cout << "\tgetting command to send...\n";
    
    Command send_command(deck->get_command_for_sys_for_code(test_system.hex, test_command));
    std::cout << "\tsending request...\n";

    std::vector<uint8_t> reply = machine->sync_tcp_send_command_for_sys(test_system_manager, send_command);
    
    std::vector<uint8_t> reply_data = machine->get_reply_data(reply, test_system.hex);
    
    std::cout << "\tgot reply: ";
    utilities::spw_print(reply, nullptr);

    return 0;
}