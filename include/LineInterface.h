#ifndef LINEINTERFACE_H
#define LINEINTERFACE_H

#include "Commanding.h"
#include "Systems.h"
#include "Buffers.h"
#include "Timing.h"
#include <boost/program_options.hpp>
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

class LineInterface {
    public:
        std::string version;

        std::vector<System> systems;

        std::unordered_map<System, Timing> lookup_timing;

        // todo: put all these in `SystemManager`. Easy
        std::unordered_map<System, std::string> lookup_command_file;
        std::unordered_map<System, std::queue<UplinkBufferElement>> lookup_uplink_buffer;
        std::unordered_map<System, PacketFramer> lookup_packet_framers;
        std::unordered_map<System, FramePacketizer> lookup_frame_packetizers;

        std::vector<Ethernet*> unique_endpoints;
        std::vector<Ethernet*> local_endpoints;
        
        std::string local_address;

        bool do_uart;

    public:
        LineInterface(int argc, char* argv[], boost::asio::io_context& context);

        CommandDeck get_command_deck() const {return command_deck;};

        std::string get_test_system_name() const {return test_system_name;};
    
    private:
        boost::program_options::options_description options;
        boost::program_options::variables_map vm;
        CommandDeck command_deck;

        std::string help_msg;
        bool do_verbose;

        std::string test_system_name;

    private:
        void collapse_endpoints();
        void build_local_endpoints();
        void verbose_print(std::string msg);
};

#endif