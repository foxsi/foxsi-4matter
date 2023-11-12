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

// Timing::Timing(double period_s) {
//     if(period_s > 0) {
//         period_millis = period_s;
//     } else {
//         throw "period must be positive\n";
//     } 
// }

// Timing::Timing() {
//     period_millis = 0.0;
//     command_millis = 0.0;
//     request_millis = 0.0;
//     reply_millis = 0.0;
//     idle_millis = 0.0;
// }

// void Timing::add_times_seconds(double total_allocation_seconds, double command_time_seconds, double request_time_seconds, double reply_time_seconds, double idle_time_seconds) {
//     period_millis = (uint32_t)(total_allocation_seconds*1000);
//     command_millis = (uint32_t)(command_time_seconds*1000);
//     request_millis = (uint32_t)(request_time_seconds*1000);
//     reply_millis = (uint32_t)(reply_time_seconds*1000);
//     idle_millis = (uint32_t)(idle_time_seconds*1000);

//     Timing::resolve_times();
// }

// void Timing::resolve_times() {
//     double sum = command_millis + request_millis + reply_millis + idle_millis;
//     command_millis = command_millis*period_millis/sum;
//     request_millis = request_millis*period_millis/sum;
//     reply_millis = reply_millis*period_millis/sum;
//     idle_millis = idle_millis*period_millis/sum;
// }

LineInterface::LineInterface(int argc, char* argv[], boost::asio::io_context& context): options("options") {
    options.add_options()
        ("help,h",                                                              "output help message")
        ("version",                                                             "output software version number")
        ("verbose,v",                                                           "output verbosely")
        ("config,c",          boost::program_options::value<std::string>(),       "config file with options")
        ("system",            boost::program_options::value<std::string>(),       "name of system to test")
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
        exit(0);
    }
    if(vm.count("verbose")) {
        do_verbose = true;
        DEBUG = true;
        verbose_print("verbose printing on");
    }
    if(vm.count("system")) {
        test_system_name = vm["system"].as<std::string>();
        std::cout << "testing system " << test_system_name << "\n";
    }
    
    // =============== NEW ============================================


    // convenience maps for lookups:
    std::unordered_map<uint8_t, System> lookup_system;

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

    verbose_print("finding systems...");

    for(auto& this_system: system_configuration.items()) {
        std::string this_name = this_system.value()["name"];
        std::string this_hex_str = this_system.value()["hex"];
        // convert hex code string to uint8_t type. Note: this strtol method accepts both 0x- prefixed and raw hex strings.
        uint8_t this_hex = utilities::string_to_byte(this_hex_str);

        verbose_print("adding new System:");
        verbose_print("\tname:    " + this_name);
        verbose_print("\tid code: " + std::to_string(this_hex));

        System this_system_object = System(this_name, this_hex);

        this_system_object.type = COMMAND_TYPE_OPTIONS::NONE;

        bool has_ethernet = false;  // log if this system has ethernet interface
        bool has_spw = false;       // log if this system has spacewire interface
        bool has_spi = false;       // log if this system has spi interface
        bool has_uart = false;      // log if this system has uart interface
        bool is_local = false;      // log if this system defines the local Ethernet endpoint
        bool is_command = false;    // log if this system can be commanded (NOR local)
        std::string command_file_path;

        // check if the system is commanded, and if so, the command type:
        try {
            auto& cmd = this_system.value()["command_type"];
            if(!cmd.is_null()) {
                is_command = true;

                // auto& this_cmd_type = this_system.at("command_type").get<std::string>();
                if (cmd == "ethernet") {
                    this_system_object.type = COMMAND_TYPE_OPTIONS::ETHERNET;
                } else if (cmd == "spacewire") {
                    this_system_object.type = COMMAND_TYPE_OPTIONS::SPW;
                } else if (cmd == "uart") {
                    this_system_object.type = COMMAND_TYPE_OPTIONS::UART;
                } else {
                    utilities::error_print("failed to find System command type!\n");
                }

                std::string this_cmd_file = this_system.value()["commands"];
                lookup_command_file.insert(std::make_pair(this_system_object, this_cmd_file));
            
            } else {
                this_system_object.type = COMMAND_TYPE_OPTIONS::NONE;
            }
        } catch(std::exception& e) {}

        // check for Ethernet, UART, SPI interfaces:
        try {
            auto& ethif = this_system.value()["ethernet_interface"];
            if(!ethif.is_null()) {
                has_ethernet = true;
            }

            std::string this_addr;
            unsigned short this_port;
            unsigned short this_max_payload;
            std::string this_prot;

            // check this_system_object.type (to try to access command-related ethernet fields)

            // all ethernet_interfaces should have an address
            this_addr = ethif.at("address").get<std::string>();
            if (this_system_object.type == COMMAND_TYPE_OPTIONS::ETHERNET) {
                try {
                    Ethernet* eth = new Ethernet(
                        this_addr,
                        ethif.at("port").get<unsigned short>(),
                        ethif.at("protocol").get<std::string>(),
                        ethif.at("mean_speed_bps").get<uint32_t>(),
                        ethif.at("max_payload_bytes").get<size_t>(),
                        ethif.at("frame_size_bytes").get<size_t>(),
                        ethif.at("static_header_size").get<size_t>(),
                        ethif.at("static_footer_size").get<size_t>()
                    );

                    this_system_object.ethernet = eth;
                    verbose_print("\tadded an endpoint: " + eth->to_string());

                } catch(std::exception& e) {
                    
                }
            } else {
                try {
                    Ethernet* eth =  new Ethernet(
                        this_addr,
                        ethif.at("port").get<unsigned short>(),
                        ethif.at("protocol").get<std::string>(),
                        0,
                        ethif.at("max_payload_bytes").get<size_t>(),
                        0,
                        0,
                        0
                    );

                    this_system_object.ethernet = eth;
                    verbose_print("\tadded an endpoint: " + eth->to_string());
                } catch(std::exception& e) {
                    is_local = true;        // assume local address if only address provided
                    // store the local address in the object
                    local_address = this_addr;
                    verbose_print("local endpoint: " + local_address);
                }
            }
        } catch(std::exception& e) {

        }

        // check for spacewire
        try {
            auto& spwif = this_system.value()["spacewire_interface"];
            if(!spwif.is_null()) {
                has_spw = true;
                bool has_rbf = false;

                if (this_system_object.type == COMMAND_TYPE_OPTIONS::SPW) {
                    SpaceWire* spw = new SpaceWire(
                        spwif.at("target_path_address").get<std::vector<uint8_t>>(),
                        spwif.at("reply_path_address").get<std::vector<uint8_t>>(),
                        utilities::string_to_byte(spwif.at("target_logical_address").get<std::string>()),
                        utilities::string_to_byte(spwif.at("source_logical_address").get<std::string>()),
                        utilities::string_to_byte(spwif.at("key").get<std::string>()),
                        // utilities::string_to_byte(spwif.at("crc_draft").get<std::string>()),
                        static_cast<char>((spwif.at("crc_draft").get<std::string>())[0]),
                        spwif.at("mean_speed_bps").get<uint32_t>(),
                        spwif.at("max_payload_bytes").get<size_t>(),
                        0, // todo: replace this with read into ring_buffer_interface dict
                        spwif.at("static_header_size").get<size_t>(),
                        spwif.at("static_footer_size").get<size_t>()
                    );

                    this_system_object.spacewire = spw;

                    // try to get ring_buffer_interface.
                    try{
                        auto& rbif = spwif.at("ring_buffer_interface");
                        try{
                            auto& this_dma = rbif.at("pc");

                            std::string this_frame_size_bytes_str = this_dma.at("ring_frame_size_bytes").get<std::string>();
                            std::string this_start_address_str = this_dma.at("ring_start_address").get<std::string>();
                            std::string this_frames_per_ring_str = this_dma.at("frames_per_ring").get<std::string>();
                            std::string this_write_pointer_address_str = this_dma.at("ring_write_pointer_address").get<std::string>();
                            std::string this_write_pointer_width_bytes_str = this_dma.at("ring_write_pointer_width").get<std::string>();

                            size_t this_frame_size_bytes = std::strtoull(this_frame_size_bytes_str.c_str(), NULL, 16);
                            uint32_t this_start_address = std::strtoul(this_start_address_str.c_str(), NULL, 16);
                            size_t this_frames_per_ring = std::strtoull(this_frames_per_ring_str.c_str(), NULL, 16);
                            uint32_t this_write_pointer_address = std::strtoul(this_write_pointer_address_str.c_str(), NULL, 16);
                            size_t this_write_pointer_width_bytes = std::strtoull(this_write_pointer_width_bytes_str.c_str(), NULL, 16);

                            RingBufferParameters* this_rbp = new RingBufferParameters(
                                this_frame_size_bytes,
                                this_start_address,
                                this_frames_per_ring,
                                this_write_pointer_address,
                                this_write_pointer_width_bytes,
                                RING_BUFFER_TYPE_OPTIONS::PC
                            );

                            this_system_object.ring_params.emplace(std::make_pair(this_rbp->type, *this_rbp));

                        } catch(std::exception& e) {}

                        try{
                            auto& this_dma = rbif.at("ql");

                            std::string this_frame_size_bytes_str = this_dma.at("ring_frame_size_bytes").get<std::string>();
                            std::string this_start_address_str = this_dma.at("ring_start_address").get<std::string>();
                            std::string this_frames_per_ring_str = this_dma.at("frames_per_ring").get<std::string>();
                            std::string this_write_pointer_address_str = this_dma.at("ring_write_pointer_address").get<std::string>();
                            std::string this_write_pointer_width_bytes_str = this_dma.at("ring_write_pointer_width").get<std::string>();

                            size_t this_frame_size_bytes = std::strtoull(this_frame_size_bytes_str.c_str(), NULL, 16);
                            uint32_t this_start_address = std::strtoul(this_start_address_str.c_str(), NULL, 16);
                            size_t this_frames_per_ring = std::strtoull(this_frames_per_ring_str.c_str(), NULL, 16);
                            uint32_t this_write_pointer_address = std::strtoul(this_write_pointer_address_str.c_str(), NULL, 16);
                            size_t this_write_pointer_width_bytes = std::strtoull(this_write_pointer_width_bytes_str.c_str(), NULL, 16);

                            RingBufferParameters* this_rbp = new RingBufferParameters(
                                this_frame_size_bytes,
                                this_start_address,
                                this_frames_per_ring,
                                this_write_pointer_address,
                                this_write_pointer_width_bytes,
                                RING_BUFFER_TYPE_OPTIONS::QL
                            );

                            this_system_object.ring_params.emplace(std::make_pair(this_rbp->type, *this_rbp));


                        } catch(std::exception& e) {}
                        
                    } catch (std::exception& e) {}
                    
                } else {
                    SpaceWire* spw = new SpaceWire(
                        spwif.at("target_path_address").get<std::vector<uint8_t>>(),
                        spwif.at("reply_path_address").get<std::vector<uint8_t>>(),
                        utilities::string_to_byte(spwif.at("target_logical_address").get<std::string>()),
                        utilities::string_to_byte(spwif.at("source_logical_address").get<std::string>()),
                        utilities::string_to_byte(spwif.at("key").get<std::string>()),
                        spwif.at("crc_draft").get<char>(),
                        0,
                        spwif.at("max_payload_bytes").get<size_t>(),
                        0, // todo: replace this with read into ring_buffer_interface dict
                        0,
                        0
                    );

                    this_system_object.spacewire = spw;
                }


                // System command file should be identified and mapped a ways up ^^^
                // lookup_command_file.insert(std::make_pair(this_system_object, spwif.at("commands").get<std::string>()));

            }

        } catch(std::exception& e) {
            utilities::debug_print("exception while adding spacewire interface\n");
            // std::cout << "couldn't find a SpaceWire interface.\n";
        }

        // check for SPI interface:
        try {
            auto& spiif = this_system.value()["spi_interface"];

            // currently (june-8-2023) no SPI interfaces exist in Formatter
            if(!spiif.is_null()) {
                std::cout << "Warning: SPI interface NOT IMPLEMENTED" << "\n";
                has_spi = true;
            }

            // else, do stuff with SPI interface
            lookup_command_file.insert(std::make_pair(this_system_object, spiif.at("commands").get<std::string>()));

        } catch(std::exception& e) {}

        // check for UART interface:
        try {
            auto& uartif = this_system.value()["uart_interface"];
            if(!uartif.is_null()) {
                has_uart = true;

                if (this_system_object.type == COMMAND_TYPE_OPTIONS::UART) {
                    UART* uart = new UART(
                        uartif.at("baud_rate").get<uint32_t>(),
                        uartif.at("parity_bits").get<uint8_t>(),
                        uartif.at("stop_bits").get<uint8_t>(),
                        uartif.at("data_bits").get<uint8_t>(),
                        uartif.at("mean_speed_bps").get<uint32_t>(),
                        uartif.at("max_payload_bytes").get<size_t>(),
                        uartif.at("frame_size").get<size_t>(),
                        uartif.at("static_header_size").get<size_t>(),
                        uartif.at("static_footer_size").get<size_t>()
                    );

                    this_system_object.uart = uart;
                } else {
                    UART* uart = new UART(
                        uartif.at("baud_rate").get<uint32_t>(),
                        uartif.at("parity_bits").get<uint8_t>(),
                        uartif.at("stop_bits").get<uint8_t>(),
                        uartif.at("data_bits").get<uint8_t>(),
                        0,
                        uartif.at("max_payload_bytes").get<size_t>(),
                        0,
                        0,
                        0
                    );
                }
            }

        } catch(std::exception& e) {}
            
        // check if we can command it
        // try {
        //     command_file_path = ethif.at("command_path").get<std::string>();
        //     is_command = true;

        //     // an Ethernet-commandable system
        //     lookup_command_file.insert(std::make_pair(this_system_object, command_file_path));
            
        //     this_system_object.type = COMMAND_TYPE_OPTIONS::ETHERNET;
            
        // } catch(std::exception& e) {}

        

        // TODO:do sanity checks
        // debug_print("\tfound command type: \n");
        // if(this_system_object.type == COMMAND_TYPE_OPTIONS::UART) {
        //         debug_print("\t\tUART\n");
        //     } else if(this_system_object.type == COMMAND_TYPE_OPTIONS::SPW) {
        //         debug_print("\t\tSPW\n");
        //     } else if(this_system_object.type == COMMAND_TYPE_OPTIONS::SPI) {
        //         debug_print("\t\tSPI\n");
        //     } else if(this_system_object.type == COMMAND_TYPE_OPTIONS::ETHERNET) {
        //         debug_print("\t\tETHERNET\n");
        //     } else if(this_system_object.type == COMMAND_TYPE_OPTIONS::NONE) {
        //         debug_print("\t\tnone\n");
        //     } else {
        //         debug_print("\t\tDID NOT FIND TYPE\n");
        // }

        // now add more command-specific info to this_system_object
        if(is_command) {
            try {
                auto& time_data = this_system.value()["timing"];
                Timing* this_times = new Timing();
                this_times->add_times_seconds(
                    time_data.at("total_allocation").get<double>()/1000.0,
                    time_data.at("command").get<double>()/1000.0,
                    time_data.at("request").get<double>()/1000.0,
                    time_data.at("reply").get<double>()/1000.0,
                    time_data.at("idle").get<double>()/1000.0
                );

                this_times->timeout_millis = time_data.at("receive_timeout_millis").get<double>();
                this_times->intercommand_space_millis = time_data.at("intercommand_spacing_millis").get<double>();

                lookup_timing.insert(std::make_pair(this_system_object, *this_times));

                // do stuff with the time info
            } catch(std::exception& e) {
                std::cout << "\tno timing info provided for commanded system " << this_name << "\n";

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
    command_deck = CommandDeck(systems, lookup_command_file);
}

void LineInterface::collapse_endpoints() {
    for (auto& sys: systems) {
        if (sys.ethernet) { // not all systems have an ethernet interface
            if (unique_endpoints.size() > 0) {
                bool is_uniq = true;
                for(auto& uniq: unique_endpoints) {
                    if(sys.ethernet->is_same_endpoint(*uniq)) {
                        is_uniq = false;
                    }
                }
                if(is_uniq) {
                    unique_endpoints.push_back(sys.ethernet);
                }
            } else {
                unique_endpoints.push_back(sys.ethernet);
            }
        }
    }
}

void LineInterface::build_local_endpoints() {
    for(auto& ept: unique_endpoints) {
        Ethernet* local_eth = new Ethernet(*ept);
        local_eth->address = local_address;
        local_endpoints.push_back(local_eth);
    }
}

void LineInterface::verbose_print(std::string msg) {
    if(do_verbose) {
        std::cout << msg << "\n";
    }
}
