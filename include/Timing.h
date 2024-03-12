/**
 * @file Timing.h
 * @author Thanasi Pantazides
 * @brief Wrapper for timing- and latency-related information.
 * @version v1.0.1
 * @date 2024-03-11
 */
#ifndef TIMING_H
#define TIMING_H

#include "Parameters.h"
/**
 * @brief A convenience datastructure for passing inner loop timing information.
 * 
 * The inner timing loop is run for each onboard system the Formatter communicates with. The inner loop has a different period for each onboard system. Inside the inner loop, 
 * 1. commands are sent to the system,
 * 2. a data request is sent to the system,
 * 3. the system responds with buffered data to telemeter to the ground,
 * 4. there is a grace period to end the exchange with the system.
 * 
 * This class stores these fields for the inner loop.
 */
class Timing {
    public:
        /**
         * @brief Construct a new empty `Timing` object.
         * 
         * The object's fields can then be populated with `Timing::add_times_seconds(...).
         */
        Timing();
        
        /**
         * @brief Populate fields of `Timing`.
         * 
         * @param total_allocation the total amount of time (in seconds) that comprises `Timing::command_millis`, `Timing::request_millis`, `Timing::reply_millis`, and `Timing::idle_millis`. 
         * 
         * @param command_time the amount of time (in seconds) spent sending commands.
         * @param request_time the amount of time (in seconds) spent requesting data.
         * @param reply_time the amount of time (in seconds) spent receiving response data/forwarding.
         * @param idle_time the amount of idle time (in seconds) at the end.
         */
        void add_times_seconds(double total_allocation, double command_time, double request_time, double reply_time, double idle_time);

        /**
         * @brief Clean up fields of `Timing` so that `Timing::command_millis`, `Timing::request_millis`, `Timing::reply_millis`, and `Timing::idle_millis` sum to `Timing::period_millis`.
         * 
         */
        void resolve_times();

        const std::string to_string();

        uint32_t period_millis;
        uint32_t command_millis;
        uint32_t request_millis;
        uint32_t reply_millis;
        uint32_t idle_millis;

        uint32_t retry_max_count;
        uint32_t timeout_millis;
        uint32_t intercommand_space_millis;
};

#endif