#include "Metronome.h"
#include "Utilities.h"
#include <boost/bind.hpp>
#include <iostream>

Metronome::Metronome(double period, std::unordered_map<SUBSYSTEM_ORDER, std::unordered_map<STATE_ORDER, double>> new_lookup_periods, boost::asio::io_context& io_context) {
    subsystem = SUBSYSTEM_ORDER::HOUSEKEEPING;
    state = STATE_ORDER::CMD_SEND;

    // todo: add property TransportLayerMachine (and bind to methods as we tick through)

    // todo: add LineInterface normalization of json "time" fields, and generate the period lookup table

    if(period <= 0) {
        throw "period must be positive\n";
    }

    lookup_periods = new_lookup_periods;

    tick_period_milliseconds = boost::asio::chrono::milliseconds(int(period*1000));
    timer = new boost::asio::steady_timer(io_context, tick_period_milliseconds);
    timer->async_wait(boost::bind(&Metronome::tick, this));
}

// Metronome::~Metronome() {}

void Metronome::run() {
    timer->async_wait(boost::bind(&Metronome::tick, this));
    // io_context.run();
}

void Metronome::tick() {

    std::cout << "tick\n";
    std::cout << "\tsubsystem: " << static_cast<unsigned short>(subsystem);
    std::cout << "\tstate: " << static_cast<unsigned short>(state) << "\n";

    // update timer
    // timer->expires_at(timer->expires_at() + tick_period_milliseconds);

    boost::asio::chrono::milliseconds next_period = boost::asio::chrono::milliseconds(int(lookup_periods[subsystem][state]));
    timer->expires_at(timer->expires_at() + next_period);
    timer->async_wait(boost::bind(&Metronome::tick, this));

    Metronome::manage_system();

    Metronome::update_state();
}

void Metronome::update_state() {
    if(state == STATE_ORDER::IDLE) {
        ++subsystem;
    }
    ++state;
    // modify tick_period if necessary
}

void Metronome::manage_system() {
    if(subsystem == SUBSYSTEM_ORDER::CDTE_1 || subsystem == SUBSYSTEM_ORDER::CDTE_2 || subsystem == SUBSYSTEM_ORDER::CDTE_3 || subsystem == SUBSYSTEM_ORDER::CDTE_4) {
        Metronome::manage_cdte_state();
    } else if(subsystem == SUBSYSTEM_ORDER::CMOS_1 || subsystem == SUBSYSTEM_ORDER::CMOS_2) {
        Metronome::manage_cmos_state();
    } else if(subsystem == SUBSYSTEM_ORDER::TIMEPIX) {
        Metronome::manage_timepix_state();
    } else if(subsystem == SUBSYSTEM_ORDER::HOUSEKEEPING) {
        Metronome::manage_housekeeping_state();
    } else {
        // problem
    }
}

void Metronome::manage_cdte_state() {
    // something to do with TransportLayerMachine
}

void Metronome::manage_cmos_state() {
    // something to do with TransportLayerMachine
}

void Metronome::manage_timepix_state() {
    // something to do with TransportLayerMachine
}

void Metronome::manage_housekeeping_state() {
    // something to do with TransportLayerMachine
}