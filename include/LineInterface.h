#ifndef LINEINTERFACE_H
#define LINEINTERFACE_H

#include "Commanding.h"
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <string>
#include <map>


// EndpointData, TimeData really just exist to be consumed by/instantiate a TransportLayerMachine.
class EndpointData {
    public:
        std::string address;
        std::string protocol;
        unsigned short port;
        EndpointData(std::string ip, std::string prot, unsigned short pt);
        EndpointData();
        bool operator==(EndpointData& other);
        std::string as_string();
};

class TimeData {
    public:
        double period_seconds;
        double command_seconds;
        double request_seconds;
        double reply_seconds;
        double idle_seconds;

        TimeData(double period_s);
        TimeData();

        void add_times(double total_allocation, double command_time, double request_time, double reply_time, double idle_time);
        void resolve_times();
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