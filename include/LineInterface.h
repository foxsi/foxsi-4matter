#ifndef LINEINTERFACE_H
#define LINEINTERFACE_H

#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <string>
#include <map>

class EndpointData {
    public:
        std::string address;
        std::string protocol;
        unsigned short port;
        EndpointData(std::string ip, std::string prot, unsigned short pt);
        EndpointData();
};

class TimeData {
    public:
        double period_seconds;
        TimeData(double period_s);
        TimeData();
};

class LineInterface {
    private:
        boost::program_options::options_description options;
        boost::program_options::variables_map vm;

        std::string help_msg;
        bool do_verbose;

    public:
        std::string version;
        std::map<std::string, bool> missings;
        std::map<std::string, EndpointData> endpoints;
        std::string local_address;
        TimeData times;

    public:
        LineInterface(int argc, char* argv[], boost::asio::io_context& context);

    private:
        void verbose_print(std::string msg);
};

#endif