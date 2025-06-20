/**
 * @file main.cpp
 * @author Thanasi Pantazides
 * @brief Main entry point for the `formatter` application.
 * @version v1.0.1
 * @date 2024-03-07
 * 
 */

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
    System& cdte5 = deck->get_sys_for_name("cdte5");
    System& cdte3 = deck->get_sys_for_name("cdte3");
    System& cdte4 = deck->get_sys_for_name("cdte4");
    System& cmos1 = deck->get_sys_for_name("cmos1");
    System& cmos2 = deck->get_sys_for_name("cmos2");
    System& timepix = deck->get_sys_for_name("timepix");
    System& uplink = deck->get_sys_for_name("uplink");

    // test:
    // std::unordered_map<System, uint8_t> test;
    // test.insert(std::make_pair(cdte1, 0x02));

    PacketFramer pf_cdte1_pc(cdte1, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cdte1_hk(cdte1, RING_BUFFER_TYPE_OPTIONS::HK);
    PacketFramer pf_cdte5(cdte5, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cdte3(cdte3, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cdte4(cdte4, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cmos1_ql(cmos1, RING_BUFFER_TYPE_OPTIONS::QL);
    PacketFramer pf_cmos1_pc(cmos1, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cmos1_hk(cmos1, RING_BUFFER_TYPE_OPTIONS::HK);
    PacketFramer pf_cmos2_ql(cmos2, RING_BUFFER_TYPE_OPTIONS::QL);
    PacketFramer pf_cmos2_pc(cmos2, RING_BUFFER_TYPE_OPTIONS::PC);

    FramePacketizer fp_cdte1_pc(cdte1, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte1_hk(cdte1, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::HK);
    FramePacketizer fp_cdte5(cdte5, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte3(cdte3, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte4(cdte4, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cmos1_ql(cmos1, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::QL);
    FramePacketizer fp_cmos1_pc(cmos1, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cmos1_hk(cmos1, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::HK);
    FramePacketizer fp_cmos2_ql(cmos2, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::QL);
    FramePacketizer fp_cmos2_pc(cmos2, deck->get_sys_for_name("gse"), RING_BUFFER_TYPE_OPTIONS::PC);

    std::queue<UplinkBufferElement> housekeeping_uplink_queue;
    std::queue<UplinkBufferElement> cdtede_uplink_queue;
    std::queue<UplinkBufferElement> cdte1_uplink_queue;
    std::queue<UplinkBufferElement> cdte5_uplink_queue;
    std::queue<UplinkBufferElement> cdte3_uplink_queue;
    std::queue<UplinkBufferElement> cdte4_uplink_queue;
    std::queue<UplinkBufferElement> cmos1_uplink_queue;
    std::queue<UplinkBufferElement> cmos2_uplink_queue;
    std::queue<UplinkBufferElement> timepix_uplink_queue;
    
    std::queue<UplinkBufferElement> global_uplink_queue;

    auto housekeeping_manager = std::make_shared<SystemManager>(housekeeping, housekeeping_uplink_queue);
    auto cdtede_manager = std::make_shared<SystemManager>(cdtede, cdtede_uplink_queue);
    auto cdte1_manager = std::make_shared<SystemManager>(cdte1, cdte1_uplink_queue);
    auto cdte5_manager = std::make_shared<SystemManager>(cdte5, cdte5_uplink_queue);
    auto cdte3_manager = std::make_shared<SystemManager>(cdte3, cdte3_uplink_queue);
    auto cdte4_manager = std::make_shared<SystemManager>(cdte4, cdte4_uplink_queue);
    auto cmos1_manager = std::make_shared<SystemManager>(cmos1, cmos1_uplink_queue);
    auto cmos2_manager = std::make_shared<SystemManager>(cmos2, cmos2_uplink_queue);
    auto timepix_manager = std::make_shared<SystemManager>(timepix, timepix_uplink_queue);
    auto uplink_manager = std::make_shared<SystemManager>(uplink, global_uplink_queue);

    housekeeping_manager->add_timing(&lif.lookup_timing[housekeeping]);
    cdtede_manager->add_timing(&lif.lookup_timing[cdtede]);
    cdte1_manager->add_timing(&lif.lookup_timing[cdte1]);
    cdte5_manager->add_timing(&lif.lookup_timing[cdte5]);
    cdte3_manager->add_timing(&lif.lookup_timing[cdte3]);
    cdte4_manager->add_timing(&lif.lookup_timing[cdte4]);
    cmos1_manager->add_timing(&lif.lookup_timing[cmos1]);
    cmos2_manager->add_timing(&lif.lookup_timing[cmos2]);
    timepix_manager->add_timing(&lif.lookup_timing[timepix]);
    
    uplink_manager->add_timing(&lif.lookup_timing[uplink]);

    std::cout << "Timing for uplink: " << uplink_manager->timing->to_string() << "\n";

    cdte1_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cdte1_pc);
    cdte1_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cdte1_pc);
    cdte1_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::HK, &fp_cdte1_hk);
    cdte1_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::HK, &pf_cdte1_hk);
    cdte1_manager->add_timing(&lif.lookup_timing[cdte1]);
    cdte5_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cdte5);
    cdte5_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cdte5);
    cdte5_manager->add_timing(&lif.lookup_timing[cdte5]);
    cdte3_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cdte3);
    cdte3_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cdte3);
    cdte3_manager->add_timing(&lif.lookup_timing[cdte3]);
    cdte4_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cdte4);
    cdte4_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cdte4);
    cdte4_manager->add_timing(&lif.lookup_timing[cdte4]);

    cmos1_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cmos1_pc);
    cmos1_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::QL, &fp_cmos1_ql);
    cmos1_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::HK, &fp_cmos1_hk);
    cmos1_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cmos1_pc);
    cmos1_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::QL, &pf_cmos1_ql);
    cmos1_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::HK, &pf_cmos1_hk);
    cmos1_manager->add_timing(&lif.lookup_timing[cmos1]);
    cmos2_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::PC, &fp_cmos2_pc);
    cmos2_manager->add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS::QL, &fp_cmos2_ql);
    cmos2_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::PC, &pf_cmos2_pc);
    cmos2_manager->add_packet_framer(RING_BUFFER_TYPE_OPTIONS::QL, &pf_cmos2_ql);
    cmos2_manager->add_timing(&lif.lookup_timing[cmos2]);

    std::vector<std::shared_ptr<SystemManager>> order;
    order.emplace_back(std::move(cdte1_manager));
    order.emplace_back(std::move(cdte5_manager));
    order.emplace_back(std::move(cmos1_manager));
    order.emplace_back(std::move(cdtede_manager));
    order.emplace_back(std::move(housekeeping_manager));
    order.emplace_back(std::move(cdte3_manager));
    order.emplace_back(std::move(cdte4_manager));
    order.emplace_back(std::move(cmos2_manager));
    order.emplace_back(std::move(timepix_manager));
    order.emplace_back(std::move(uplink_manager));      // added uplink_manager to the loop order so it is accessible inside Circle.
    // order.emplace_back(std::move(cmos1_manager));

    DownlinkBufferElement pcdbe(&cdte1, &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::PC);
    std::cout << pcdbe.to_string() << "\n";
    std::vector<uint8_t> fake(2000);
    fp_cdte1_pc.set_frame( fake);
    std::cout << "set frame\n";
    DownlinkBufferElement othcdbe(fp_cdte1_pc.pop_buffer_element());
    std::cout << othcdbe.to_string() << "\n";


    std::cout << "endpoints: \n";
    boost::asio::ip::udp::endpoint local_udp_end(
        boost::asio::ip::make_address(lif.local_address),
        deck->get_sys_for_name("gse").ethernet->port
    );
    boost::asio::ip::tcp::endpoint local_tcp_end(
        boost::asio::ip::make_address(lif.local_address),
        deck->get_sys_for_name("cdte1").ethernet->port
    );
    boost::asio::ip::udp::endpoint local_udp_housekeeping_end(
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
    boost::asio::ip::udp::endpoint remote_udp_housekeeping_end(
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
    (*new_uplink_buffer)[deck->get_sys_for_name("cdte5")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cdte3")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cdte4")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cmos1")];
    (*new_uplink_buffer)[deck->get_sys_for_name("cmos2")];
    (*new_uplink_buffer)[deck->get_sys_for_name("timepix")];
    // note: uplink system itself is not in the uplink buffer. Just there to manage access.

    auto new_downlink_buffer = std::make_shared<moodycamel::ConcurrentQueue<DownlinkBufferElement>>();

    std::cout << "uart: \n";
    if (deck->get_sys_for_name("timepix").uart) {
        std::cout << "timepix has uart interface\n";
        std::cout << "uart interface: " << deck->get_sys_for_name("timepix").uart->to_string();
    } else {
        std::cout << "timepix has no uart interface!\n";
    }
    if (deck->get_sys_for_name("uplink").uart) {
        std::cout << "uplink has uart interface\n";
        std::cout << "uart interface: " << deck->get_sys_for_name("uplink").uart->to_string();
    } else {
        std::cout << "uplink has no uart interface!\n";
    }


    std::cout << "machine: \n";
    utilities::debug_log("starting main()...");
    try {
        UART timepix_uart_pre = *(deck->get_sys_for_name("timepix").uart);
        auto timepix_uart = std::make_shared<UART>(*(deck->get_sys_for_name("timepix").uart));
        auto uplink_uart = std::make_shared<UART>(*(deck->get_sys_for_name("uplink").uart));


        auto machine = std::make_shared<TransportLayerMachine>(
            local_udp_end,
            local_tcp_end,
            local_udp_housekeeping_end,
            remote_udp_end,
            remote_tcp_end,
            remote_udp_housekeeping_end,
            new_uplink_buffer,
            new_downlink_buffer,
            timepix_uart,
            uplink_uart,
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

        std::cout <<"setup done\n";
        context.poll();
        // context.run();
        // circle_timer_context.run();
        loop.start();

    } catch (std::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << "trying to run again\n";
    }
    std::cout <<"exiting\n";

    return 0;
}