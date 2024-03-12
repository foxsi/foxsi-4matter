/**
 * @file LineInterface.h
 * @author Thanasi Pantazides
 * @brief Handle setup of this software via command line arguments.
 * @version v1.0.1
 * @date 2024-03-11
 */

#ifndef LINEINTERFACE_H
#define LINEINTERFACE_H

#include "Commanding.h"
#include "Systems.h"
#include "Buffers.h"
#include "Timing.h"
#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <string>
#include <queue>
#include <map>

/**
 * @deprecated Superseded by `Ethernet` (in Buffers.h).
 */
class EndpointData {
    public:

        /**
         * @brief Construct a new Endpoint Data object explicitly.
         * Populates `EndpointData::address`, `EndpointData::protocol`, `EndpointData::port` from the provided parameters.
         * @param ip the IP address of the endpoint, formatted "xxx.xxx.xxx.xxx".
         * @param prot the connection type for the endpoint. Consumers of this class within the Formatter software expect either "tcp" or "udp" for this value.
         * @param pt the port number for the endpoint. 
         */
        EndpointData(std::string ip, std::string prot, unsigned short pt);
        /**
         * @brief Construct a new empty `EndpointData` object with empty IP address, empty protocol type, and zero port.
         * 
         */
        EndpointData();
        /**
         * @brief Compares two `EndpointData` objects.
         * 
         * @param other the other `EndpointData` object to compare to.
         * @return true if all the fields are equal.
         * @return false if all the fields are not all equal.
         */
        bool operator==(EndpointData& other);
        /**
         * @brief Returns a string representation of the `EndpointData` in the format: `(protocol)xxx.xxx.xxx.xxx:yyyyy`.
         * 
         * @return std::string the string representation of the `EndpointData` object.
         */
        std::string as_string();

        /**
         * @brief The IP address of the endpoint.
         */
        std::string address;
        /**
         * @brief The connection type of the endpoint (e.g. "tcp" or "udp").
         * Any `std::string` can be used here, but consumers of this class in the Formatter software expect "tcp" or "udp" as value.
         */
        std::string protocol;
        /**
         * @brief The port number to connect on (0-65535, with 0-1023 typically reserved for system use).
         */
        unsigned short port;
};

/**
 * @brief `LineInterface` is used to setup most Formatter software from a few arguments passed at the command line. 
 * The constructor for this object expects a JSON file with specific structure to be passed in at the command line with the flag `--config`. See `foxsi4-commands` and the "Configuration data" page of documentation for information on the JSON structure.
 */
class LineInterface {
    public:
        /**
         * @brief Semantic version number of this software.
         */
        std::string version;

        /**
         * @brief A list of the `System`s generated from the JSON.
         */
        std::vector<System> systems;

        /**
         * @brief A lookup table for each `System`'s `Timing` data.
         * Consume this to build `SystemManager` objects.
         */
        std::unordered_map<System, Timing> lookup_timing;

        /**
         * @brief A lookup table of command JSON lists for each `System`.
         * This is used internally in the `LineInterface` to construct `LineInterface::command_deck` via `CommandDeck(std::vector<System> new_systems, std::unordered_map<System, std::string> command_paths)`.
         */
        std::unordered_map<System, std::string> lookup_command_file;
        /**
         * @deprecated Unused.
         * Would have been nice to use to construct these higher-level objects, so it doesn't need to be done in `main`.
         */
        std::unordered_map<System, std::queue<UplinkBufferElement>> lookup_uplink_buffer;
        /**
         * @deprecated Unused.
         * Would have been nice to use to construct these higher-level objects, so it doesn't need to be done in `main`.
         */
        std::unordered_map<System, PacketFramer> lookup_packet_framers;
        /**
         * @deprecated Unused.
         * Would have been nice to use to construct these higher-level objects, so it doesn't need to be done in `main`.
         */
        std::unordered_map<System, FramePacketizer> lookup_frame_packetizers;

        /**
         * @brief List of all unique `Ethernet` endpoints extracted from the JSON.
         */
        std::vector<Ethernet*> unique_endpoints;
        /**
         * @brief List all `Ethernet` endpoints in the JSON identified as on the local machine.
         * These should all have the same IP address but different port numbers.
         */
        std::vector<Ethernet*> local_endpoints;
        
        /**
         * @brief The `Ethernet` address from the JSON identified as the local machine.
         */
        std::string local_address;

        /**
         * @brief Flag to indicate if UART interface is desired.
         * @note No consumers of `LineInterface` in this software do anything with this flag.
         */
        bool do_uart;

    public:
        /**
         * @brief Constructor for `LineInterface` that ingests command line arguments.
         * Must pass in `--config path/to/config/file.json` in order to run. See foxsi4-commands documentation for description of JSON contents.
         * @param argc number of arguments passed.
         * @param argv  value of arguments passed.
         * @param context an `io_context` used to create sockets and drive the asynchronous operations in this software.
         */
        LineInterface(int argc, char* argv[], boost::asio::io_context& context);

        /**
         * @brief Get `CommandDeck` object that was constructed.
         * @return CommandDeck
         */
        CommandDeck get_command_deck() const {return command_deck;};

        /**
         * @brief Get the name of the `System` that was passed in on the command line for testing.
         * @return std::string the `System::name`.
         */
        std::string get_test_system_name() const {return test_system_name;};
    
    private:
        boost::program_options::options_description options;
        boost::program_options::variables_map vm;
        CommandDeck command_deck;

        std::string help_msg;
        bool do_verbose;

        std::string test_system_name;

    private:
        bool add_ring_buffers_to_system(System& system, nlohmann::json interface_map);
        void collapse_endpoints();
        void build_local_endpoints();
        void verbose_print(std::string msg);
};

#endif