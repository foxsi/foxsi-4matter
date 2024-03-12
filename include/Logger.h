#ifndef LOGGER_H
#define LOGGER_H

#include "Buffers.h"

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <memory>



/**
 * @brief For logging incoming UDP datagrams to disk.
 * 
 * @note This is currently unimplemented. Would have been used in the GSE.
 */
class Logger {
    public:
        enum class log_modes{FSTREAM, SPDLOG};

        Logger(
            std::string file_prefix,
            log_modes new_log_mode,
            std::vector<std::shared_ptr<SystemManager>> new_system_managers,
            boost::asio::ip::udp::endpoint new_local_endpoint,
            boost::asio::ip::udp::endpoint new_remote_endpoint,
            boost::asio::io_context& context
        );
        
        void async_receive();

        bool try_log(std::vector<uint8_t> packet);
        void log(std::vector<uint8_t> packet);

        SystemManager* find_system_manager(uint8_t hex);
        int find_system_manager_position(uint8_t hex);

        boost::asio::ip::udp::endpoint local_endpoint;
        boost::asio::ip::udp::endpoint remote_endpoint;
        boost::asio::ip::udp::socket local_socket;

        size_t header_size;
        size_t max_packet_size;
    
    private:
        std::vector<std::shared_ptr<SystemManager>> system_managers;
        
        std::unordered_map<uint8_t, std::unordered_map<uint8_t, std::shared_ptr<std::vector<uint8_t>>>> packet_buffers;
        /**
         * @brief Lookup output file streams by hex ID of sender and hex ID of datatype.
         * 
         */
        std::unordered_map<uint8_t, std::unordered_map<uint8_t, std::shared_ptr<std::ofstream>>> file_streams;

        std::unordered_map<uint8_t, std::unordered_map<uint8_t, std::string>> file_names;

        /**
         * @brief To switch between using spdlog or fstream logging, or other modes.
         * 
         */
        log_modes log_mode;

        void log_raw(std::vector<uint8_t> packet);
        void spdlog_raw(std::vector<uint8_t> packet);

        /**
         * @brief Position of system hex code in packet header.
         * 
         */
        const size_t system_index = 0;
        const size_t system_index_size = 1;

        const size_t packet_count_index = 1;
        const size_t packet_count_index_size = 2;

        const size_t packet_index_index = 3;
        const size_t packet_index_index_size = 2;
        /**
         * @brief Position of datatype hex code in packet header.
         * See Parameters.h/RING_BUFFER_TYPE_OPTIONS.
         */
        const size_t type_index = 5;
        const size_t type_index_size = 1;

};

#endif