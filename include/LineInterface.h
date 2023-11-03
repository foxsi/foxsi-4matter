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

// /**
//  * @brief A convenience datastructure for passing inner loop timing information.
//  * 
//  * The inner timing loop is run for each onboard system the Formatter communicates with. The inner loop has a different period for each onboard system. Inside the inner loop, 
//  * 1. commands are sent to the system,
//  * 2. a data request is sent to the system,
//  * 3. the system responds with buffered data to telemeter to the ground,
//  * 4. there is a grace period to end the exchange with the system.
//  * 
//  * This class stores these fields for the inner loop.
//  */
// class Timing {
//     public:
//         /**
//          * @brief Construct a new empty `Timing` object.
//          * 
//          * The object's fields can then be populated with `Timing::add_times_seconds(...).
//          */
//         Timing();
        
//         /**
//          * @brief Populate fields of `Timing`.
//          * 
//          * @param total_allocation the total amount of time (in seconds) that comprises `Timing::command_millis`, `Timing::request_millis`, `Timing::reply_millis`, and `Timing::idle_millis`. 
//          * 
//          * @param command_time the amount of time (in seconds) spent sending commands.
//          * @param request_time the amount of time (in seconds) spent requesting data.
//          * @param reply_time the amount of time (in seconds) spent receiving response data/forwarding.
//          * @param idle_time the amount of idle time (in seconds) at the end.
//          */
//         void add_times_seconds(double total_allocation, double command_time, double request_time, double reply_time, double idle_time);

//         /**
//          * @brief Clean up fields of `Timing` so that `Timing::command_millis`, `Timing::request_millis`, `Timing::reply_millis`, and `Timing::idle_millis` sum to `Timing::period_millis`.
//          * 
//          */
//         void resolve_times();

//         uint32_t period_millis;
//         uint32_t command_millis;
//         uint32_t request_millis;
//         uint32_t reply_millis;
//         uint32_t idle_millis;

//         uint32_t timeout_millis;
//         uint32_t intercommand_space_millis;
// };

class LineInterface {
    public:
        std::string version;

        /*
        std::unordered_map<std::string, bool> missings;                 // remove
        std::unordered_map<std::string, EndpointData> endpoints;        // remove
        std::unordered_map<uint8_t, EndpointData*> lookup_endpoints;    // remove.
        std::unordered_map<uint8_t, Timing*> lookup_times;              // retype key to `System`
        std::unordered_map<uint8_t, std::string> lookup_command_file;   // retype key to `System`
        
        std::string local_address;                                      // preserve

        std::vector<System> systems;                                    // preserve
        std::vector<EndpointData> unique_endpoints;                     // replace inner type
        std::vector<EndpointData> local_endpoints;                      // replace inner type
        */

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

    public:
        LineInterface(int argc, char* argv[], boost::asio::io_context& context);

        CommandDeck get_command_deck() const {return command_deck;};
    
    private:
        boost::program_options::options_description options;
        boost::program_options::variables_map vm;
        CommandDeck command_deck;

        std::string help_msg;
        bool do_verbose;

    private:
        void collapse_endpoints();
        void build_local_endpoints();
        void verbose_print(std::string msg);
};

#endif