#include "LineInterface.h"
#include "Buffers.h"
#include "Circle.h"
#include "Parameters.h"
#include "Utilities.h"

#include "moodycamel/concurrentqueue.h"

#include <unordered_map>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <queue>


int main(int argc, char** argv) {
    utilities::setup_logs_nowtime("log/");

    utilities::debug_log("main check debug log");
    utilities::error_log("main check error log");

    boost::asio::io_context context;
    boost::asio::io_context circle_timer_context;
    LineInterface lif(argc, argv, context);
    bool do_uart = lif.do_uart;
    auto deck = std::make_shared<CommandDeck>(lif.get_command_deck());

    System& housekeeping = deck->get_sys_for_name("housekeeping");
    System& cdtede = deck->get_sys_for_name("cdtede");
    System& cdte1 = deck->get_sys_for_name("cdte1");
    System& cdte2 = deck->get_sys_for_name("cdte2");
    System& cdte3 = deck->get_sys_for_name("cdte3");
    System& cdte4 = deck->get_sys_for_name("cdte4");
    System& cmos1 = deck->get_sys_for_name("cmos1");
    System& cmos2 = deck->get_sys_for_name("cmos2");

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

    housekeeping_manager->add_timing(&lif.lookup_timing[housekeeping]);
    cdtede_manager->add_timing(&lif.lookup_timing[cdtede]);
    cdte1_manager->add_timing(&lif.lookup_timing[cdte1]);
    cdte2_manager->add_timing(&lif.lookup_timing[cdte2]);
    cdte3_manager->add_timing(&lif.lookup_timing[cdte3]);
    cdte4_manager->add_timing(&lif.lookup_timing[cdte4]);
    cmos1_manager->add_timing(&lif.lookup_timing[cmos1]);
    cmos2_manager->add_timing(&lif.lookup_timing[cmos2]);

    std::cout << "Timing for cdtede: " << cdtede_manager->timing->to_string() << "\n";

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
    order.emplace_back(std::move(cdte2_manager));
    order.emplace_back(std::move(cdte3_manager));
    order.emplace_back(std::move(cdte4_manager));
    order.emplace_back(std::move(cdtede_manager));
    order.emplace_back(std::move(cmos1_manager));
    order.emplace_back(std::move(cmos2_manager));
    order.emplace_back(std::move(housekeeping_manager));
    // order.emplace_back(std::move(cmos1_manager));

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
    // std::cout << "\t local address: ";
    // std::cout << lif.local_address << "\n";
    
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

    auto new_downlink_buffer = std::make_shared<moodycamel::ConcurrentQueue<DownlinkBufferElement>>();

    std::cout << "uart: \n";
    if (do_uart) {
        if (deck->get_sys_for_name("timepix").uart) {
            std::cout << "timepix has uart interface\n";
        } else {
            std::cout << "timepix has no uart interface!\n";
        }
        std::cout << "uart interface: " << deck->get_sys_for_name("timepix").uart->to_string();
    }


    std::cout << "machine: \n";
    try {
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

        std::cout << "loop: \n";
        Circle loop(
            2.0,
            order,
            deck,
            machine,
            circle_timer_context
        );

        loop.slowmo_gain = 1;

        // std::cout << "async udp listen: \n";

        // machine->async_udp_receive_to_uplink_buffer();
        // machine->async_udp_send_downlink_buffer();
        
        // debug:
        // machine->recv_udp_fwd_tcp_cmd();
        // machine->recv_tcp_fwd_udp();

        std::cout <<"setup done\n";
        context.poll();
        circle_timer_context.run();
    } catch (std::exception& e) {
        std::cout << e.what() << "\n";
    }
    std::cout <<"exiting\n";

    return 0;
}