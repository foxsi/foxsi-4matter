#include "Circle.h"
#include "Buffers.h"
#include "Parameters.h"
#include "Systems.h"
#include "Utilities.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <cmath>
#include <ctime>
#include <thread>
#include <chrono>
#include <unordered_map>

Circle::Circle(double new_period_s, std::vector<std::shared_ptr<SystemManager>> new_system_order, std::shared_ptr<CommandDeck> new_deck, std::shared_ptr<TransportLayerMachine> new_transport, boost::asio::io_context &new_context): 
    deck(new_deck),
    transport(new_transport),
    system_order(new_system_order) {
    
    if (new_period_s <= 0) {
        utilities::error_log("Circle::Circle()\tinvalid period.");
    }
    period_s = new_period_s;

    current_system = 0;
    current_state = STATE_ORDER::CMD_SEND;
    
    normalize_times_to_period();
    init_systems();

    // disable for testing
    // transport->await_loop_begin();

    slowmo_gain = 1;

    Circle::get_sys_man_for_name("cdte1")->last_write_pointer[RING_BUFFER_TYPE_OPTIONS::PC] = 0x00;
    Circle::get_sys_man_for_name("cdte5")->last_write_pointer[RING_BUFFER_TYPE_OPTIONS::PC] = 0x00;
    Circle::get_sys_man_for_name("cdte3")->last_write_pointer[RING_BUFFER_TYPE_OPTIONS::PC] = 0x00;
    Circle::get_sys_man_for_name("cdte4")->last_write_pointer[RING_BUFFER_TYPE_OPTIONS::PC] = 0x00;
    Circle::get_sys_man_for_name("cmos1")->last_write_pointer[RING_BUFFER_TYPE_OPTIONS::PC] = 0x00;
    Circle::get_sys_man_for_name("cmos1")->last_write_pointer[RING_BUFFER_TYPE_OPTIONS::QL] = 0x00;
    Circle::get_sys_man_for_name("cmos2")->last_write_pointer[RING_BUFFER_TYPE_OPTIONS::PC] = 0x00;
    Circle::get_sys_man_for_name("cmos2")->last_write_pointer[RING_BUFFER_TYPE_OPTIONS::QL] = 0x00;

    Circle::get_sys_man_for_name("cdte1")->active_type = RING_BUFFER_TYPE_OPTIONS::PC;
    Circle::get_sys_man_for_name("cdte5")->active_type = RING_BUFFER_TYPE_OPTIONS::PC;
    Circle::get_sys_man_for_name("cdte3")->active_type = RING_BUFFER_TYPE_OPTIONS::PC;
    Circle::get_sys_man_for_name("cdte4")->active_type = RING_BUFFER_TYPE_OPTIONS::PC;
    Circle::get_sys_man_for_name("cmos1")->active_type = RING_BUFFER_TYPE_OPTIONS::PC;
    Circle::get_sys_man_for_name("cmos2")->active_type = RING_BUFFER_TYPE_OPTIONS::PC;

    // run the main loop
    run = true;
}

void Circle::start() {
    while (run) {
        update_state();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Circle::update_state() {
    // take care of the current system:
    manage_systems();
    // set the next system:
    current_system = (current_system + 1) % system_order.size();
}

void Circle::init_systems() {

    init_housekeeping();

    init_timepix();

    init_cdte();

    init_cmos();
}

void Circle::init_housekeeping() {
    utilities::debug_print("\ninitializing housekeeping system\n");
    SystemManager* housekeeping = Circle::get_sys_man_for_name("housekeeping");
    // setup both temperature sensors
    transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x06));
    transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x07));
    // sleep to give LTC2983s time to set up:
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // start conversion
    transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x04));
    transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x05));

    // setup power ADC
    transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x28));

    housekeeping->system_state = SYSTEM_STATE::LOOP;
}

void Circle::init_cdte() {
    utilities::debug_print("\ninitializing cdte system\n");
    SystemManager* cdtede = Circle::get_sys_man_for_name("cdtede");
    System& cdte1 = deck->get_sys_for_name("cdte1");
    System& cdte5 = deck->get_sys_for_name("cdte5");
    System& cdte3 = deck->get_sys_for_name("cdte3");
    System& cdte4 = deck->get_sys_for_name("cdte4");

    auto delay = std::chrono::milliseconds(5000);
    auto delay_init = std::chrono::seconds(10);
    auto delay_60to200v = std::chrono::seconds(30);
    auto delay_post200v = std::chrono::seconds(300);

// debug cmos
    // Circle::get_sys_man_for_name("cdte1")->system_state = SYSTEM_STATE::ABANDON;
    // Circle::get_sys_man_for_name("cdte5")->system_state = SYSTEM_STATE::ABANDON;
    // Circle::get_sys_man_for_name("cdte3")->system_state = SYSTEM_STATE::ABANDON;
    // Circle::get_sys_man_for_name("cdte4")->system_state = SYSTEM_STATE::ABANDON;
    // Circle::get_sys_man_for_name("cdtede")->system_state = SYSTEM_STATE::ABANDON;
    // return;

    // todo: add check for ethernet port open correctly.

    // Check canister ping status       0x08 0x8a
    utilities::debug_print("checking canister status...\n");
    std::vector<uint8_t> can_status = transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x8a));
    can_status = transport->get_reply_data(can_status, cdtede->system);
    cdtede->system_state = SYSTEM_STATE::LOOP;
    utilities::debug_print("canisters status: ");
    utilities::hex_print(can_status);

    std::vector<System> cdte_sys = {cdte1, cdte5, cdte3, cdte4};
    for (uint8_t i = 0; i < 4; ++i) {
        std::string this_name = cdte_sys[i].name;
        if (can_status.size() < 4) {
            // utilities::error_print("got too-short canister status reply! Abandoning CdTe.\n");
            utilities::error_log("Circle::init_cdte()\tcanisters status disconnected.");
            cdtede->errors |= errors::system::reading_invalid;
            
            // Circle::get_sys_man_for_name("cdte1")->system_state = SYSTEM_STATE::ABANDON;
            // Circle::get_sys_man_for_name("cdte5")->system_state = SYSTEM_STATE::ABANDON;
            // Circle::get_sys_man_for_name("cdte3")->system_state = SYSTEM_STATE::ABANDON;
            // Circle::get_sys_man_for_name("cdte4")->system_state = SYSTEM_STATE::ABANDON;
        } else {
            if(can_status.at(i) != 0x00) {
                Circle::get_sys_man_for_name(this_name)->system_state = SYSTEM_STATE::LOOP;
            } else {
                // Circle::get_sys_man_for_name(this_name)->system_state = SYSTEM_STATE::ABANDON;
            }
        }
    }

    // std::this_thread::sleep_for(delay);

    // DE init                          0x08 0x09
    // transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x09));
    // std::this_thread::sleep_for(delay_init);

    // utilities::debug_print("trying new write...\n");
    // DE standby                       0x08 0x0a
    // transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x0a));
    // std::this_thread::sleep_for(delay);

    // DE observe                       0x08 0x0b
    // transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x0b));
    // std::this_thread::sleep_for(delay);

    // Canister 1 start                 0x09 0x11
    // transport->sync_send_command_to_system(*cdte1, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x11));
    // std::this_thread::sleep_for(delay);

    // Apply HV 0V for all canister     0x08 0x13
    // transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x13));
    // std::this_thread::sleep_for(delay);

    // Apply HV 60V for all canister    0x08 0x14
    // transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x14));
    // utilities::debug_print("waiting 30 seconds...\n");
    // std::this_thread::sleep_for(delay_60to200v);

    // Apply HV 200V for all canister    0x08 0x16
    // transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x16));
    // utilities::debug_print("waiting 5 minutes...\n");
    // std::this_thread::sleep_for(delay_post200v);

    // Set full readout for all canister    0x08 0x19
    // transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x19));
    // std::this_thread::sleep_for(delay);

    // Set sparse readout for all canister    0x08 0x18
    // transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x18));
    // std::this_thread::sleep_for(delay);

    // Start observe for all canister   0x08 0x11
    // transport->sync_send_command_to_system(*cdtede, deck->get_command_for_sys_for_code(cdtede->system.hex, 0x11));
    // std::this_thread::sleep_for(delay);

    // later, will need to end observe and lower bias for all.
}
void Circle::init_cmos() {
    utilities::debug_print("\ninitializing cmos system\n");

// debug cdte:
    // utilities::debug_print("removing all cmos, isolating cdte\n");
    // Circle::get_sys_man_for_name("cmos1")->system_state = SYSTEM_STATE::ABANDON;
    // Circle::get_sys_man_for_name("cmos2")->system_state = SYSTEM_STATE::ABANDON;
    // return;

    SystemManager* cmos1 = Circle::get_sys_man_for_name("cmos1");
    SystemManager* cmos2 = Circle::get_sys_man_for_name("cmos2");

    cmos1->system_state = SYSTEM_STATE::LOOP;
    cmos2->system_state = SYSTEM_STATE::LOOP;

    auto delay = std::chrono::milliseconds(2000);

    /*----------------------- for cmos1 -----------------------*/

    // todo: add check for ethernet port open correctly.

    // Check cmos linetime       0x0f 0xa0
    utilities::debug_print("checking cmos1 status...\n");
    std::vector<uint8_t> cmos1_status = transport->sync_send_command_to_system(*cmos1, deck->get_command_for_sys_for_code(cmos1->system.hex, 0xa0));
    if (cmos1_status.size() < 4) {
        // utilities::error_print("could not receive from cmos1: ABANDONing\n");
        utilities::error_log("Circle::init_cmos()\treceived no response to cmos1 linetime request.");
        cmos1->errors |= errors::system::reading_packet;
        // cmos1->system_state = SYSTEM_STATE::ABANDON;

    } else {
        cmos1_status = transport->get_reply_data(cmos1_status, cmos1->system);
        if (cmos1_status.size() < 4) {
            utilities::error_log("Circle::init_cmos()\treceived invalid response to cmos1 linetime request:");
            utilities::error_log("Circle::init_cmos()\t" + utilities::bytes_to_string(cmos1_status));
            cmos1->errors |= errors::system::reading_invalid;
        }
        utilities::debug_print("cmos1 linetime: ");
        utilities::hex_print(cmos1_status);
    }

    // send start_cmos_init         0x0f 0x18
    // transport->sync_send_command_to_system(*cmos1, deck->get_command_for_sys_for_code(cmos1->system.hex, 0x18));
    // std::this_thread::sleep_for(delay);
	
    // send start_cmos_training     0x0f 0x1f
    // transport->sync_send_command_to_system(*cmos1, deck->get_command_for_sys_for_code(cmos1->system.hex, 0x1f));
    // std::this_thread::sleep_for(delay);
	
    // send set_cmos_params         0x0f 0x10
    // transport->sync_send_command_to_system(*cmos1, deck->get_command_for_sys_for_code(cmos1->system.hex, 0x10));
    // std::this_thread::sleep_for(delay);
	
    // send start_cmos_exposure     0x0f 0x12
    // transport->sync_send_command_to_system(*cmos1, deck->get_command_for_sys_for_code(cmos1->system.hex, 0x12));
    // std::this_thread::sleep_for(delay);

    /*----------------------- for cmos2 -----------------------*/

    // Check cmos linetime       0x0f 0xa0
    utilities::debug_print("checking cmos2 status...\n");
    std::vector<uint8_t> cmos2_status = transport->sync_send_command_to_system(*cmos2, deck->get_command_for_sys_for_code(cmos2->system.hex, 0xa0));
    if (cmos2_status.size() < 4) {
        // utilities::error_print("could not receive from cmos2: ABANDONing\n");
        utilities::error_log("Circle::init_cmos()\treceived no response to cmos2 linetime request.");
        // cmos2->system_state = SYSTEM_STATE::ABANDON;
    } else {
        cmos2_status = transport->get_reply_data(cmos2_status, cmos2->system);
        if (cmos2_status.size() < 4) {
            utilities::error_log("Circle::init_cmos()\treceived invalid response to cmos2 linetime request:");
            utilities::error_log("Circle::init_cmos()\t" + utilities::bytes_to_string(cmos2_status));
            cmos2->errors |= errors::system::reading_invalid;
        }
        utilities::debug_print("cmos2 linetime: ");
        utilities::hex_print(cmos2_status);
        std::this_thread::sleep_for(delay);
    }
}

void Circle::init_timepix() {
    utilities::debug_print("\ninitializing timepix system\n");
    SystemManager* timepix = Circle::get_sys_man_for_name("timepix");
    
    // check that serial port opened correctly. 
    // todo: figure out why `.is_open()` segfaults if port is not open.
    // a better implementation (soon) is to store port status in a global state and check it in this `if`.
    if (!transport->local_uart_port.is_open()) {
        // utilities::error_print("timepix uart port failed to open! Will try to talk to it anyway.");
        utilities::error_log("Circle::init_timepix()\ttimepix uart port failed to open. Will abandon timepix.");
        // utilities::error_print("timepix uart port failed to open! ABANDONing.");
        timepix->system_state = SYSTEM_STATE::ABANDON;
        return;
    }

    utilities::debug_print("\tsending ping...\n");
    Command& ping_ask = deck->get_command_for_sys_for_code(timepix->system.hex, 0x80);
    std::vector<uint8_t> response = transport->sync_send_command_to_system(*timepix, ping_ask);

    if (response.size() > 0) {
        utilities::debug_print("got response from timepix: ");
        utilities::hex_print(response);
        utilities::debug_log("Circle::init_timepix()\tgot timepix pingback.");
        timepix->system_state = SYSTEM_STATE::LOOP;
    } else {
        // utilities::error_print("got no response from timepix. Will try to talk to it anyway.");
        utilities::error_log("Circle::init_timepix()\tgot no response to ping.");
        // utilities::error_print("got no response from timepix. ABANDONing!");
        // timepix->system_state = SYSTEM_STATE::ABANDON;
    }
}

void Circle::manage_systems() {
    utilities::debug_log("Circle::manage_systems()\tfor " + system_order.at(current_system)->system.name + "...");

    record_uplink();    // trying to implement uplink without blocking everything
    flush();
    send_global_health();

    auto cdtede = *Circle::get_sys_man_for_name("cdtede");
    auto cdte1 = *Circle::get_sys_man_for_name("cdte1");
    auto cdte5 = *Circle::get_sys_man_for_name("cdte5");
    auto cdte3 = *Circle::get_sys_man_for_name("cdte3");
    auto cdte4 = *Circle::get_sys_man_for_name("cdte4");
    auto cmos1 = *Circle::get_sys_man_for_name("cmos1");
    auto cmos2 = *Circle::get_sys_man_for_name("cmos2");
    
    utilities::debug_print("\n");
    std::chrono::milliseconds delay_inter_cdte_ms(10);
    std::chrono::milliseconds delay_inter_cmos_ms(10);
    
    // immediately skip if we are trying to talk to a system marked "ABANDONED".
    if (system_order.at(current_system)->system_state == SYSTEM_STATE::ABANDON) {
        utilities::error_log("Circle::manage_systems()\tcurrent system " + system_order.at(current_system)->system.name + " was abandoned.");
        // utilities::error_print("current system " + system_order.at(current_system)->system.name + " was abandoned! Continuing.\n");
        return;
    }

    if (system_order.at(current_system)->system == deck->get_sys_for_name("cdte1")) {
        utilities::debug_print("managing cdte1 system\n");

        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cdtede"));
        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cdte1"));
        Circle::get_sys_man_for_name("cdte1")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC) = transport->sync_remote_buffer_transaction(*Circle::get_sys_man_for_name("cdte1"), RING_BUFFER_TYPE_OPTIONS::PC, Circle::get_sys_man_for_name("cdte1")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC));

        std::vector<uint8_t> hk = transport->sync_send_command_to_system(cdte1, deck->get_command_for_sys_for_code(cdte1.system.hex, 0xbf));

        bool did_queue_hk;
        if (hk.size() > 0) {
            std::vector<uint8_t> hk_data = transport->get_reply_data(hk, cdte1.system);
            DownlinkBufferElement dbe(&(cdte1.system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::HK);
            dbe.set_payload(hk_data);
            // queue and send the downlink buffer:
            did_queue_hk = transport->downlink_buffer->enqueue(dbe);
        }

        bool has_data = transport->sync_udp_send_all_downlink_buffer();

        if (has_data || !did_queue_hk) {
            (*Circle::get_sys_man_for_name("cdte1")).errors |= errors::system::downlink_buffering;
        }

        // delay before reading again 
        std::this_thread::sleep_for(delay_inter_cdte_ms);

    } else if (system_order.at(current_system)->system == deck->get_sys_for_name("cdte5")) {
        utilities::debug_print("managing cdte5 system\n");

        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cdtede"));
        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cdte5"));
        Circle::get_sys_man_for_name("cdte5")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC) = transport->sync_remote_buffer_transaction(*Circle::get_sys_man_for_name("cdte5"), RING_BUFFER_TYPE_OPTIONS::PC, Circle::get_sys_man_for_name("cdte5")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC));
        
        std::vector<uint8_t> hk = transport->sync_send_command_to_system(cdte5, deck->get_command_for_sys_for_code(cdte5.system.hex, 0xbf));
        if (hk.size() > 0) {
            std::vector<uint8_t> hk_data = transport->get_reply_data(hk, cdte5.system);
            DownlinkBufferElement dbe(&(cdte5.system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::HK);
            dbe.set_payload(hk_data);
            // queue and send the downlink buffer:
            transport->downlink_buffer->enqueue(dbe);
        }
        
        bool has_data = transport->sync_udp_send_all_downlink_buffer();

        // delay before reading again to avoid duplicate 
        std::this_thread::sleep_for(delay_inter_cdte_ms);

    } else if (system_order.at(current_system)->system == deck->get_sys_for_name("cdte3")) {
        utilities::debug_print("managing cdte3 system\n");

        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cdtede"));
        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cdte3"));
        Circle::get_sys_man_for_name("cdte3")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC) = transport->sync_remote_buffer_transaction(*Circle::get_sys_man_for_name("cdte3"), RING_BUFFER_TYPE_OPTIONS::PC, Circle::get_sys_man_for_name("cdte3")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC));
        
        std::vector<uint8_t> hk = transport->sync_send_command_to_system(cdte3, deck->get_command_for_sys_for_code(cdte3.system.hex, 0xbf));
        if (hk.size() > 0) {
            std::vector<uint8_t> hk_data = transport->get_reply_data(hk, cdte3.system);
            DownlinkBufferElement dbe(&(cdte3.system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::HK);
            dbe.set_payload(hk_data);
            // queue and send the downlink buffer:
            transport->downlink_buffer->enqueue(dbe);
        }

        bool has_data = transport->sync_udp_send_all_downlink_buffer();

        // delay before reading again to avoid duplicate 
        std::this_thread::sleep_for(delay_inter_cdte_ms);

    } else if (system_order.at(current_system)->system == deck->get_sys_for_name("cdte4")) {
        utilities::debug_print("managing cdte4 system\n");

        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cdtede"));
        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cdte4"));
        Circle::get_sys_man_for_name("cdte4")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC) = transport->sync_remote_buffer_transaction(*Circle::get_sys_man_for_name("cdte4"), RING_BUFFER_TYPE_OPTIONS::PC, Circle::get_sys_man_for_name("cdte4")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC));
        
        std::vector<uint8_t> hk = transport->sync_send_command_to_system(cdte4, deck->get_command_for_sys_for_code(cdte4.system.hex, 0xbf));
        if (hk.size() > 0) {
            std::vector<uint8_t> hk_data = transport->get_reply_data(hk, cdte4.system);
            DownlinkBufferElement dbe(&(cdte4.system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::HK);
            dbe.set_packets_per_frame(1);
            dbe.set_this_packet_index(1);
            dbe.set_payload(hk_data);
            // queue and send the downlink buffer:
            transport->downlink_buffer->enqueue(dbe);
        }

        bool has_data = transport->sync_udp_send_all_downlink_buffer();

        // delay before reading again to avoid duplicate 
        std::this_thread::sleep_for(delay_inter_cdte_ms);

    } else if (system_order.at(current_system)->system == deck->get_sys_for_name("cdtede")) {
        utilities::debug_print("managing cdtede system\n");

        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cdtede"));
        std::vector<uint8_t> hk = transport->sync_send_command_to_system(cdtede, deck->get_command_for_sys_for_code(cdtede.system.hex, 0xaf));
        if (hk.size() > 0) {
            std::vector<uint8_t> hk_data = transport->get_reply_data(hk, cdtede.system);
            DownlinkBufferElement dbe(&(cdtede.system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::HK);
            dbe.set_payload(hk_data);
            // queue and send the downlink buffer:
            transport->downlink_buffer->enqueue(dbe);
        }

    } else if (system_order.at(current_system)->system == deck->get_sys_for_name("cmos1")) {
        utilities::debug_print("managing cmos1\n");
        // debug for CdTe
        // utilities::debug_print("\tskipping");
        // return;

        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cmos1"));
        
        // move this state inside SystemManager
        if (Circle::get_sys_man_for_name("cmos1")->active_type == RING_BUFFER_TYPE_OPTIONS::PC) {
            Circle::get_sys_man_for_name("cmos1")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC) = transport->sync_remote_buffer_transaction(*Circle::get_sys_man_for_name("cmos1"), RING_BUFFER_TYPE_OPTIONS::PC, Circle::get_sys_man_for_name("cmos1")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC));
            
            Circle::get_sys_man_for_name("cmos1")->active_type = RING_BUFFER_TYPE_OPTIONS::QL;
        } else {
            Circle::get_sys_man_for_name("cmos1")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::QL) = transport->sync_remote_buffer_transaction(*Circle::get_sys_man_for_name("cmos1"), RING_BUFFER_TYPE_OPTIONS::QL, Circle::get_sys_man_for_name("cmos1")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::QL));
            
            Circle::get_sys_man_for_name("cmos1")->active_type = RING_BUFFER_TYPE_OPTIONS::PC;
        }

        std::vector<uint8_t> hk = transport->sync_send_command_to_system(cmos1, deck->get_command_for_sys_for_code(cmos1.system.hex, 0x88));
        if (hk.size() > 0) {
            std::vector<uint8_t> hk_data = transport->get_reply_data(hk, cmos1.system);
            DownlinkBufferElement dbe(&(cmos1.system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::HK);
            dbe.set_payload(hk_data);
            // queue and send the downlink buffer:
            transport->downlink_buffer->enqueue(dbe);
        }

        bool has_data = transport->sync_udp_send_all_downlink_buffer();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

    } else if (system_order.at(current_system)->system == deck->get_sys_for_name("cmos2")) {
        utilities::debug_print("managing cmos2\n");
        // debug for CdTe
        // utilities::debug_print("\tskipping");
        // return;

        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("cmos2"));
        
        // move this state inside SystemManager
        if (Circle::get_sys_man_for_name("cmos2")->active_type == RING_BUFFER_TYPE_OPTIONS::PC) {
            Circle::get_sys_man_for_name("cmos2")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC) = transport->sync_remote_buffer_transaction(*Circle::get_sys_man_for_name("cmos2"), RING_BUFFER_TYPE_OPTIONS::PC, Circle::get_sys_man_for_name("cmos2")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::PC));
            
            Circle::get_sys_man_for_name("cmos2")->active_type = RING_BUFFER_TYPE_OPTIONS::QL;
        } else {
            Circle::get_sys_man_for_name("cmos2")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::QL) = transport->sync_remote_buffer_transaction(*Circle::get_sys_man_for_name("cmos2"), RING_BUFFER_TYPE_OPTIONS::QL, Circle::get_sys_man_for_name("cmos2")->last_write_pointer.at(RING_BUFFER_TYPE_OPTIONS::QL));
            
            Circle::get_sys_man_for_name("cmos2")->active_type = RING_BUFFER_TYPE_OPTIONS::PC;
        }

        std::vector<uint8_t> hk = transport->sync_send_command_to_system(cmos2, deck->get_command_for_sys_for_code(cmos2.system.hex, 0x88));
        if (hk.size() > 0) {
            std::vector<uint8_t> hk_data = transport->get_reply_data(hk, cmos2.system);
            DownlinkBufferElement dbe(&(cmos2.system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::HK);
            dbe.set_payload(hk_data);
            // queue and send the downlink buffer:
            transport->downlink_buffer->enqueue(dbe);
        }

        bool has_data = transport->sync_udp_send_all_downlink_buffer();
        std::this_thread::sleep_for(delay_inter_cmos_ms);

    } else if (system_order.at(current_system)->system == deck->get_sys_for_name("timepix")) {
        utilities::debug_print("managing timepix system\n");

        // todo: write this to implement command forwarding for Timepix:
        // transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("timepix"));
        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("timepix"));

        SystemManager* timepix = Circle::get_sys_man_for_name("timepix");
        Command flags_req_cmd  = deck->get_command_for_sys_for_code(timepix->system.hex, 0x8a);
        Command hk_req_cmd     = deck->get_command_for_sys_for_code(timepix->system.hex, 0x88);
        Command rates_req_cmd  = deck->get_command_for_sys_for_code(timepix->system.hex, 0x81);
        Command pcap_req_cmd   = deck->get_command_for_sys_for_code(timepix->system.hex, 0x25);
        Command pc_req_cmd    = deck->get_command_for_sys_for_code(timepix->system.hex, 0x20);

        utilities::debug_print("\tsending " + utilities::bytes_to_string(flags_req_cmd.get_uart_instruction()) + "\n");
        utilities::debug_print("\tsending " + utilities::bytes_to_string(hk_req_cmd.get_uart_instruction()) + "\n");
        utilities::debug_print("\tsending " + utilities::bytes_to_string(rates_req_cmd.get_uart_instruction()) + "\n");
        utilities::debug_print("\tsending " + utilities::bytes_to_string(pcap_req_cmd.get_uart_instruction()) + "\n");
        utilities::debug_print("\tsending " + utilities::bytes_to_string(pc_req_cmd.get_uart_instruction()) + "\n");
        
        std::vector<uint8_t> flags_response(flags_req_cmd.get_uart_reply_length());
        std::vector<uint8_t> hk_response(hk_req_cmd.get_uart_reply_length());
        std::vector<uint8_t> rates_response(rates_req_cmd.get_uart_reply_length());
        std::vector<uint8_t> pcap_response(pcap_req_cmd.get_uart_reply_length());
        std::vector<uint8_t> pc_response(pc_req_cmd.get_uart_reply_length());
        
        std::vector<uint8_t> request_time = utilities::splat_to_nbytes(4, static_cast<uint32_t>(std::time(nullptr)));

        flags_response = transport->sync_send_command_to_system(*timepix, flags_req_cmd);
        hk_response = transport->sync_send_command_to_system(*timepix, hk_req_cmd);
        rates_response = transport->sync_send_command_to_system(*timepix, rates_req_cmd);
        pcap_response = transport->sync_send_command_to_system(*timepix, pcap_req_cmd);
        pc_response = transport->sync_send_command_to_system(*timepix, pc_req_cmd);

        utilities::debug_print("\tsent all\n");
        
        // if got no response, resize these so they are full of zero and correct length:
        if (flags_response.size() + hk_response.size() + rates_response.size() == 0) {
            utilities::error_log("Circle::manage_systems\ttimepix\tgot no replies.");
            return;
        }

        flags_response.resize(flags_req_cmd.get_uart_reply_length());
        hk_response.resize(hk_req_cmd.get_uart_reply_length());
        rates_response.resize(rates_req_cmd.get_uart_reply_length());

        // utilities::debug_print("\tresized all\n");

        // build the downlink packet:
        std::vector<uint8_t> downlink_health;
        downlink_health.push_back(timepix->system.hex);
        downlink_health.push_back(0x00);
        downlink_health.insert(downlink_health.end(), request_time.begin(), request_time.end());
        downlink_health.insert(downlink_health.end(), flags_response.begin(), flags_response.end());
        downlink_health.insert(downlink_health.end(), hk_response.begin(), hk_response.end());
        downlink_health.insert(downlink_health.end(), rates_response.begin(), rates_response.end());
        
        utilities::debug_print("\tresized all\n");

        // create element for downlink buffer:
        DownlinkBufferElement dbe_health(&(deck->get_sys_for_name("timepix")), &(deck->get_sys_for_name("gse")), 
        RING_BUFFER_TYPE_OPTIONS::TPX);
        dbe_health.set_packets_per_frame(1);
        dbe_health.set_this_packet_index(1);
        dbe_health.set_payload(downlink_health);

        transport->downlink_buffer->enqueue(dbe_health);

        // check if we got pcap response, downlink if so.
        if (pcap_response.size() == pcap_req_cmd.get_uart_reply_length()) {
            // populate packet:
            std::vector<uint8_t> downlink_pcap;
            downlink_pcap.push_back(timepix->system.hex);
            downlink_pcap.push_back(0x00);
            downlink_pcap.insert(downlink_pcap.end(), pcap_response.begin(), pcap_response.end());
            // put it in a DownlinkBufferElement
            DownlinkBufferElement dbe_pcap(&(deck->get_sys_for_name("timepix")), &(deck->get_sys_for_name("gse")), 
                RING_BUFFER_TYPE_OPTIONS::PCAP);
            dbe_pcap.set_packets_per_frame(1);
            dbe_pcap.set_this_packet_index(1);
            dbe_pcap.set_payload(downlink_pcap);
            // queue the buffer element
            transport->downlink_buffer->enqueue(dbe_pcap);
        }
        if (pc_response.size() == pc_req_cmd.get_uart_reply_length()) {
            // populate packet:
            std::vector<uint8_t> downlink_pc;
            downlink_pc.push_back(timepix->system.hex);
            downlink_pc.push_back(0x00);
            downlink_pc.insert(downlink_pc.end(), pc_response.begin(), pc_response.end());
            // put it in a DownlinkBufferElement
            DownlinkBufferElement dbe_pc(&(deck->get_sys_for_name("timepix")), &(deck->get_sys_for_name("gse")), 
                RING_BUFFER_TYPE_OPTIONS::PC);
            dbe_pc.set_packets_per_frame(1);
            dbe_pc.set_this_packet_index(1);
            dbe_pc.set_payload(downlink_pc);
            // qpce the buffer element
            transport->downlink_buffer->enqueue(dbe_pc);
        }

        DownlinkBufferElement dbe_img(&(deck->get_sys_for_name("timepix")), &(deck->get_sys_for_name("gse")), 
        RING_BUFFER_TYPE_OPTIONS::PC);
        dbe_health.set_packets_per_frame(1);
        dbe_health.set_this_packet_index(1);
        dbe_health.set_payload(downlink_health);

        utilities::debug_print("\tbuffered downlink\n");

        // queue and send the downlink buffer:
        bool has_data = transport->sync_udp_send_all_downlink_buffer();

    } else if (system_order.at(current_system)->system == deck->get_sys_for_name("housekeeping")) {
        utilities::debug_print("managing housekeeping system\n");
        SystemManager* housekeeping = Circle::get_sys_man_for_name("housekeeping");
        // read out both sensors (0x01 0xf2, then 0x02 0xf2)
        // then start a new conversion (0x01 0xf0, then 0x02 0xf0)

        // return Formatter-only HK data here.

        if (housekeeping->system_state == SYSTEM_STATE::DISCONNECT) {
            utilities::error_log("Circle::manage_systems()\thousekeeping\thas been DISCONNECTed.");
            return;
        }

        transport->sync_send_buffer_commands_to_system(*Circle::get_sys_man_for_name("housekeeping"));

        size_t zero_finder = 1;
        if (housekeeping->counter % config::timing::HOUSEKEEPING_LOOP_EVERY == 0) {
            utilities::debug_log("Circle::manage_systems()\thousekeeping\ttrying read.");
            // unix timestamp
            std::vector<uint8_t> reply_time = utilities::splat_to_nbytes(4, static_cast<uint32_t>(std::time(nullptr)));
            if (housekeeping->enable & 0x01) {
                utilities::debug_log("Circle::manage_systems()\thousekeeping\treading power.");
                // do read power ADC
                std::vector<uint8_t> adc_reply = transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0xa0));
                
                zero_finder *= adc_reply.size();

                utilities::debug_print("adc:\t" + utilities::bytes_to_string(adc_reply) + "\n");
                
                if (adc_reply.size() > 0) {
                    std::vector<uint8_t> packet_power = {0x04, 0x00};
                    packet_power.insert(packet_power.end(), reply_time.begin(), reply_time.end());
                    packet_power.insert(packet_power.end(), adc_reply.begin(), adc_reply.end());
                    DownlinkBufferElement dbe_power(&(housekeeping->system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::POW);
                    dbe_power.set_payload(packet_power);
                    transport->downlink_buffer->enqueue(dbe_power);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
            if (housekeeping->enable & 0x02) {
                utilities::debug_log("Circle::manage_systems()\thousekeeping\treading RTDs.");
                // do read RTD data
                std::vector<uint8_t> temp1_reply = transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x84));
                std::vector<uint8_t> temp2_reply = transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x85));
                
                zero_finder *= temp1_reply.size();
                zero_finder *= temp2_reply.size();

                utilities::debug_print("temp1:\t" + utilities::bytes_to_string(temp1_reply) + "\n");
                utilities::debug_print("temp2:\t" + utilities::bytes_to_string(temp2_reply) + "\n");

                if (temp1_reply.size() > 0) {
                    std::vector<uint8_t> packet_temp1 = {0x01, 0x00};
                    packet_temp1.insert(packet_temp1.end(), reply_time.begin(), reply_time.end());
                    packet_temp1.insert(packet_temp1.end(), temp1_reply.begin(), temp1_reply.end());
                    DownlinkBufferElement dbe_temp1(&(housekeeping->system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::RTD);
                    dbe_temp1.set_payload(packet_temp1);
                    transport->downlink_buffer->enqueue(dbe_temp1);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    // start a new conversion
                    transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x04));
                }
                
                if (temp2_reply.size() > 0) {
                    std::vector<uint8_t> packet_temp2 = {0x02, 0x00};
                    packet_temp2.insert(packet_temp2.end(), reply_time.begin(), reply_time.end());
                    packet_temp2.insert(packet_temp2.end(), temp2_reply.begin(), temp2_reply.end());
                    DownlinkBufferElement dbe_temp2(&(housekeeping->system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::RTD);
                    dbe_temp2.set_payload(packet_temp2);
                    transport->downlink_buffer->enqueue(dbe_temp2);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    // start a new conversion
                    transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x05));
                }
            }
            if (housekeeping->enable & 0x04) {
                utilities::debug_log("Circle::manage_systems()\thousekeeping\treading intro.");
                // do read introspection
                // // microcontroller clock counter reading:
                std::vector<uint8_t> clock_reply = transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x8e));
                // flight state reading:
                std::vector<uint8_t> state_reply = transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0xf0));
                // microcontroller error state reading:
                std::vector<uint8_t> error_reply = transport->sync_send_command_to_system(*housekeeping, deck->get_command_for_sys_for_code(housekeeping->system.hex, 0x8f));

                zero_finder *= clock_reply.size();
                zero_finder *= state_reply.size();
                zero_finder *= error_reply.size();

                utilities::debug_print("stt:\t" + utilities::bytes_to_string(state_reply) + "\n");
                utilities::debug_print("clk:\t" + utilities::bytes_to_string(clock_reply) + "\n");
                utilities::debug_print("err:\t" + utilities::bytes_to_string(error_reply) + "\n");

                if (clock_reply.size() * state_reply.size() * error_reply.size() != 0) {
                    std::vector<uint8_t> packet_intro = {0x07, 0x00};
                    packet_intro.insert(packet_intro.end(), reply_time.begin(), reply_time.end());      // 4 B
                    packet_intro.insert(packet_intro.end(), clock_reply.begin(), clock_reply.end());    // 2 B
                    packet_intro.insert(packet_intro.end(), state_reply.begin(), state_reply.end());    // 2 B
                    packet_intro.insert(packet_intro.end(), error_reply.begin(), error_reply.end());    // 2 B
                    DownlinkBufferElement dbe_intro(&(housekeeping->system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::INTRO);
                    dbe_intro.set_payload(packet_intro);
                    transport->downlink_buffer->enqueue(dbe_intro);
                }
            }
        }

        if (zero_finder == 0) {
            utilities::error_print("Housekeeping failed to respond!\n");
            utilities::error_print("Using UDP now, so we don't care! We'll be back again next loop.\n");
            return;
        }

        housekeeping->counter += 1;

        bool has_data = transport->sync_udp_send_all_downlink_buffer();

    } else {
        utilities::debug_print("system management fell through in Circle for " + system_order.at(current_system)->system.name +  "\n");
        utilities::debug_log("Circle::manage_systems()\t fell through for " + system_order.at(current_system)->system.name);
    }
}

void Circle::normalize_times_to_period() {
    
    double total_system_allocation_millis;
    for (auto& this_system: system_order) {
        total_system_allocation_millis += this_system->timing->period_millis;
    }
    
    for (auto& this_system: system_order) {
        this_system->timing->period_millis = (uint32_t)(std::round(this_system->timing->period_millis*period_s*1000.0/total_system_allocation_millis));

        this_system->timing->resolve_times();
    }
}

void Circle::record_uplink() {
    // transport->sync_udp_receive_to_uplink_buffer(*get_sys_man_for_name("uplink"));
    transport->sync_uart_receive_to_uplink_buffer(*get_sys_man_for_name("uplink"));
}

void Circle::flush() {
    transport->sync_tcp_read_some(transport->local_tcp_sock, std::chrono::milliseconds(1));
}

boost::asio::chrono::milliseconds Circle::get_state_time()
{
    uint32_t state_time_millis = 0;

    // std::cout << system_order[current_system]->timing->to_string();

    switch (current_state) {
        case STATE_ORDER::IDLE:
            state_time_millis = system_order.at(current_system)->timing->idle_millis*slowmo_gain;
            break;
        case STATE_ORDER::CMD_SEND:
            state_time_millis = system_order.at(current_system)->timing->command_millis*slowmo_gain;
            break;
        case STATE_ORDER::DATA_RECV:
            state_time_millis = system_order.at(current_system)->timing->reply_millis*slowmo_gain;
            break;
        default:
            utilities::debug_print("switch/case in Circle::get_state_time() fell through\n");
            state_time_millis = 1;
            break;
    }

    utilities::debug_print("\twaiting " + std::to_string(state_time_millis) + " ms\n");
    return boost::asio::chrono::milliseconds(state_time_millis);
}

SystemManager *Circle::get_sys_man_for_name(std::string name) {
    for (auto sys_man: system_order) {
        if (sys_man->system.name == name) {
            return sys_man.get();
        }
    }
    return nullptr;
}

SystemManager *Circle::get_sys_man_for_hex(uint8_t hex) {
    for (auto sys_man: system_order) {
        if (sys_man->system.hex == hex) {
            return sys_man.get();
        }
    }
    return nullptr;
}

void Circle::send_global_health() {
    utilities::debug_print("Circle::send_global_health()\tcalled.");
    
    SystemManager* hk = Circle::get_sys_man_for_name("housekeeping");

    std::vector<uint8_t> data = make_global_health_packet();

    utilities::debug_print("Circle::send_global_health()\tcalled.");
    DownlinkBufferElement dbe_health(&(hk->system), &(deck->get_sys_for_name("gse")), RING_BUFFER_TYPE_OPTIONS::PING);
    dbe_health.set_payload(data);
    transport->downlink_buffer->enqueue(dbe_health);
}

std::vector<uint8_t> Circle::make_global_health_packet() {
    utilities::debug_print("Circle::make_global_health_packet()\tcalled.");
    std::vector<uint8_t> content(system_order.size()*4);
    
    size_t k = 0;
    for (size_t i=0; i<system_order.size(); ++i) {
        content[k++] = system_order[i]->system.hex;
        content[k++] = static_cast<uint8_t>(system_order[i]->system_state);
        content[k++] = static_cast<uint8_t>((system_order[i]->errors >> 8 & 0xff));
        content[k++] = static_cast<uint8_t>((system_order[i]->errors & 0xff));
    }

    auto this_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint32_t out_time = static_cast<uint32_t>(this_time & 0xffffffff);
    auto time_bytes = utilities::splat_to_nbytes(4, out_time);

    std::vector<uint8_t> packet;
    packet.insert(packet.end(), time_bytes.begin(), time_bytes.end());
    packet.push_back(0x00);
    // find the uplink command counter, wrap it at 1 byte, and send it for confirmation.
    packet.push_back((Circle::get_sys_man_for_name("uplink")->counter % 256) & 0xff);
    packet.insert(packet.end(), content.begin(), content.end());
    return packet;
}