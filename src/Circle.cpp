#include "Circle.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <cmath>


Circle::Circle(double new_period_s, std::vector<std::shared_ptr<SystemManager>> new_system_order, CommandDeck new_deck, std::shared_ptr<TransportLayerMachine> new_transport, boost::asio::io_context &new_context): 
    deck(new_deck),
    transport(new_transport),
    system_order(new_system_order) {
    
    if (new_period_s <= 0) {
        utilities::error_print("invalid period.\n");
    }
    period_s = new_period_s;

    // debug:
    // system_order.resize(new_system_order.size());
    // system_order = std::move(new_system_order);
    // utilities::debug_print("order size: " + std::to_string(new_system_order.size()) + "\n");
    utilities::debug_print("order size: " + std::to_string(system_order.size()) + "\n");

    current_system = 0;
    current_state = STATE_ORDER::CMD_SEND;

    slowmo_gain = 1;

    normalize_times_to_period();

    period_ms = boost::asio::chrono::milliseconds(int(period_s*1000));
    timer = new boost::asio::steady_timer(new_context, period_ms);
    timer->async_wait(boost::bind(&Circle::update_state, this));
}



void Circle::update_state() {

    utilities::debug_print("current system: " + system_order[current_system]->system.name + "\n");
    utilities::debug_print("current state: " + std::to_string(std::to_underlying(current_state)) + "\n");

    boost::asio::chrono::milliseconds state_time = get_state_time();
    timer->expires_at(timer->expires_at() + state_time);
    timer->async_wait(boost::bind(&Circle::update_state, this));

    if (current_state == STATE_ORDER::IDLE) {
        current_state = STATE_ORDER::CMD_SEND;
        current_system = (current_system + 1)%system_order.size();
    } else {
        ++current_state;
    }
}

void Circle::manage_system() {
    if (system_order[current_system]->system == deck.get_sys_for_name("cdte1")) {
    
    } else {

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

    std::cout << system_order[current_system]->timing->to_string();

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
