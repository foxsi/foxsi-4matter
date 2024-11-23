#ifndef FOXSIMILE_H
#define FOXSIMILE_H

#pragma once

#include "Buffers.h"
#include "Commanding.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>
#include <map>
#include <unordered_map>

namespace foxsimile {

    enum class response_strategies: uint8_t {
        ignore  =       0x00,
        lookup  =       0x01,
        random  =       0x02,
        mmap    =       0x03
    };

    namespace mmap {
        /**
         * @brief Determine the type of memory for the range spanning `[address : offset]`. 
         * 
         * This is used to figure out if the memory is photon counting data, quick look data, or housekeeping.
         * 
         * @param sys `System` object defining the system that contains this memory.
         * @param address start address to check.
         * @param offset length (in bytes) to check.
         * @return type of memory stored here.
         */
        RING_BUFFER_TYPE_OPTIONS is_ring_buffer(System& sys, size_t address, size_t offset);
        /**
         * @brief Push the `system`'s write pointer forward by one frame increment, and update the appropriate field in the memory map `mmap`.
         * 
         * @param system the `System` containing the memory.
         * @param type the type of memory who's write pointer needs advancing.
         * @param mmap the memory map representing the `System`'s current physical memory.
         */
        void advance_write_pointer(System& system, RING_BUFFER_TYPE_OPTIONS type, std::unordered_map<uint8_t, std::vector<uint8_t>>& mmap);
        /**
         * @brief Push the `sys_man`'s write pointer forward by one frame increment, and update the appropriate field in the memory map `mmap`.
         * 
         * @param sys_man the manager of the `System` containing the memory.
         * @param type the type of memory who's write pointer needs advancing.
         * @param mmap the memory map representing the `System`'s current physical memory.
         */
        void advance_write_pointer(SystemManager& sys_man, RING_BUFFER_TYPE_OPTIONS type, std::unordered_map<uint8_t, std::vector<uint8_t>>& mmap);
        /**
         * @brief Currently unimplemented.
         * Will be used in the future to increment unixtime/TI counters.
         */
        void advance_time(SystemManager& sys_man, RING_BUFFER_TYPE_OPTIONS type, std::vector<uint8_t>& mmap);
        /**
         * @brief Currently unimplemented.
         * Will be used in the future to randomly modify the data in the memory map.
         */
        void random_update(SystemManager& sys_man, RING_BUFFER_TYPE_OPTIONS type, std::vector<uint8_t>& mmap);
    };

    /**
     * @brief Class for mocking detector system responses to commands.
     * 
     * This object can be used as an endpoint for the Formatter to talk to, emulating a flight subsystem. Responses to Formatter requests can be provided in two ways:
     * 1. As a lookup table, mapping full request messages to full responses (a `std::map` with both keys and values `std::vector<uint8_t>`)
     * 2. As a memory map, to be accessed by SpaceWire's Remote Memory Access Protocol (RMAP).
     * 
     * Note that for option (2), messages from the Formatter are fed SPMU-001 style, over TCP and with a 12-byte preheader specifying SpaceWire message length.
     */
    class Responder {
        public:
            /**
             * @brief Construct a new Responder using a response lookup map.
             * 
             * @param new_response_lookup lookup table, mapping received request messages onto responses.
             * @param new_system_managers list of `SystemManager` objects, representing the physical system(s) this `Responder` emulates.
             * @param new_deck deck of commands that may be sent to each system.
             * @param context thread context for running communication.
             */
            Responder(
                std::map<std::vector<uint8_t>, std::vector<uint8_t>> new_response_lookup,
                std::vector<std::shared_ptr<SystemManager>> new_system_managers, 
                std::shared_ptr<CommandDeck> new_deck, 
                boost::asio::io_context& context
            );

            /**
             * @brief Construct a new Responder using a list of memory maps.
             * 
             * This Responder will query the provided memory map files based on received SpaceWire RMAP messages, directly indexing the provided files as if they were mass storage.
             * 
             * @param new_response_mmap_files a lookup table of memory map filenames, keyed by System ID hex codes (see `foxsi4-commands/systems.json`).
             * @param new_system_managers list of `SystemManager` objects, representing the physical system(s) this `Responder` emulates. Each relevant `SystemManager::System::hex` is used as a key to `new_response_mmap_files` to determine a response to received SpaceWire messages.
             * @param new_deck deck of commands that may be sent to each system.
             * @param context thread context for running communication.
             */
            Responder(
                std::map<uint8_t, std::string> new_response_mmap_files, 
                std::vector<std::shared_ptr<SystemManager>> new_system_managers, 
                std::shared_ptr<CommandDeck> new_deck, 
                boost::asio::io_context& context
            );

            /**
             * @brief Build the `Responder::lookup_code_by_rmap_address` map using information in `SystemManager`.
             */
            void construct_rmap_lookup();
            
            /**
             * @brief Asynchronously wait for data on the socket, callback to `::await_full_buffer` when complete.
             */
            void async_receive();
            /**
             * @brief Callback to `::async_receive()` on TCP accept.
             * 
             * @param err accept error.
             */
            void handle_accept(const boost::system::error_code& err);
            /**
             * @brief Logic to decide if a full message has been received or not.
             * 
             * This is necessary because TCP is a streaming protocol: a clump of bytes transmitted as a single message may be reported by the receiving socket in several blocks. So this function contains buffering logic to reassemble packets. The *expected* message size is presumed to be present in the SPMU-001 RMAP header, if the total message is longer than 12 B. Otherwise, if the total message is shorter than 12 B, the `Responder` will try to look up the response in the response lookup table.
             * 
             * @param err receive error.
             * @param byte_count number of bytes last received by the socket.
             */
            void await_full_buffer(const boost::system::error_code& err, std::size_t byte_count);
            /**
             * @brief Waits a nominal delay, then sends the provided response.
             * 
             * @param response the response message to send.
             */
            void send_response(std::vector<uint8_t> response);
            
            /**
             * @brief Handle remote host disconnect from `Responder`, and start waiting for a new connection.
             */
            void end_session();

            /**
             * @brief Construct the response to a received query, either using lookup tables or the memory map.
             * 
             * @param message the request message received.
             * @return the response to transmit back to host.
             */
            std::vector<uint8_t> get_response(std::vector<uint8_t> message);
            /**
             * @brief For lookup-based responses, insert a potential response into the lookup table.
             * 
             * @param message the query that begets the response.
             * @param response the response to send when `message` is received.
             * @return `true` if the response was added to the table successfully, `false` otherwise.
             */
            bool add_response(std::vector<uint8_t> message, std::vector<uint8_t> response);
            /**
             * @brief Attempt to identify the system transmitting the `message` based on the value of the Target Logical Address in the SpaceWire RMAP packet. 
             * 
             * Note: there is no guarantee that Target Logical Address is unique in poorly-constructed RMAP networks, in which case this lookup will fail to provide reliable sender information.
             * 
             * If lookup fails altogether, 0xff will be returned.
             * 
             * @param message the SpaceWire RMAP message used to identify the sender.
             * @return the byte identifier of the `System` transmitting the message.
             */
            uint8_t identify_sender(std::vector<uint8_t> message);

            /**
             * @brief Modify the default response message.
             * 
             * @param message the new default response.
             */
            void add_default_response(std::vector<uint8_t> message);

            /**
             * @brief Underlying TCP socket object used for communication.
             */
            boost::asio::ip::tcp::socket socket;
            /**
             * @brief TCP acceptor for remote hosts.
             */
            boost::asio::ip::tcp::acceptor acceptor;

        private:
            /**
             * @brief A name for this `Responder`.
             */
            std::string name;
            /**
             * @brief Handle expiration of the frame incrementing timer (used for moving RMAP write pointers on a clock).
             */
            void handle_frame_timer();
            /**
             * @brief Timer object for incrementing RMAP write pointers.
             */
            boost::asio::steady_timer* frame_timer;
            /**
             * @brief Delay between RMAP write pointer updates.
             */
            std::chrono::milliseconds frame_update_ms;

            /**
             * @brief Lookup table for received messages, mapping received message onto response.
             * 
             * Can be used as an alternative to a memory map file, see constructors.
             */
            std::map<std::vector<uint8_t>, std::vector<uint8_t>> response_lookup;
            /**
             * @brief Lookup table for response memory map files, keyed by `System` ID code. 
             * 
             * One `Responder` can provide responses for several unique `System`s with different ID codes. This is useful e.g. for emulating the SPMU-001 behavior, where a single IP address (`Responder::socket`) is the router for several SpaceWire systems.
             */
            std::map<uint8_t, std::string> response_mmap_files;
            /**
             * @brief Raw memory maps (read for `response_mmap_files`), keyed by `System` ID code.
             */
            std::unordered_map<uint8_t, std::vector<uint8_t>> mmaps;
            
            /**
             * @brief Produce reply to host for a given `message` query.
             * 
             * This method will either lookup in the `::response_lookup` or parse the received message as a SpaceWire RMAP packet to get a response from a memory map, depending on the `::response_strategy` of this `Responder`.
             * 
             * @param message the query message from host.
             * @return a reply packet to the `message`.
             */
            std::vector<uint8_t> make_reply(std::vector<uint8_t> message);

            /**
             * @brief Whether to use lookup table or memory map for building responses.
             */
            response_strategies response_strategy;
            
            /**
             * @brief Redundant with `::response_strategy`, should deprecate.
             */
            bool lookup;
            /**
             * @brief Default response message to send if lookup or memory fail.
             * 
             */
            std::vector<uint8_t> default_response;

            /**
             * @brief Last buffer of data received by socket.
             */
            std::vector<uint8_t> read_buffer;
            /**
             * @brief Accumulation of data received on socket. Cleared when the message is considered complete.
             */
            std::vector<uint8_t> message_accumulator;

            /**
             * @brief May be used in the future for more precise response timing.
             */
            uint64_t static_latency_us;

            /**
             * @brief May be used in the future for more precise response timing.
             */
            uint64_t bytewise_latency_us;

            /**
             * @brief List of `SystemManager` objects that this `Responder` can emulate. 
             * 
             * Used to determine RMAP properties for response, if needed.
             */
            std::vector<std::shared_ptr<SystemManager>> system_managers;

            /**
             * @brief Deck of admissable command messages for each `System`.
             */
            std::shared_ptr<CommandDeck> deck; 

            /**
             * @brief Table to find `System::hex` ID code for each RMAP address provided.
             * 
             * Note: in poorly-constructed RMAP networks, there is no guarantee of RMAP address uniqueness.
             */
            std::unordered_map<uint8_t, uint8_t> lookup_code_by_rmap_address;
    };
}

#endif