#ifndef FOXSIMILE_H
#define FOXSIMILE_H

#pragma once

#include "Utilities.h"
#include "Buffers.h"
#include "CommandDeck.h"
#include <boost/asio.hpp>
#include <memory>

namespace foxsimile {
    /**
     * @brief Abstract base for mocking detector system responses to commands.
     */
    class Responder {
        public:
            std::vector<uint8_t> get_response(std::vector<uint8_t>);
            bool add_response(std::vector<uint8_t>);

            void add_default_response(std::vector<uint8_t>);
            void set_bad_request_behavior(bool do_except);

        private:
            std::unordered_map<std::vector<uint8_t>, std::vector<uint8_t>> response_lookup;

            bool except_on_bad_request;
            std::vector<uint8_t> default_response;

            uint64_t static_latency_us;
            uint64_t bytewise_latency_us;

            std::shared_ptr<SystemManager> system_manager;
            boost::asio::ip::tcp::socket socket;

            std::shared_ptr<CommandDeck> deck; 
    };

    class CdTeResponder: Responder {
        public:

        private:


    };

    class HousekeepingResponder: Responder {
        public:

        private:


    }

}

#endif