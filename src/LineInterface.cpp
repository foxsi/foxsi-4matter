#include "LineInterface.h"
#include "Parameters.h"

#include <iostream>

LineInterface::LineInterface(int argc, char* argv[]): options("options") {
    options.add_options()
        ("help,h",                                                              "output help message")
        ("version",                                                             "output software version number")
        ("verbose,v",                                                           "output verbosely")
        ("file,f",          boost::program_options::value<std::string>(),       "config file with options")
        ("local.ip,I",      boost::program_options::value<std::string>(),       "local IP address")
        ("gse.ip",          boost::program_options::value<std::string>(),       "remote GSE IP address")
        ("evtm.ip",         boost::program_options::value<std::string>(),       "remote EVTM IP address")
        ("spmu.ip",         boost::program_options::value<std::string>(),       "remote SPMU IP address")
        ("gse.port",        boost::program_options::value<unsigned short>(),    "port number GSE uses to talk to local")
        ("evtm.port",       boost::program_options::value<unsigned short>(),    "port number EVTM uses to talk to local")
        ("spmu.port",       boost::program_options::value<unsigned short>(),    "port number SPMU uses to talk to local")
        ("local.gseport",   boost::program_options::value<unsigned short>(),    "port number local uses to talk to GSE")
        ("local.evtmport",  boost::program_options::value<unsigned short>(),    "port number local uses to talk to EVTM")
        ("local.spmuport",  boost::program_options::value<unsigned short>(),    "port number local uses to talk to SPMU")
        ("gse.protocol",    boost::program_options::value<std::string>(),       "protocol (TCP or UDP) used between local and GSE")
        ("evtm.protocol",   boost::program_options::value<std::string>(),       "protocol (TCP or UDP) used between local and EVTM")
        ("spmu.protocol",   boost::program_options::value<std::string>(),       "protocol (TCP or UDP) used between local and SPMU")
        ("period,T",        boost::program_options::value<unsigned long>(),     "main loop period in milliseconds")
    ;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).run(), vm);
    boost::program_options::notify(vm);

    version = std::to_string(MAJOR_VERSION) + "." + std::to_string(MINOR_VERSION) + "." + std::to_string(PATCH_VERSION);
    // long help message (move this elsewhere?)
    help_msg = R"(usage: etherlogger [options]
    
    Log incoming UDP messages to file.
    
    General options:
        --help,         -h                  Display help message.
        --version,                          Check software version.
    )";

    // handle all the options:
    if(vm.count("help")) {
        std::cout << help_msg << "\n";
        exit(0);
    }
    if(vm.count("version")) {
        std::cout << version << "\n";
        exit(0);
    }
    if(vm.count("file")) {
        std::string config_filename = vm["file"].as<std::string>();
        std::cout << "reading options from " << config_filename << "\n";
        boost::program_options::store(boost::program_options::parse_config_file(config_filename.c_str(), options), vm);
    }
    boost::program_options::notify(vm);
}
