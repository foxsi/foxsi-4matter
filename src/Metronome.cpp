#include "Metronome.h"
#include "Utilities.h"
#include <boost/bind.hpp>
#include <iostream>

Metronome::Metronome(double period, boost::asio::io_context& io_context) {
    subsystem = SUBSYSTEM_ORDER::HOUSEKEEPING;
    state = STATE_ORDER::CMD_SEND;

    if(period <= 0) {
        throw "period must be positive\n";
    }

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

    Metronome::update();

    // update timer
    timer->expires_at(timer->expires_at() + tick_period_milliseconds);
    timer->async_wait(boost::bind(&Metronome::tick, this));
}

void Metronome::update() {
    if(state == STATE_ORDER::DATA_STORE) {
        ++subsystem;
    }
    ++state;
    // modify tick_period if necessary
}

