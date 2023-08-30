#include "LineInterface.h"
#include "Parameters.h"

#include "Utilities.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

// this constructor uses initializer lists.
EndpointData::EndpointData(std::string ip, std::string prot, unsigned short pt): address(ip), protocol(prot), port(pt) {}

EndpointData::EndpointData() {
    // build an EndpointData object with empty fields
    address = "";
    protocol = "";
    port = 0;
}

bool EndpointData::operator==(EndpointData& other) {
    // check that all the fields are equal
    if(address.compare(other.address) == 0 && protocol.compare(other.protocol) == 0 && port == other.port) {
        return true;
    } else {
        return false;
    }
}

std::string EndpointData::as_string() {
    return "("+protocol+")"+address+":"+std::to_string(port);
}

// TimeData::TimeData(double period_s) {
//     if(period_s > 0) {
//         period_millis = period_s;
//     } else {
//         throw "period must be positive\n";
//     } 
// }

TimeData::TimeData() {
    period_millis = 0.0;
    command_millis = 0.0;
    request_millis = 0.0;
    reply_millis = 0.0;
    idle_millis = 0.0;
}

void TimeData::add_times_seconds(double total_allocation_seconds, double command_time_seconds, double request_time_seconds, double reply_time_seconds, double idle_time_seconds) {
    period_millis = (unsigned int)(total_allocation_seconds*1000);
    command_millis = (unsigned int)(command_time_seconds*1000);
    request_millis = (unsigned int)(request_time_seconds*1000);
    reply_millis = (unsigned int)(reply_time_seconds*1000);
    idle_millis = (unsigned int)(idle_time_seconds*1000);

    TimeData::resolve_times();
}

void TimeData::resolve_times() {
    double sum = command_millis + request_millis + reply_millis + idle_millis;
    command_millis = command_millis*period_millis/sum;
    request_millis = request_millis*period_millis/sum;
    reply_millis = reply_millis*period_millis/sum;
    idle_millis = idle_millis*period_millis/sum;
}

LineInterface::LineInterface(int argc, char* argv[], boost::asio::io_context& context): options("options") {
    options.add_options()
        ("help,h",                                                              "output help message")
        ("version",                                                             "output software version number")
        ("verbose,v",                                                           "output verbosely")
        ("config,c",          boost::program_options::value<std::string>(),       "config file with options")
    ;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).run(), vm);
    boost::program_options::notify(vm);

    version = std::to_string(MAJOR_VERSION) + "." + std::to_string(MINOR_VERSION) + "." + std::to_string(PATCH_VERSION);
    // long help message (move this elsewhere? TODO:write it)
    help_msg = R"(usage: ??? [options]
    
    Launch Formatter with set global options.
    
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
    
    // =============== NEW ============================================


    // convenience maps for lookups:
    std::unordered_map<uint8_t, System> lookup_system;
    // std::unordered_map<uint8_t, EndpointData*> lookup_endpoint_data;
    // std::unordered_map<uint8_t, TimeData*> lookup_timing;
    // std::unordered_map<uint8_t, std::string> lookup_command_file;

    std::string config_filename;
    std::ifstream config_file;
    if(vm.count("config")) {
        config_filename = vm["config"].as<std::string>();
        verbose_print("reading file from " + config_filename);

        try {
            config_file.open(config_filename, std::ios::in);
        } catch(const std::exception& e) {
            std::cout << e.what();
            // TODO: some default behavior
        }
    }
    nlohmann::json system_configuration = nlohmann::json::parse(config_file);
    /* Things to pull out:
        - Systems: first-order in config_file
        - CommandDeck
        - Ethernet interfaces 
    */
    verbose_print("finding systems...");

    for(auto& this_system: system_configuration.items()) {
        std::string this_name = this_system.value()["name"];
        std::string this_hex_str = this_system.value()["hex"];
        // convert hex code string to uint8_t type. Note: this strtol method accepts both 0x- prefixed and raw hex strings.
        uint8_t this_hex = string_to_byte(this_hex_str);

        verbose_print("adding new System:");
        verbose_print("\tname:    " + this_name);
        verbose_print("\tid code: " + std::to_string(this_hex));

        System this_system_object = System(this_name, this_hex);

        this_system_object.set_type(COMMAND_TYPE_OPTIONS::NONE);

        bool has_ethernet = false;  // log if this system has ethernet interface
        bool has_spw = false;       // log if this system has spacewire interface
        bool has_spi = false;       // log if this system has spi interface
        bool has_uart = false;      // log if this system has uart interface
        bool is_local = false;      // log if this system defines the local Ethernet endpoint
        bool is_command = false;    // log if this system can be commanded (NOR local)
        std::string command_file_path;

        // check for Ethernet, UART, SPI interfaces:
        try {
            auto& ethif = this_system.value()["ethernet_interface"];
            if(!ethif.is_null()) {
                has_ethernet = true;
            }

            std::string this_addr;
            unsigned short this_port;
            std::string this_prot;

            // all ethernet_interface s should have an address
            this_addr = ethif.at("address").get<std::string>();
            try {
                // if the interface only has an address (no port/protocol), it is the local address
                this_prot = ethif.at("protocol").get<std::string>();
                this_port = ethif.at("port").get<unsigned short>();
                EndpointData* ept = new EndpointData(this_addr, this_prot, this_port);
                // store this endpoint data in the lookup table for systems
                lookup_endpoints.insert(std::make_pair(this_hex, ept));
                verbose_print("\tadded an endpoint: " + ept->as_string());
            } catch(std::exception& e) {
                is_local = true;
                // store the local address in the object
                local_address = this_addr;
            }
            
            // check if we can command it
            try {
                command_file_path = ethif.at("command_path").get<std::string>();
                is_command = true;

                // an Ethernet-commandable system
                lookup_command_file.insert(std::make_pair(this_hex, command_file_path));
                
                this_system_object.set_type(COMMAND_TYPE_OPTIONS::ETHERNET);
                
            } catch(std::exception& e) {}

            // check for spacewire
            try {
                auto& spwif = this_system.value()["ethernet_interface"]["spacewire_interface"];
                if(!spwif.is_null()) {
                    has_spw = true;
                    this_system_object.set_type(COMMAND_TYPE_OPTIONS::SPW);
                }

                // add the spacewire properties to the System
                this_system_object.target_path_address = spwif.at("target_path_address").get<std::vector<uint8_t>>();
                this_system_object.reply_path_address = spwif.at("reply_path_address").get<std::vector<uint8_t>>();
                this_system_object.target_logical_address = string_to_byte(spwif.at("target_logical_address").get<std::string>());
                this_system_object.source_logical_address = string_to_byte(spwif.at("source_logical_address").get<std::string>());
                this_system_object.key = string_to_byte(spwif.at("key").get<std::string>());
                this_system_object.crc_version = spwif.at("crc_draft").get<std::string>();
                this_system_object.hardware_name = spwif.at("hardware").get<std::string>();

                // add an entry for this System's commands file in the lookup table (to create Commands later)
                lookup_command_file.insert(std::make_pair(this_hex, spwif.at("command_path").get<std::string>()));
                is_command = true;

            } catch(std::exception& e) {
                // std::cout << "couldn't find a SpaceWire interface.\n";
            }

            

        } catch(std::exception& e) {
            // no ethif
        }

        // check for SPI interface:
        try {
            auto& spiif = this_system.value()["spi_interface"];

            // currently (june-8-2023) no SPI interfaces exist in Formatter
            if(!spiif.is_null()) {
                std::cout << "Warning: SPI interface NOT IMPLEMENTED" << "\n";
                has_spi = true;
                this_system_object.set_type(COMMAND_TYPE_OPTIONS::SPI);
            }

            // else, do stuff with SPI interface
            lookup_command_file.insert(std::make_pair(this_hex, spiif.at("command_path").get<std::string>()));
            is_command = true;

        } catch(std::exception& e) {}

        // check for UART interface:
        try {
            auto& uartif = this_system.value()["uart_interface"];
            if(!uartif.is_null()) {
                has_uart = true;
                this_system_object.set_type(COMMAND_TYPE_OPTIONS::UART);
            }
            this_system_object.baud_rate = uartif.at("baud_rate").get<unsigned long>();
            this_system_object.parity_bits = uartif.at("parity_bits").get<unsigned short>();
            this_system_object.data_bits = uartif.at("data_bits").get<unsigned short>();
            this_system_object.stop_bits = uartif.at("stop_bits").get<unsigned short>();

            lookup_command_file.insert(std::make_pair(this_hex, uartif.at("command_path").get<std::string>()));

        } catch(std::exception& e) {}

        // TODO:do sanity checks
        debug_print("\tfound command type: \n");
        if(this_system_object.type == COMMAND_TYPE_OPTIONS::UART) {
                debug_print("\t\tUART\n");
            } else if(this_system_object.type == COMMAND_TYPE_OPTIONS::SPW) {
                debug_print("\t\tSPW\n");
            } else if(this_system_object.type == COMMAND_TYPE_OPTIONS::SPI) {
                debug_print("\t\tSPI\n");
            } else if(this_system_object.type == COMMAND_TYPE_OPTIONS::ETHERNET) {
                debug_print("\t\tETHERNET\n");
            } else if(this_system_object.type == COMMAND_TYPE_OPTIONS::NONE) {
                debug_print("\t\tnone\n");
            } else {
                debug_print("\t\tDID NOT FIND TYPE\n");
        }

        // now add more command-specific info to this_system_object
        if(is_command) {
            try {
                auto& time_data = this_system.value()["time"];
                TimeData* this_times = new TimeData();
                this_times->add_times_seconds(
                    time_data.at("total_allocation").get<double>(),
                    time_data.at("command").get<double>(),
                    time_data.at("request").get<double>(),
                    time_data.at("reply").get<double>(),
                    time_data.at("idle").get<double>()
                );

                lookup_times.insert(std::make_pair(this_hex, this_times));

                // do stuff with the time info
            } catch(std::exception& e) {
                std::cout << "\texpect timing info to be provided for a commanded system. Discarding system named " << this_name << "\n";

                // TODO: ACTUALLY REMOVE IT? OR JUST ADD EVERYTHING DOWN BELOW
            }
        }

        systems.push_back(this_system_object);
        lookup_system.insert(std::make_pair(this_hex, this_system_object));

    }

    // lookup_endpoints = lookup_endpoint_data;
    collapse_endpoints();
    build_local_endpoints();

    // finish parsing
    config_file.close();

    // build CommandDeck
    command_deck = CommandDeck(systems,lookup_command_file);

    // =============== OLD ============================================

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
        endpoints.insert({"gse", thisendpoint});
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
        endpoints.insert({"evtm", thisendpoint});
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
        endpoints.insert({"spmu", thisendpoint});
    } else {
        missings["spmu"] = true;
        verbose_print("didn't find spmu in config");
    }

    if(vm.count("local.ip")) {
        verbose_print("local ip: " + vm["local.ip"].as<std::string>());
        local_address = vm["local.ip"].as<std::string>();
    } else {
        verbose_print("didn't find local ip");
    }

    TimeData times;

    // get timing info
    if(vm.count("period")) {
        verbose_print("loop period: " + std::to_string(vm["period"].as<double>()));
        // TimeData temp(vm["period"].as<double>());
        TimeData temp;
        times = temp;
    }

    // get system info (to use for commands)
    bool found_systems = false;
    std::unordered_map<std::string, bool> found_files;
    std::unordered_map<std::string, std::string> named_paths;

    if(vm.count("systems.codepath")) {
        verbose_print("systems path: " + vm["systems.codepath"].as<std::string>());
        found_systems = true;
        named_paths.insert(std::make_pair("SYSTEMS", vm["systems.codepath"].as<std::string>()));
    }
    // get command info
    found_files.insert(std::make_pair("cdte1.codepath", vm.count("cdte1.codepath")));
    found_files.insert(std::make_pair("cdte2.codepath", vm.count("cdte2.codepath")));
    found_files.insert(std::make_pair("cdte3.codepath", vm.count("cdte3.codepath")));
    found_files.insert(std::make_pair("cdte4.codepath", vm.count("cdte4.codepath")));
    found_files.insert(std::make_pair("cdtede.codepath", vm.count("cdtede.codepath")));
    found_files.insert(std::make_pair("cmos1.codepath", vm.count("cmos1.codepath")));
    found_files.insert(std::make_pair("cmos2.codepath", vm.count("cmos2.codepath")));
    found_files.insert(std::make_pair("timepix.codepath", vm.count("timepix.codepath")));
    found_files.insert(std::make_pair("housekeeping.codepath", vm.count("housekeeping.codepath")));

    for(auto& [field_name, exists]: found_files) {
        if(exists) {
            // pull the all-caps system name out of CLI string
            std::string reduced_name = field_name;
            std::transform(reduced_name.begin(), reduced_name.end(), reduced_name.begin(), ::toupper);
            size_t pos = reduced_name.find(".");
            reduced_name.erase(pos,reduced_name.size());

            // for each found system, try to make command deck with its file
            named_paths.insert(std::pair(reduced_name, vm[field_name].as<std::string>()));
        }
    }
    // command_deck = CommandDeck(named_paths);
}

void LineInterface::collapse_endpoints() {
    for(auto& ept_map: lookup_endpoints) {
        EndpointData* ept = ept_map.second;
        if(unique_endpoints.size() > 0) {
            bool is_uniq = true;
            for(auto& uniq: unique_endpoints) {
                if(*ept == uniq) {
                    is_uniq = false;
                }
            }
            if(is_uniq) {
                unique_endpoints.push_back(*ept);
            }
        } else {
            unique_endpoints.push_back(*ept);
        }
    }
}

void LineInterface::build_local_endpoints() {
    for(auto& ept: unique_endpoints) {
        EndpointData* local_ept = new EndpointData(local_address, ept.protocol, ept.port);
        local_endpoints.push_back(*local_ept);
    }
}

void LineInterface::verbose_print(std::string msg) {
    if(do_verbose) {
        std::cout << msg << "\n";
    }
}
