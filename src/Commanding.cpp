#include "Commanding.h"
#include "Utilities.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <stdlib.h>


Command::Command(std::string new_name, char new_hex, bool new_read, COMMAND_TYPE_OPTIONS new_type) {
    name = new_name;
    hex = new_hex;
    read = new_read;
    Command::set_type(new_type);
}

void Command::set_type(COMMAND_TYPE_OPTIONS new_type) {
    type = new_type;
    switch(type) {
        case SPW:
            // require set_spw_options()?
            break;
        case SPI:
            // require set_spi_options()?
            break;
        case UART:
            // require set_uart_options?
            break;
        default:
            std::cerr << "switch/case in Command::set_type fell through\n";
    }
}

void Command::set_spw_options(
    char new_spw_instruction,
    std::vector<char> new_spw_address,
    std::vector<char> new_spw_write_data,
    unsigned short new_spw_reply_length
) {
    if(type != SPW) {
        std::cout << "this Command was not initialized as a SpaceWire command, but is being provided SpaceWire field data. May need to change Command.type later\n";
    }
    spw_instruction = new_spw_instruction;

    if(new_spw_address.size() > 0) {
        spw_address = new_spw_address;
    } else {
        std::cerr << "input SpaceWire address field must have nonzero length\n";
    }

    if(read) {
        if(new_spw_write_data.size() > 0) {
            std::cerr << "this Command was initialized as a read-command, but has been given data to write\n";
        }
        if(new_spw_reply_length == 0) {
           std::cerr << "this Command was initialized as a read-command, but expects a zero-length reply\n";
        }
    } else {
        if(new_spw_write_data.size() == 0) {
            std::cerr << "this Command was initialized as a write-command, but has been given no data to write\n";
        }
    }

    spw_write_data = new_spw_write_data;
    spw_reply_length = new_spw_reply_length;
}

std::vector<char> Command::get_command_bytes() {
    // hook into Takayuki Yuasa RMAP lib here?
    std::vector<char> target_spw_address;
    char target_log_address;
    char protocol_id;
    // instruction already have
    char key;
    std::vector<char> reply_address;
    //...

    std::vector<char> result;

    result.insert(result.end(), target_spw_address.begin(), target_spw_address.end());

    if(read) {

        // append more stuff
    } else {

    }

    return result;
}

char* Command::get_command_bytes_raw() {
    std::vector<char> command_bytes = Command::get_command_bytes();
    // return ref to first element of the vector
    char* out_buff;

    return std::copy(command_bytes.begin(), command_bytes.end(), out_buff);
}

Command::Command(const Command& other): 
    name(other.name),
    hex(other.hex),
    read(other.read),
    type(other.type),
    spw_instruction(other.get_spw_instruction()),
    spw_write_data(other.get_spw_write_data()),
    spw_address(other.get_spw_address()),
    spw_reply_length(other.get_spw_reply_length()),
    full_command(other.get_full_command()),
    uart_instruction(other.get_uart_instruction()),
    spi_address(other.get_spi_address()),
    spi_write_data(other.get_spi_write_data()),
    spi_reply_length(other.get_spi_reply_length())
{}

Command& Command::operator=(const Command& other) {
    name = other.name;
    hex = other.hex;
    read = other.read;
    type = other.type;
    spw_instruction = other.get_spw_instruction();
    spw_write_data = other.get_spw_write_data();
    spw_address = other.get_spw_address();
    spw_reply_length = other.get_spw_reply_length();
    full_command = other.get_full_command();
    uart_instruction = other.get_uart_instruction();
    spi_address = other.get_spi_address();
    spi_write_data = other.get_spi_write_data();
    spi_reply_length = other.get_spi_reply_length();

    return *this;
}

Command::Command() {}

System::System(std::string new_name, char new_hex) {
    name = new_name;
    hex = new_hex;
}

System::System(const System& other): name(other.name), hex(other.hex) {}

System& System::operator=(const System& other) {
    name = other.name;
    hex = other.hex;
    return *this;
}


CommandDeck::CommandDeck(std::unordered_map<std::string, std::string> named_paths) {
    did_add_systems_ = false;
    std::cout << "constructing command deck...\n";

    // build systems list first
    std::string sys_filename;
    std::unordered_map<std::string, std::string> named_paths_no_systems = named_paths;
    if(auto search = named_paths.find("SYSTEMS"); search != named_paths.end()) {
        // write down the system filename to use to open later
        sys_filename = search->second;
        // remove the system file from the map so can just look at commands later
        named_paths_no_systems.erase(search->first);
    } else {
        std::cerr << "could not find `SYSTEMS` label in named_paths, so cannot build CommandDeck\n";
        exit(0);
    }

    CommandDeck::add_systems(sys_filename);

    if(!did_add_systems_) {
        std::cerr << "did not successfully add systems from file\n";
        exit(0);
    }
    
    // now add commands
    CommandDeck::add_commands(named_paths_no_systems);
    std::cout << "finished adding commands\n";
}

void CommandDeck::add_systems(std::string sys_filename) {
    std::cout << "constructing command deck...\n";

    std::ifstream sys_file;

    // try to open the systems file
    sys_file.open(sys_filename);
    if(!sys_file.is_open()) {
        std::cerr << "couldn't open settings file in CommandDeck\n";
        exit(0);
    }

    // parse the systems file:
    nlohmann::json settings_data = nlohmann::json::parse(sys_file);
    for(auto& this_entry: settings_data.items()) {
        std::string this_name = this_entry.value()["name"];
        std::string this_hex_str = this_entry.value()["id"];
        char this_hex;
        // convert hex code string to char type. Note: this strtol method accepts both 0x- prefixed and raw hex strings.
        this_hex = strtol(this_hex_str.c_str(), NULL, 16);

        debug_print("adding new System:");
        debug_print("\tname:    " + this_name);
        debug_print("\tid code: " + std::to_string(this_hex));
        
        systems.push_back(System(this_name, this_hex));
    }

    sys_file.close();
    did_add_systems_ = true;
}

void CommandDeck::add_commands(std::unordered_map<std::string, std::string> named_paths) {
    for(const auto &[path_key, path_val]: named_paths) {
        // maybe wrap this in try{}
        std::ifstream file;
        file.open(path_val, std::ios::in);
        if(!file.is_open()) {
            std::cerr << "couldn't open file in CommandDeck\n";
            exit(0);
        }

        // parse the file
        nlohmann::json data = nlohmann::json::parse(file);
        
        if(path_key.compare("SYSTEMS") == 0) {
            debug_print("\tgot system again");
            // already did this above, no need to repeat
        } else {

            std::unordered_map<char, Command> inner_command_map;

            debug_print("adding commands...");

            for(auto& this_entry: data.items()) {
                // set primary command properties

                std::string this_hex_str = this_entry.value()["hex"].get<std::string>();
                char this_hex;
                // convert hex code string to char type. Note: this strtol method accepts both 0x- prefixed and raw hex strings.
                this_hex = strtol(this_hex_str.c_str(), NULL, 16);
                std::string this_name = this_entry.value()["name"].get<std::string>();
                std::string this_read_str = this_entry.value()["R=1/W=0"].get<std::string>();
                bool this_read;
                std::istringstream(this_read_str) >> this_read;

                debug_print("\tadding protocol-specific data...");
                
                // add data to command based on specific protocol used
                if(path_key.compare("TIMEPIX") == 0) {
                    // handle commands for TIMEPIX (over UART)
                    Command this_command = Command(this_name, this_hex, this_read, UART);

                    //TODO: more here
                
                } else if(path_key.compare("CDTE1") == 0 || path_key.compare("CDTE2") == 0 || path_key.compare("CDTE3") == 0 || path_key.compare("CDTE4") == 0 || path_key.compare("CDTEDE") == 0 || path_key.compare("CMOS1") == 0 || path_key.compare("CMOS2") == 0) {
                    // handle commands for CDTE or CMOS (over SPW)

                    // make a new Command object to store this information
                    Command this_command = Command(this_name, this_hex, this_read, SPW);

                    // fill in protocol-specific details in the Command (for SpaceWire)
                    std::string this_spw_instr_str = this_entry.value()["instruction"];
                    std::string this_spw_addr_str = this_entry.value()["address"];
                    std::string this_spw_write_data_str = this_entry.value()["length"];

                    char this_spw_instr = strtol(this_spw_instr_str.c_str(), NULL, 16);
                    std::vector<char> this_spw_addr(this_spw_addr_str.begin(), this_spw_addr_str.end());
                    // TODO: add reply length explicitly to JSON file field.

                    std::vector<char> this_spw_write_data(this_spw_write_data_str.begin(), this_spw_write_data_str.end());

                    this_command.set_spw_options(this_spw_instr, this_spw_addr, this_spw_write_data, 0);

                    // add the command to the deck for this subsystem
                    commands[CommandDeck::get_sys_code_for_name(path_key)].insert(std::make_pair(this_hex, this_command));

                } else if(path_key.compare("HOUSEKEEPING") == 0) {
                    // handle commands for HK (over SPI (via Ethernet))
                    Command this_command = Command(this_name, this_hex, this_read, SPI);

                    // TODO: add more here for SPI

                } else {
                    std::cerr << "unknown filetype " + path_key + "found in CommandDeck constructor.\n";
                    exit(0);
                }
            }
            // add all the commands for this system to the total deck
            debug_print("\tadded system commands to deck");
        }
        file.close();
    }
}

System& CommandDeck::get_sys_for_name(std::string name) {
    // search for System with name in systems
    for(int i=0; i < systems.size(); i++) {
        if(name.compare(systems[i].name) == 0) {
            return systems[i];
        }
    }

    // return some default error System
    std::cout << "found no system with name " << name << "\n";
    static System null_sys = System("null", 0xFF);
    return null_sys;
}

System& CommandDeck::get_sys_for_code(char code) {
    // search for System with name in systems
    for(int i=0; i < systems.size(); i++) {
        if(code == systems[i].hex) {
            return systems[i];
        }
    }
    
    // return some default error System
    std::cout << "found no system with code " << std::to_string(code) << "\n";
    static System null_sys = System("null", 0xFF);
    return null_sys;
}

char CommandDeck::get_sys_code_for_name(std::string name) {
    return CommandDeck::get_sys_for_name(name).hex;
}

std::string CommandDeck::get_sys_name_for_code(char code) {
    return CommandDeck::get_sys_for_code(code).name;
}

Command& CommandDeck::get_command_for_sys_for_code(char sys, char code) {
    // check for key `sys` in outer map
    if(commands.contains(sys)) {
        debug_print("\tcontains sys");
        // check for key `code` in inner map
        if(commands[sys].contains(code)) {
            debug_print("\tcontains code");
            debug_print("\tsystem: " + CommandDeck::get_sys_name_for_code(sys));
            return (commands[sys][code]);
        }
        throw std::runtime_error("couldn't find " + std::to_string(code) + " in CommandDeck.commands\n");
    }
    throw std::runtime_error("couldn't find " + std::to_string(sys) + " in CommandDeck.commands\n");
    
}

void CommandDeck::print() {
    std::cout << "printing commands:\n";
    for(const auto& [sys_hex, map_val]: commands) {
        std::cout << "\tsystem " + get_sys_name_for_code(sys_hex) + " has commands:\n";
        for(const auto& [com_hex, com]: map_val) {
            std::cout << "\t\t0x" << std::hex << (unsigned int)com_hex << ": " << com.name << "\n";
        }
    }
}