#include "LineInterface.h"
#include "Parameters.h"

#include "Utilities.h"
#include <iostream>

EndpointData::EndpointData(std::string ip, std::string prot, unsigned short pt) {
    address = ip;
    protocol = prot;
    port = pt;
}
EndpointData::EndpointData() {
    address = "";
    protocol = "";
    port = 0;
}

TimeData::TimeData(double period_s) {
    if(period_s > 0) {
        period_seconds = period_s;
    } else {
        throw "period must be positive\n";
    } 
}
TimeData::TimeData() {
    period_seconds = 0.0;
}

LineInterface::LineInterface(int argc, char* argv[], boost::asio::io_context& context): options("options") {
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
        ("period,T",        boost::program_options::value<double>(),     "main loop period in seconds")
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
        std::cout << "version: " << version << "\n";
    }
    if(vm.count("verbose")) {
        do_verbose = true;
        verbose_print("verbose printing on");
    }
    if(vm.count("file")) {
        std::string config_filename = vm["file"].as<std::string>();
        verbose_print("reading file from " + config_filename);
        boost::program_options::store(boost::program_options::parse_config_file(config_filename.c_str(), options), vm);
    }
    boost::program_options::notify(vm);

    // initialize missings map:
    missings = {
        {"gse", false},
        {"evtm", false},
        {"spmu", false},
    };

    EndpointData gse;
    EndpointData evtm;
    EndpointData spmu;

    // define gse:
    if(vm.count("gse.ip") && vm.count("gse.port") && vm.count("gse.protocol")) {
        verbose_print("gse: connect over " + vm["gse.protocol"].as<std::string>()+ " to " + vm["gse.ip"].as<std::string>() + ":" + std::to_string(vm["gse.port"].as<unsigned short>()));

        EndpointData thisendpoint(
            vm["gse.ip"].as<std::string>(),
            vm["gse.protocol"].as<std::string>(),
            vm["gse.port"].as<unsigned short>()
        );
        gse = thisendpoint;
        endpoints.push_back(thisendpoint);
    } else {
        missings["gse"] = true;
        verbose_print("didn't find gse in config");
    }
    // define evtm:
    if(vm.count("evtm.ip") && vm.count("evtm.port") && vm.count("evtm.protocol")) {
        verbose_print("evtm: connect over " + vm["evtm.protocol"].as<std::string>()+ " to " + vm["evtm.ip"].as<std::string>() + ":" + std::to_string(vm["evtm.port"].as<unsigned short>()));

        EndpointData thisendpoint(
            vm["evtm.ip"].as<std::string>(),
            vm["evtm.protocol"].as<std::string>(),
            vm["evtm.port"].as<unsigned short>()
        );
        evtm = thisendpoint;
        endpoints.push_back(thisendpoint);
    } else {
        missings["evtm"] = true;
        verbose_print("didn't find evtm in config");
    }
    // define spmu:
    if(vm.count("spmu.ip") && vm.count("spmu.port") && vm.count("spmu.protocol")) {
        verbose_print("spmu: connect over " + vm["spmu.protocol"].as<std::string>()+ " to " + vm["spmu.ip"].as<std::string>() + ":" + std::to_string(vm["spmu.port"].as<unsigned short>()));

        EndpointData thisendpoint(
            vm["spmu.ip"].as<std::string>(),
            vm["spmu.protocol"].as<std::string>(),
            vm["spmu.port"].as<unsigned short>()
        );
        spmu = thisendpoint;
        endpoints.push_back(thisendpoint);
    } else {
        missings["spmu"] = true;
        verbose_print("didn't find spmu in config");
    }

    if(vm.count("local.ip")) {
        verbose_print("local ip: " + vm["local.ip"].as<std::string>());
    } else {
        verbose_print("didn't find local ip");
    }

    TimeData times;

    // get timing info
    if(vm.count("period")) {
        verbose_print("loop period: " + std::to_string(vm["period"].as<double>()));
        TimeData temp(vm["period"].as<double>());
        times = temp;
    }
}

void LineInterface::verbose_print(std::string msg) {
    if(do_verbose) {
        std::cout << msg << "\n";
    }
}
