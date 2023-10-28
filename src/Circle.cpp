#include "Circle.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <cmath>
#include <ctime>
#include <thread>
#include <chrono>

Circle::Circle(double new_period_s, std::vector<std::shared_ptr<SystemManager>> new_system_order, std::shared_ptr<CommandDeck> new_deck, std::shared_ptr<TransportLayerMachine> new_transport, boost::asio::io_context &new_context): 
    deck(new_deck),
    transport(new_transport),
    system_order(new_system_order) {
    
    if (new_period_s <= 0) {
        utilities::error_print("invalid period.\n");
    }
    period_s = new_period_s;

    current_system = 0;
    current_state = STATE_ORDER::CMD_SEND;
    
    normalize_times_to_period();
    init_systems();

    // write:
    // 1. filtering of uplink buffer into housekeeping system
    // 2. this blocking TransportLayerMachine::await_loop_begin() to block until uplink command received to proceed.
    transport->await_loop_begin();

    slowmo_gain = 1;


    period_ms = boost::asio::chrono::milliseconds(int(period_s*1000));
    timer = new boost::asio::steady_timer(new_context, period_ms);
    timer->async_wait(boost::bind(&Circle::update_state, this));
}

void Circle::update_state() {

    utilities::debug_print("update time: " + std::to_string(std::time(nullptr)) + " [s]\n");
    utilities::debug_print("current system: " + system_order[current_system]->system.name + "\n");
    utilities::debug_print("current state: " + std::to_string(static_cast<uint8_t>(current_state)) + "\n");

    if (current_state == STATE_ORDER::CMD_SEND) {
        manage_systems();
    }
    // debug
    // transport->async_udp_send_downlink_buffer();

    boost::asio::chrono::milliseconds state_time = get_state_time();
    timer->expires_at(timer->expiry() + state_time);
    timer->async_wait(boost::bind(&Circle::update_state, this));

    if (current_state == STATE_ORDER::IDLE) {
        current_state = STATE_ORDER::CMD_SEND;
        current_system = (current_system + 1)%system_order.size();
    } else {
        ++current_state;
    }
}

void Circle::init_systems() {
    // for housekeeping, init and start a conversion. (0x01 0xff, then 0x01 0xf0, then same for 0x02).
    init_housekeeping();

    init_cdte();

    // init_cmos();
}

void Circle::init_housekeeping() {
    utilities::debug_print("initializing housekeeping system\n");
    // setup both sensors
    transport->sync_tcp_housekeeping_send(deck->get_command_bytes_for_sys_for_code(deck->get_sys_for_name("housekeeping").hex, 0x06));
    transport->sync_tcp_housekeeping_send(deck->get_command_bytes_for_sys_for_code(deck->get_sys_for_name("housekeeping").hex, 0x07));
    // sleep to give LTC2983s time to set up:
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // start conversion
    transport->sync_tcp_housekeeping_send(deck->get_command_bytes_for_sys_for_code(deck->get_sys_for_name("housekeeping").hex, 0x04));
    transport->sync_tcp_housekeeping_send(deck->get_command_bytes_for_sys_for_code(deck->get_sys_for_name("housekeeping").hex, 0x05));
}
void Circle::init_cdte() {
    utilities::debug_print("initializing cdte system\n");
    System& cdtede = deck->get_sys_for_name("cdtede");
    System& cdte1 = deck->get_sys_for_name("cdte1");

    auto delay = std::chrono::milliseconds(2000);

    // Check canister ping status       0x08 0x8a
    utilities::debug_print("checking canister status...\n");
    std::vector<uint8_t> can_status = transport->sync_tcp_send_command_for_sys(cdtede, deck->get_command_for_sys_for_code(cdtede.hex, 0x8a));
    can_status = transport->get_reply_data(can_status, cdtede.hex);
    utilities::debug_print("canisters status: ");
    utilities::hex_print(can_status);
    std::this_thread::sleep_for(delay);

    // DE init                          0x08 0x09
    transport->sync_tcp_send_command_for_sys(cdtede, deck->get_command_for_sys_for_code(cdtede.hex, 0x09));
    std::this_thread::sleep_for(delay);

    // DE standby                       0x08 0x0a
    transport->sync_tcp_send_command_for_sys(cdtede, deck->get_command_for_sys_for_code(cdtede.hex, 0x0a));
    std::this_thread::sleep_for(delay);

    // DE observe                       0x08 0x0b
    transport->sync_tcp_send_command_for_sys(cdtede, deck->get_command_for_sys_for_code(cdtede.hex, 0x0b));
    std::this_thread::sleep_for(delay);

    // Canister 1 start                 0x09 0x11
    // transport->sync_tcp_send_command_for_sys(cdte1, deck->get_command_for_sys_for_code(cdte1.hex, 0x11));
    // std::this_thread::sleep_for(delay);

    // Apply HV 0V for all canister     0x08 0x13
    // transport->sync_tcp_send_command_for_sys(cdtede, deck->get_command_for_sys_for_code(cdtede.hex, 0x13));
    // std::this_thread::sleep_for(delay);

    // Apply HV 60V for all canister    0x08 0x14
    transport->sync_tcp_send_command_for_sys(cdtede, deck->get_command_for_sys_for_code(cdtede.hex, 0x14));
    std::this_thread::sleep_for(delay);
    
    // Start observe for all canister   0x08 0x11
    transport->sync_tcp_send_command_for_sys(cdtede, deck->get_command_for_sys_for_code(cdtede.hex, 0x11));
    std::this_thread::sleep_for(delay);

    // later, will need to end observe and lower bias for all.
}
void Circle::init_cmos() {
    utilities::debug_print("initializing cmos system\n");
}
void Circle::init_timepix() {
    utilities::debug_print("initializing timepix system\n");
}

void Circle::manage_systems() {
    if (system_order[current_system]->system == deck->get_sys_for_name("cdte1")) {
        utilities::debug_print("managing cdte1 system\n");

        // 0 is cdte1
        // 1 is cdtede
        // 2 is housekeeping

        transport->sync_tcp_send_buffer_commands_to_system(*(system_order[0]));
        transport->sync_tcp_send_buffer_commands_to_system(*(system_order[1]));
        transport->sync_remote_buffer_transaction(*system_order[0], RING_BUFFER_TYPE_OPTIONS::PC);
        bool has_data = transport->sync_udp_send_all_downlink_buffer();

    } else if (system_order[current_system]->system == deck->get_sys_for_name("housekeeping")) {
        utilities::debug_print("managing housekeeping system\n");
        // read out both sensors (0x01 0xf2, then 0x02 0xf2)
        // then start a new conversion (0x01 0xf0, then 0x02 0xf0)

        transport->sync_tcp_send_buffer_commands_to_system(*system_order[2]);

        transport->sync_tcp_housekeeping_send({0x01, 0xf0});
        transport->sync_tcp_housekeeping_send({0x02, 0xf0});
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        // todo: no more magic numbers.

        std::vector<uint8_t> reply1 = transport->sync_tcp_housekeeping_transaction({0x01, 0xf2});
        std::vector<uint8_t> reply1_time = utilities::splat_to_nbytes(4, static_cast<uint32_t>(std::time(nullptr)));
        
        
        std::vector<uint8_t> reply2 = transport->sync_tcp_housekeeping_transaction({0x02, 0xf2});
        std::vector<uint8_t> reply2_time = utilities::splat_to_nbytes(4, static_cast<uint32_t>(std::time(nullptr)));

        std::vector<uint8_t> head1 = {0x01, 0x00};
        std::vector<uint8_t> head2 = {0x02, 0x00};
        reply1_time.insert(reply1_time.begin(), head1.begin(), head1.end());
        reply1_time.insert(reply1_time.end(), reply1.begin(), reply1.end());
        reply2_time.insert(reply2_time.begin(), head2.begin(), head2.end());
        reply2_time.insert(reply2_time.end(), reply2.begin(), reply2.end());

        DownlinkBufferElement dbe1(&(deck->get_sys_for_name("housekeeping")), &(deck->get_sys_for_name("gse")), 
        RING_BUFFER_TYPE_OPTIONS::NONE);
        DownlinkBufferElement dbe2(&(deck->get_sys_for_name("housekeeping")), &(deck->get_sys_for_name("gse")), 
        RING_BUFFER_TYPE_OPTIONS::NONE);
        // utilities::debug_print("dbe1 max payload: " + std::to_string(dbe1.get_max_packet_size()) + "\n");
        dbe1.set_payload(reply1_time);
        dbe2.set_payload(reply2_time);

        // utilities::debug_print("from RTD1: ");
        // utilities::hex_print(reply1);
        // utilities::debug_print("whole RTD1 downlink: ");
        // utilities::hex_print(reply1_time);

        transport->downlink_buffer->enqueue(dbe1);
        transport->downlink_buffer->enqueue(dbe2);

        bool has_data = transport->sync_udp_send_all_downlink_buffer();
        // while (has_data) {
        //     has_data = transport->sync_udp_send_all_downlink_buffer();
        // }
        
    } else {
        utilities::debug_print("system management fell through in Circle for " + system_order[current_system]->system.name +  "\n");

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

boost::asio::chrono::milliseconds Circle::get_state_time() {
    uint32_t state_time_millis = 0;

    // std::cout << system_order[current_system]->timing->to_string();

    switch (current_state) {
        case STATE_ORDER::IDLE:
            state_time_millis = system_order[current_system]->timing->idle_millis*slowmo_gain;
            break;
        case STATE_ORDER::CMD_SEND:
            state_time_millis = system_order[current_system]->timing->command_millis*slowmo_gain;
            break;
        case STATE_ORDER::DATA_RECV:
            state_time_millis = system_order[current_system]->timing->reply_millis*slowmo_gain;
            break;
        default:
            utilities::debug_print("switch/case in Circle::get_state_time() fell through\n");
            state_time_millis = 1;
            break;
    }

    utilities::debug_print("\twaiting " + std::to_string(state_time_millis) + " ms\n");
    return boost::asio::chrono::milliseconds(state_time_millis);
}