#include "Commanding.h"
#include "Utilities.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <stdlib.h>


Command::Command(std::string name, char hex, bool read, COMMAND_TYPE_OPTIONS type) {
    name = name;
    hex = hex;
    read = read;
    Command::set_type(type);
}

void Command::set_type(COMMAND_TYPE_OPTIONS new_type) {
    type = new_type;
    switch(type) {
        case SPW:

            break;
        case SPI:

            break;
        case UART:

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
    // hook into Takayuki Yuasa RMAP lib here.
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


System::System(std::string name, char hex) {
    name = name;
    hex = hex;
}



CommandDeck::CommandDeck(std::unordered_map<std::string, std::string> named_paths) {
    std::cout << "constructing command deck...\n";

    std::ifstream sys_file;

    // first, make all Systems:
    sys_file.open(named_paths["SYSTEMS"]);
    if(!sys_file.is_open()) {
        std::cerr << "couldn't open settings file in CommandDeck\n";
        exit(0);
    }

    // parse the settings file:
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
        System new_system = System(
            this_name,
            this_hex
        );
        systems.push_back(new_system);
    }

    sys_file.close();

    for(const auto &[path_key, path_val]: named_paths) {
        // try to open file
        // construct Command-derived class from all the commands in the file
        // store in commands map
        
        // maybe wrap this in try{}
        std::ifstream file;
        file.open(path_val, std::ios::in);
        if(!file.is_open()) {
            std::cerr << "couldn't open file in CommandDeck\n";
            exit(0);
        }

        // parse the file
        nlohmann::json data = nlohmann::json::parse(file);
        
        debug_print(path_key);
        if(path_key.compare("SYSTEMS") == 0) {
            debug_print("\tgot system again");
            // already did this above
        } else {
            // loop over commands
            // construct Command-derived object for each command
            // store in commands map.

            // first-order data in command list is an array. In this case, this_entry.key() returns an index (starting at 0) and this_entry.value() returns the json data at that index.

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
                    Command this_command = Command(this_name, this_hex, this_read, UART);
                
                } else if(path_key.compare("CMOS") == 0) {
                    Command this_command = Command(this_name, this_hex, this_read, SPW);
                
                } else if(path_key.compare("CDTE") == 0) {
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
                    inner_command_map.insert(std::pair{this_hex, this_command});

                } else if(path_key.compare("HK") == 0) {
                    Command this_command = Command(this_name, this_hex, this_read, SPI);

                } else if(path_key.compare("CDTE_OFFSET") == 0) {

                } else {
                    std::cerr << "unknown filetype " + path_key + "found in CommandDeck constructor.\n";
                    exit(0);
                }
            }
            // add all the commands for this system to the total deck
            commands.insert(std::pair{CommandDeck::get_sys_code_for_name(path_key), inner_command_map});
            debug_print("\tadded system commands to deck");
        }
        file.close();
    }
}

char CommandDeck::get_sys_code_for_name(std::string name) {
    for(System sys: systems) {
        if(name.compare(sys.name) == 0) {
            return sys.hex;
        }
    }
}