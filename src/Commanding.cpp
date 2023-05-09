#include "Commanding.h"
#include "Utilities.h"
#include <nlohmann/json.hpp>
#include <iostream>

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
    std::vector<char> new_spw_instruction,
    std::vector<char> new_spw_address,
    std::vector<char> new_spw_write_data,
    unsigned short new_spw_reply_length
) {
    if(type != SPW) {
        std::cout << "this Command was not initialized as a SpaceWire command, but is being provided SpaceWire field data. May need to change Command.type later\n";
    }
    if(new_spw_instruction.size() > 0) {
        spw_instruction = new_spw_instruction;
    } else {
        std::cerr << "input SpaceWire instruction field must have nonzero length\n";
    }

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

    if(read) {
        result.insert(result.end(), target_spw_address.begin(), target_spw_address.end());
    }
}


char* Command::get_command_bytes_raw() {
    std::vector<char> command_bytes = Command::get_command_bytes();
    // return ref to first element of the vector
    return &command_bytes[0];
}


System::System(std::string name, char hex) {
    name = name;
    hex = hex;
}




CommandDeck::CommandDeck(std::unordered_map<FILES, std::string> named_paths) {
    std::cout << "constructing command deck...\n";
    for(const auto &[path_key, path_val]: named_paths) {
        // try to open file
        // construct Command-derived class from all the commands in the file
        // store in commands map

        std::ifstream file;
        
        // maybe wrap this in try{}
        file.open(path_val, std::ios::in);
        if(!file.is_open()) {
            std::cerr << "couldn't open file in CommandDeck.\n";
            exit(0);
        }

        // parser:
        nlohmann::json data = nlohmann::json::parse(file);

        if(path_key == SYSTEMS) {

        } else {
            // loop over commands
            // construct Command-derived object for each command
            // store in commands map.

            // first-order data in command list is an array. In this case, this_entry.key() returns an index (starting at 0) and this_entry.value() returns the json data at that index.

            for(auto& this_entry: data.items()) {
                switch(path_key) {
                    case TIMEPIX: {
                        // new_command = UARTCommand::UARTCommand()
                        break;
                    }
                    case CDTE: {
                        Command new_command = Command(
                            this_entry.value()["name"].get<std::string>(),
                            this_entry.value()["hex"].get<char>(),
                            this_entry.value()["read"].get<bool>(),
                            COMMAND_TYPE_OPTIONS::SPW
                        );
                        break;
                    }
                    case CDTE_OFFSET:

                    case CMOS:

                    case HK:

                    default:
                        std::cerr << "switch/case fell through in CommandDeck constructor\n";
                }
            }
        }

        
        file.close();

    }
}