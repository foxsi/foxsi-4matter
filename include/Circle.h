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
            CommandDeck new_deck,
            std::shared_ptr<TransportLayerMachine> new_transport,
            boost::asio::io_context& new_context
        );

        void init();
        void pause();

        void update_state();
        void manage_system();

        void normalize_times_to_period();

        std::vector<std::shared_ptr<SystemManager>> system_order;
        CommandDeck deck;
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

        boost::asio::chrono::milliseconds period_ms;

};

#endif