#include "Foxsimile.h"
#include "LineInterface.h"
#include "Utilities.h"

#include <boost/asio.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
    boost::asio::io_context context;
    LineInterface lif(argc, argv, context);
    auto deck = std::make_shared<CommandDeck>(lif.get_command_deck());

    System& gse = deck->get_sys_for_name("gse");
    System& cdte1 = deck->get_sys_for_name("cdte1");
    System& cdte2 = deck->get_sys_for_name("cdte2");
    System& cdte3 = deck->get_sys_for_name("cdte3");
    System& cdte4 = deck->get_sys_for_name("cdte4");
    System& cmos1 = deck->get_sys_for_name("cmos1");
    System& cmos2 = deck->get_sys_for_name("cmos2");
    System& housekeeping = deck->get_sys_for_name("housekeeping");

    std::map<std::vector<uint8_t>, std::vector<uint8_t>> cdte1_response_lookup;
    std::map<std::vector<uint8_t>, std::vector<uint8_t>> housekeeping_response_lookup;
    std::map<std::vector<uint8_t>, std::vector<uint8_t>> gse_response_lookup;
    
    std::queue<UplinkBufferElement> cdte1_uplink_queue;
    std::queue<UplinkBufferElement> housekeeping_uplink_queue;
    std::queue<UplinkBufferElement> gse_uplink_queue;
    auto cdte1_manager = std::make_shared<SystemManager>(cdte1, cdte1_uplink_queue);
    auto housekeeping_manager = std::make_shared<SystemManager>(housekeeping, housekeeping_uplink_queue);
    auto gse_manager = std::make_shared<SystemManager>(gse, gse_uplink_queue);

    std::cout << "cdte1 address: " << cdte1_manager->system.ethernet->address << ":" << cdte1_manager->system.ethernet->port << "\n";
    std::cout << "housekeeping address: " << housekeeping_manager->system.ethernet->address << ":" << housekeeping_manager->system.ethernet->port << "\n";
    std::cout << "gse address: " << gse_manager->system.ethernet->address << ":" << gse_manager->system.ethernet->port << "\n";

    std::string cdte1_mmap_file = "util/mock/de_mmap_mod";

    std::vector<uint8_t> housekeeping_request1 = {0x01, 0xf2};
    std::vector<uint8_t> housekeeping_request2 = {0x02, 0xf2};
    std::vector<uint8_t> housekeeping_reply = {
        0x01,0x00,0x65,0x04,0xb8,0x42,0x01,0x70,0x44,0x00,0x01,0x30,0x64,0x00,
        0x01,0x10,0x64,0x00,0x81,0x30,0x64,0x00,0x05,0xf0,0x64,0x00,0x01,0xf0,
        0x64,0x00,0x01,0x70,0x64,0x00,0x81,0xf0,0x64,0x00,0x01,0x00,0x5f,0x13
    };
    housekeeping_response_lookup[housekeeping_request1] = housekeeping_reply;
    housekeeping_response_lookup[housekeeping_request2] = housekeeping_reply;

    try {
        std::cout << "starting cdte1\n";
        foxsimile::Responder cdte1_mock(
            false, 
            cdte1_mmap_file, 
            cdte1_manager,
            deck, 
            context
        );
        std::cout << "starting housekeeping\n";
        foxsimile::Responder housekeeping_mock(
            false, 
            housekeeping_response_lookup, 
            housekeeping_manager,
            deck, 
            context
        );
        std::cout << "starting gse\n";
        foxsimile::Responder gse_mock(
            false, 
            gse_response_lookup, 
            gse_manager,
            deck, 
            context
        );

        std::cout << "listening...\n";
        // cdte1_mock.async_receive();
        // housekeeping_mock.async_receive();

        context.run();

    } catch (const char* err) {
        std::cout << err << "\n";
    } catch (std::exception& err) {
        std::cout << err.what() << "\n";
    }

    return 0;
}