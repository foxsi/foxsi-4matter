#ifndef LINEINTERFACE_H
#define LINEINTERFACE_H

#include <boost/program_options.hpp>
#include <string>

class LineInterface {
    public:
        boost::program_options::options_description desc;
        boost::program_options::variables_map vm;
    public:
        LineInterface(int argc, char* argv[]);
};

#endif