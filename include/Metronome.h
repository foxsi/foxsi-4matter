#pragma once
#ifndef METRONOME_H
#define METRONOME_H

#include "Parameters.h"
#include <boost/asio.hpp>
#include <map>

/**
 * @deprecated No longer used, superseded by `Circle`. 
 */
class Metronome {
    public:
        Metronome(double period, std::unordered_map<SUBSYSTEM_ORDER, std::unordered_map<STATE_ORDER, double>> new_lookup_periods,boost::asio::io_context& io_context);
        // Metronome();
        // ~Metronome();

        // boost::asio::io_context io_context;
        
        boost::asio::chrono::milliseconds tick_period_milliseconds;         // delay after last tock before tick method called
        // boost::asio::chrono::seconds tock_period_seconds;         // delay after tick or tock before tock method called

        /**
         * @brief use to lookup timer period for each subsystem and state.
         * 
         */
        std::unordered_map<SUBSYSTEM_ORDER, std::unordered_map<STATE_ORDER, double>> lookup_periods;

        boost::asio::steady_timer* timer;

        STATE_ORDER state;
        SUBSYSTEM_ORDER subsystem;

        // int next();
        void run();
        void update_state();
        void manage_system();
        void manage_cdte_state();
        void manage_cmos_state();
        void manage_timepix_state();
        void manage_housekeeping_state();
        // void tick(const boost::system::error_code& error);
        void tick();
        void tock(const boost::system::error_code& error);

};

#endif