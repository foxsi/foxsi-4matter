#pragma once
#ifndef METRONOME_H
#define METRONOME_H

#include "Parameters.h"
#include <boost/asio.hpp>

class Metronome {
    public:
        Metronome(double period, boost::asio::io_context& io_context);
        // Metronome();
        // ~Metronome();

        // boost::asio::io_context io_context;
        
        boost::asio::chrono::milliseconds tick_period_milliseconds;         // delay after last tock before tick method called
        // boost::asio::chrono::seconds tock_period_seconds;         // delay after tick or tock before tock method called

        boost::asio::steady_timer* timer;

        STATE_ORDER state;
        SUBSYSTEM_ORDER subsystem;

        // int next();
        void run();
        void update();
        // void tick(const boost::system::error_code& error);
        void tick();
        void tock(const boost::system::error_code& error);

};

#endif