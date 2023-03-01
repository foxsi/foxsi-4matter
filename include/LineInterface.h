#ifndef LINEINTERFACE_H
#define LINEINTERFACE_H

#include <boost/program_options.hpp>
#include <string>
#include <map>

class LineInterface {
    private:
        boost::program_options::options_description options;
        boost::program_options::variables_map vm;

        std::string help_msg;
        bool do_verbose;

    public:
        std::string version;

    public:
        LineInterface(int argc, char* argv[]);
};

#endif