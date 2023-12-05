#ifndef CIRCLE_H
#define CIRCLE_H

#include "Parameters.h"
#include "Buffers.h"
#include "Systems.h"
#include "Commanding.h"
#include "TransportLayer.h"
#include "Utilities.h"

#include "moodycamel/concurrentqueue.h"

#include <boost/asio.hpp>

#include <vector>
#include <unordered_map>
#include <queue>
#include <memory>


class Circle {
    public:
        Circle(
            double new_period_s,
            std::vector<std::shared_ptr<SystemManager>> new_system_order,
            std::shared_ptr<CommandDeck> new_deck,
            std::shared_ptr<TransportLayerMachine> new_transport,
            boost::asio::io_context& new_context
        );

        void pause();
        void init();

        void update_state();
        
        /**
         * @brief Initialize all systems in `::system_order`.
         * 
         * Delegates initialization to `::init_housekeeping()`, `::init_cdte()`, etc. These all send specific commands to initialize their systems. 
         */
        void init_systems();

        void init_housekeeping();
        void init_cdte();
        void init_cmos();
        void init_timepix();
        
        /**
         * @brief Perform state- and system-specific actions on each `System`.
         * Currently handles all systems by lookup of `::current_system`. In the future, delegate management to `::manage_cdte_state()`, `::manage_housekeeping_state()`, etc.
         */
        void manage_systems();

        void normalize_times_to_period();

        std::vector<std::shared_ptr<SystemManager>> system_order;
        std::shared_ptr<CommandDeck> deck;
        std::shared_ptr<TransportLayerMachine> transport;
        
        double period_s;
        boost::asio::steady_timer* timer;

        size_t current_system;
        STATE_ORDER current_state;

        uint32_t slowmo_gain;
    
    private:
        void manage_cdte_state();
        void manage_housekeeping_state();
        boost::asio::chrono::milliseconds get_state_time();

        SystemManager* get_sys_man_for_name(std::string name);
        SystemManager* get_sys_man_for_hex(uint8_t hex);

        boost::asio::chrono::milliseconds period_ms;

        // find better way to save this (factor into SystemManager hk):
        // size_t last_cdte_write_pointer;
        // size_t last_cmos_pc_write_pointer;
        // size_t last_cmos_ql_write_pointer;
        // bool cmos_pc_state; 

};

#endif