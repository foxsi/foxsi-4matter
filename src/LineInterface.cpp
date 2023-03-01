#include "LineInterface.h"

LineInterface::LineInterface(int argc, char* argv[]): options("options") {
    options.add_options()
        ("help,h",          "output help message")
        ("file,f",          boost::program_options::value<std::string>(),         "config file with options")
        ("local-ip,I",      boost::program_options::value<std::string>(),         "local IP address")
    ;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).run(), vm);
    boost::program_options::notify(vm);
}
