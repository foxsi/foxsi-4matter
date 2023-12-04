#ifndef FOXSIMILE_H
#define FOXSIMILE_H

#pragma once

#include "Utilities.h"
#include "Buffers.h"
#include "Commanding.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>
#include <map>

namespace foxsimile {

    enum class response_strategies: uint8_t {
        ignore  =       0x00,
        lookup  =       0x01,
        random  =       0x02,
        mmap    =       0x03
    };

    /**
     * @brief Abstract base for mocking detector system responses to commands.
     */
    class Responder {
        public:
            Responder(
                bool do_except, 
                std::map<std::vector<uint8_t>, std::vector<uint8_t>> new_response_lookup,
                std::shared_ptr<SystemManager> new_system_manager, std::shared_ptr<CommandDeck> new_deck, 
                boost::asio::io_context& context
            );

            Responder(
                bool do_except, 
                std::string new_response_mmap_file, 
                std::shared_ptr<SystemManager> new_system_manager, std::shared_ptr<CommandDeck> new_deck, 
                boost::asio::io_context& context
            );

            void async_receive();
            void handle_accept(const boost::system::error_code& err);
            void await_full_buffer(const boost::system::error_code& err, std::size_t byte_count);
            void send_response(std::vector<uint8_t> response);
            void end_session();

            std::vector<uint8_t> get_response(std::vector<uint8_t> message);
            bool add_response(std::vector<uint8_t> message, std::vector<uint8_t> response);

            void add_default_response(std::vector<uint8_t> message);
            void set_bad_request_behavior(bool do_except);

            boost::asio::ip::tcp::socket socket;
            boost::asio::ip::tcp::acceptor acceptor;

        private:
            std::map<std::vector<uint8_t>, std::vector<uint8_t>> response_lookup;
            std::string response_mmap_file;

            std::vector<uint8_t> make_reply(std::vector<uint8_t> message);

            response_strategies response_strategy;
            
            bool except_on_bad_request;
            bool lookup;
            std::vector<uint8_t> default_response;

            std::vector<uint8_t> read_buffer;
            std::vector<uint8_t> message_accumulator;

            uint64_t static_latency_us;
            uint64_t bytewise_latency_us;

            std::shared_ptr<SystemManager> system_manager;

            std::shared_ptr<CommandDeck> deck; 
    };
}

#endif