#ifndef LINEINTERFACE_H
#define LINEINTERFACE_H

#include "Commanding.h"
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <string>
#include <map>

/**
 * @brief A convenience datastructure for passing Ethernet protocol, port number, and IP address information.
 * 
 * This class is used to extract TCP or UDP endpoint information from the JSON configuration file and pass it into the software. 
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
class TimeData {
    public:
        /**
         * @brief Construct a new empty `TimeData` object.
         * 
         * The object's fields can then be populated with `TimeData::add_times_seconds(...).
         */
        TimeData();
        
        /**
         * @brief Populate fields of `TimeData`.
         * 
         * @param total_allocation the total amount of time (in seconds) that comprises `TimeData::command_millis`, `TimeData::request_millis`, `TimeData::reply_millis`, and `TimeData::idle_millis`. 
         * 
         * @param command_time the amount of time (in seconds) spent sending commands.
         * @param request_time the amount of time (in seconds) spent requesting data.
         * @param reply_time the amount of time (in seconds) spent receiving response data/forwarding.
         * @param idle_time the amount of idle time (in seconds) at the end.
         */
        void add_times_seconds(double total_allocation, double command_time, double request_time, double reply_time, double idle_time);

        /**
         * @brief Clean up fields of `TimeData` so that `TimeData::command_millis`, `TimeData::request_millis`, `TimeData::reply_millis`, and `TimeData::idle_millis` sum to `TimeData::period_millis`.
         * 
         */
        void resolve_times();

        unsigned int period_millis;
        unsigned int command_millis;
        unsigned int request_millis;
        unsigned int reply_millis;
        unsigned int idle_millis;
};

class LineInterface {
    public:
        std::string version;
        std::unordered_map<std::string, bool> missings;
        std::unordered_map<std::string, EndpointData> endpoints;
        std::unordered_map<uint8_t, EndpointData*> lookup_endpoints;
        std::unordered_map<uint8_t, TimeData*> lookup_times;
        std::unordered_map<uint8_t, std::string> lookup_command_file;
        
        std::string local_address;

        std::vector<System> systems;
        std::vector<EndpointData> unique_endpoints;
        std::vector<EndpointData> local_endpoints;

        TimeData times;

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