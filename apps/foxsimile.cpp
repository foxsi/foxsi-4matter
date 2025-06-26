/**
 * @file foxsimile.cpp
 * @author Thanasi Pantazides
 * @brief Emulator for onboard housekeeping data and CdTe data.
 * @version v1.2.1
 * @date 2024-10-18
 * 
 */

#include "Foxsimile.h"
#include "LineInterface.h"
#include "Utilities.h"

#include <boost/asio.hpp>

#include <iostream>

int main(int argc, char** argv) {
    boost::asio::io_context context;
    LineInterface lif(argc, argv, context);
    auto deck_basic = lif.get_command_deck();
    auto deck = std::make_shared<CommandDeck>(deck_basic);

    System& gse = deck->get_sys_for_name("gse");
    System& cdte1 = deck->get_sys_for_name("cdte1");
    System& cdte5 = deck->get_sys_for_name("cdte5");
    System& cdte3 = deck->get_sys_for_name("cdte3");
    System& cdte4 = deck->get_sys_for_name("cdte4");
    System& cmos1 = deck->get_sys_for_name("cmos1");
    System& cmos2 = deck->get_sys_for_name("cmos2");
    System& housekeeping = deck->get_sys_for_name("housekeeping");

    std::map<std::vector<uint8_t>, std::vector<uint8_t>> housekeeping_response_lookup;
    std::map<std::vector<uint8_t>, std::vector<uint8_t>> gse_response_lookup;
    
    std::queue<UplinkBufferElement> cdte1_uplink_queue;
    std::queue<UplinkBufferElement> cdte5_uplink_queue;
    std::queue<UplinkBufferElement> cdte3_uplink_queue;
    std::queue<UplinkBufferElement> cdte4_uplink_queue;
    std::queue<UplinkBufferElement> cmos1_uplink_queue;
    std::queue<UplinkBufferElement> cmos2_uplink_queue;
    std::queue<UplinkBufferElement> housekeeping_uplink_queue;
    std::queue<UplinkBufferElement> gse_uplink_queue;
    auto cdte1_manager = std::make_shared<SystemManager>(cdte1, cdte1_uplink_queue);
    auto cdte5_manager = std::make_shared<SystemManager>(cdte5, cdte5_uplink_queue);
    auto cdte3_manager = std::make_shared<SystemManager>(cdte3, cdte3_uplink_queue);
    auto cdte4_manager = std::make_shared<SystemManager>(cdte4, cdte4_uplink_queue);
    auto cmos1_manager = std::make_shared<SystemManager>(cmos1, cmos1_uplink_queue);
    auto cmos2_manager = std::make_shared<SystemManager>(cmos2, cmos2_uplink_queue);
    auto housekeeping_manager = std::make_shared<SystemManager>(housekeeping, housekeeping_uplink_queue);
    auto gse_manager = std::make_shared<SystemManager>(gse, gse_uplink_queue);

    std::cout << "cdte1 address: " << cdte1_manager->system.ethernet->address << ":" << cdte1_manager->system.ethernet->port << "\n";
    std::cout << "cmos1 address: " << cmos1_manager->system.ethernet->address << ":" << cmos1_manager->system.ethernet->port << "\n";
    std::cout << "housekeeping address: " << housekeeping_manager->system.ethernet->address << ":" << housekeeping_manager->system.ethernet->port << "\n";
    std::cout << "gse address: " << gse_manager->system.ethernet->address << ":" << gse_manager->system.ethernet->port << "\n";

    std::string cdte_mmap_file = "util/mock/de_mmap_mod";
    std::string cmos_mmap_file = "util/mock/cmos_mmap_mod";

    std::vector<uint8_t> housekeeping_request1 = {0x01, 0xf2};
    std::vector<uint8_t> housekeeping_request2 = {0x02, 0xf2};
    std::vector<uint8_t> housekeeping_reply = {
        0x01,0x00,0x65,0x04,0xb8,0x42,0x01,0x70,0x44,0x00,0x01,0x30,0x64,0x00,
        0x01,0x10,0x64,0x00,0x81,0x30,0x64,0x00,0x05,0xf0,0x64,0x00,0x01,0xf0,
        0x64,0x00,0x01,0x70,0x64,0x00,0x81,0xf0,0x64,0x00,0x01,0x00,0x5f,0x13
    };
    housekeeping_response_lookup[housekeeping_request1] = housekeeping_reply;
    housekeeping_response_lookup[housekeeping_request2] = housekeeping_reply;

    auto bulk_lookup = {
        cdte1_manager, 
        cdte5_manager, 
        cdte3_manager, 
        cdte4_manager, 
        cmos1_manager,
        cmos2_manager
    };
    std::map<uint8_t, std::string> bulk_mmap = {
        {cdte1.hex, cdte_mmap_file},
        {cdte5.hex, cdte_mmap_file},
        {cdte3.hex, cdte_mmap_file},
        {cdte4.hex, cdte_mmap_file},
        {cmos1.hex, cmos_mmap_file},
        {cmos2.hex, cmos_mmap_file}
    };

    try {
        std::cout << "starting responder\n";
        foxsimile::Responder mock(
            bulk_mmap, 
            bulk_lookup,
            deck, 
            context
        );
        std::cout << "starting housekeeping\n";
        foxsimile::Responder housekeeping_mock(
            housekeeping_response_lookup, 
            {housekeeping_manager},
            deck, 
            context
        );
        
        std::cout << "listening...\n";
        
        context.run();

    } catch (const char* err) {
        std::cout << err << "\n";
    } catch (std::exception& err) {
        std::cout << err.what() << "\n";
    }

    return 0;
}