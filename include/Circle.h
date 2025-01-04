/**
 * @file Circle.h
 * @author Thanasi Pantazides
 * @brief Run loop around all onboard `System`s, retrieving and downlinking data.
 * @version v1.0.1
 * @date 2024-03-11
 */

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

/**
 * @brief Run the main loop to manage onboard `System`s.
 * This object runs a loop that performs the following actions for the onboard systems:
 *  1. Forward commands to `System` (sent to Formatter via uplink)
 *  2. Collect data from `System`
 *  3. Transmit data from `System` to ground.
 * 
 * `TransportLayerMachine` drives the communication interface with each `System`.
 * 
 * Some high-level behavior is implemented e.g. for Housekeeping system, which can be disabled/enabled via uplink intercept command.
 */
class Circle {
    public:
        Circle(
            double new_period_s,
            std::vector<std::shared_ptr<SystemManager>> new_system_order,
            std::shared_ptr<CommandDeck> new_deck,
            std::shared_ptr<TransportLayerMachine> new_transport,
            boost::asio::io_context& new_context
        );

        /**
         * @deprecated Unimplemented.
         */
        void pause();
        /**
         * @brief Begin running the main loop.
         */
        void start();
        /**
         * @deprecated Unimplemented. See `circle::init_systems()`.
         */
        void init();
        /**
         * @brief Update the current `STATE_ORDER` value.
         * @note The `STATE_ORDER` value changes through the loop, but there is no behavior that depends on the current `STATE_ORDER` value. So I would like to remove it later.
         */
        void update_state();
        
        /**
         * @brief Initialize all systems included in `Circle::system_order`.
         * Delegates initialization to `::init_housekeeping()`, `::init_cdte()`, etc. These all send specific commands to initialize their systems. 
         *  - For the Housekeeping system, ADC conversions are started.
         *  - For the CdTe system, ping status of each canister is checked.
         *  - For the CMOS system, linetime of each detector is read.
         *  - For the Timepix system, ping is checked.
         */
        void init_systems();

        void init_housekeeping();
        void init_cdte();
        void init_cmos();
        void init_timepix();
        
        /**
         * @brief Perform `System`-specific tasks for each value in `Circle::system_order`..
         * @note This is the core event loop in this software.
         * 
         * There is separate logic for each type of onboard system, but the general scheme is to first check for any uplink commands for a given system, forward them to the system, request typical data products from the system (usually detector data and housekeeping), then downlink all the received data. 
         */
        void manage_systems();

        /**
         * @brief Reads and discards any data in TCP sockets.
         * Specifically, calls `TransportLayerMachine::sync_tcp_read_some()` on `TransportLayerMachine::local_tcp_sock` and `TransportLayerMachine::local_tcp_housekeeping_sock`.
         */
        void flush();

        /**
         * @brief Add the Formatter status packet to the downlink buffer for transmission.
         */
        void send_global_health();


        /**
         * @brief Utility to normalized `Timing` data for each `SystemManager` to the total loop period.
         */
        void normalize_times_to_period();

        /**
         * @brief Assemble a packet reporting health of all the `System`s in `::system_order` for downlink.
         * 
         * @return std::vector<uint8_t> a health packet below the MTU derived from the `System` named .
         */
        std::vector<uint8_t> make_global_health_packet();

        /**
         * @brief The ordered list of `SystemManager`s that will be accessed in the event loop. 
         */
        std::vector<std::shared_ptr<SystemManager>> system_order;
        /**
         * @brief The `CommandDeck` to use to parse uplink commands.
         */
        std::shared_ptr<CommandDeck> deck;
        /**
         * @brief The `TransportLayerMachine` to use to transmit and receive data.
         */
        std::shared_ptr<TransportLayerMachine> transport;
        
        double period_s;
        /**
         * @brief A `boost::asio` timer object to drive the event loop.
         * @warning I would like to deprecate this (in favor of a counter loop), but haven't yet.
         */
        boost::asio::steady_timer* timer;

        /**
         * @brief The index (in `Circle::system_order`) of the `SystemManager` being accessed currently.
         */
        size_t current_system;
        /**
         * @brief The current `STATE_ORDER` value of the loop.
         * @warning I would like to deprecate this. It is not consumed anywhere currently.
         */
        STATE_ORDER current_state;
        
        uint32_t slowmo_gain;
    
    private:
        void record_uplink();
        void manage_cdte_state();
        void manage_housekeeping_state();
        boost::asio::chrono::milliseconds get_state_time();

        SystemManager* get_sys_man_for_name(std::string name);
        SystemManager* get_sys_man_for_hex(uint8_t hex);

        boost::asio::chrono::milliseconds period_ms;

        bool run;
};

#endif