#include "LineInterface.h"
#include "TransportLayer.h"
#include "Parameters.h"

#include <boost/asio.hpp>

#include <memory>
#include <chrono>
#include <thread>
#include <iostream>

/**
 * A simple script to issue shutdown commands to connected systems.
 * 
*/
int main(int argc, char** argv) {

    auto delay = std::chrono::milliseconds(2000);

    boost::asio::io_context context;

    LineInterface lif(argc, argv, context);
    auto deck = std::make_shared<CommandDeck>(lif.get_command_deck());

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

    TransportLayerMachine machine(
        local_udp_end,
        local_tcp_end,
        local_tcp_housekeeping_end,
        remote_udp_end,
        remote_tcp_end,
        remote_tcp_housekeeping_end,
        {},
        {},
        context
    );
    machine.add_commands(deck);

    auto cdtede = deck->get_sys_for_name("cdtede");
    auto cdte1 = deck->get_sys_for_name("cdte1");
    auto cdte2 = deck->get_sys_for_name("cdte2");
    auto cdte3 = deck->get_sys_for_name("cdte3");
    auto cdte4 = deck->get_sys_for_name("cdte4");
    auto cmos1 = deck->get_sys_for_name("cmos1");
    auto cmos2 = deck->get_sys_for_name("cmos2");

    std::queue<UplinkBufferElement> placeholder_queue;
    auto cdtede_manager = std::make_shared<SystemManager>(cdtede, placeholder_queue);
    auto cmos1_manager = std::make_shared<SystemManager>(cmos1, placeholder_queue);
    auto cmos2_manager = std::make_shared<SystemManager>(cmos2, placeholder_queue);

    std::cout << "broadcasting CdTe observation end...\n";
    machine.sync_tcp_send_command_for_sys(*cdtede_manager, deck->get_command_for_sys_for_code(cdtede.hex, 0x12));
    std::this_thread::sleep_for(delay);

    // std::cout << "broadcasting CdTe canisters bias voltage 60 V...\n";
    // machine.sync_tcp_send_command_for_sys(*cdtede_manager, deck->get_command_for_sys_for_code(cdtede.hex, 0x14));
    // std::cout << "\twaiting 30 seconds...\n";
    // std::this_thread::sleep_for(std::chrono::seconds(30));

    // std::cout << "broadcasting CdTe canisters bias voltage 0 V...\n";
    // machine.sync_tcp_send_command_for_sys(*cdtede_manager, deck->get_command_for_sys_for_code(cdtede.hex, 0x13));
    // std::this_thread::sleep_for(delay);

    std::cout << "sending CMOS stop to both detectors...\n";
    machine.sync_tcp_send_command_for_sys(*cmos1_manager, deck->get_command_for_sys_for_code(cmos1.hex, 0x13));
    std::this_thread::sleep_for(delay);
    machine.sync_tcp_send_command_for_sys(*cmos2_manager, deck->get_command_for_sys_for_code(cmos2.hex, 0x13));
    std::this_thread::sleep_for(delay);

    std::cout << "enabling CMOS double commands...\n";
    machine.sync_tcp_send_command_for_sys(*cmos1_manager, deck->get_command_for_sys_for_code(cmos1.hex, 0x0f));
    std::this_thread::sleep_for(delay);
    machine.sync_tcp_send_command_for_sys(*cmos2_manager, deck->get_command_for_sys_for_code(cmos2.hex, 0x0f));
    std::this_thread::sleep_for(delay);

    std::cout << "shutting down CMOS...\n";
    machine.sync_tcp_send_command_for_sys(*cmos1_manager, deck->get_command_for_sys_for_code(cmos1.hex, 0x0c));
    std::this_thread::sleep_for(delay);
    machine.sync_tcp_send_command_for_sys(*cmos2_manager, deck->get_command_for_sys_for_code(cmos2.hex, 0x0c));
    std::this_thread::sleep_for(delay);
    
    std::cout << "closing sockets...\n";
    machine.local_tcp_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    machine.local_udp_sock.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    machine.local_tcp_housekeeping_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    
    machine.local_tcp_sock.close();
    machine.local_udp_sock.close();
    machine.local_tcp_housekeeping_sock.close();
    
    std::cout << "systems are shut down.\n";
    return 0;
}