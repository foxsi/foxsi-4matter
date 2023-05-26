#include "Commanding.h"
#include "Utilities.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <stdlib.h>


Command::Command(std::string new_name, uint8_t new_hex, bool new_read, COMMAND_TYPE_OPTIONS new_type) {
    name = new_name;
    hex = new_hex;
    read = new_read;
    Command::set_type(new_type);
}

void Command::set_type(COMMAND_TYPE_OPTIONS new_type) {
    type = new_type;
    switch(type) {
        case COMMAND_TYPE_OPTIONS::SPW:
            // require set_spw_options()?
            break;
        case COMMAND_TYPE_OPTIONS::SPI:
            // require set_spi_options()?
            break;
        case COMMAND_TYPE_OPTIONS::UART:
            // require set_uart_options?
            break;
        default:
            std::cerr << "switch/case in Command::set_type fell through\n";
    }
}

void Command::set_spw_options(
    uint8_t new_spw_instruction,
    std::vector<uint8_t> new_spw_address,
    std::vector<uint8_t> new_spw_write_data,
    unsigned short new_spw_reply_length
) {
    if(type != COMMAND_TYPE_OPTIONS::SPW) {
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

std::vector<uint8_t> Command::get_command_bytes() {
    // hook into Takayuki Yuasa RMAP lib here?
    std::vector<uint8_t> result;
    result.push_back(0x00);
    return  result;
}

uint8_t* Command::get_command_bytes_raw() {
    std::vector<uint8_t> command_bytes = Command::get_command_bytes();
    // return ref to first element of the vector
    uint8_t* out_buff;

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

System::System(std::string new_name, uint8_t new_hex) {
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
        uint8_t this_hex;
        // convert hex code string to uint8_t type. Note: this strtol method accepts both 0x- prefixed and raw hex strings.
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

            std::unordered_map<uint8_t, Command> inner_command_map;

            debug_print("adding commands...");

            for(auto& this_entry: data.items()) {
                // set primary command properties

                std::string this_hex_str = this_entry.value()["hex"].get<std::string>();
                uint8_t this_hex;
                // convert hex code string to uint8_t type. Note: this strtol method accepts both 0x- prefixed and raw hex strings.
                this_hex = strtol(this_hex_str.c_str(), NULL, 16);
                std::string this_name = this_entry.value()["name"].get<std::string>();
                std::string this_read_str = this_entry.value()["R=1/W=0"].get<std::string>();
                bool this_read;
                std::istringstream(this_read_str) >> this_read;

                debug_print("\tadding protocol-specific data...");
                
                // add data to command based on specific protocol used
                if(path_key.compare("TIMEPIX") == 0) {
                    // handle commands for TIMEPIX (over UART)
                    Command this_command = Command(this_name, this_hex, this_read, COMMAND_TYPE_OPTIONS::UART);

                    //TODO: more here
                
                } else if(path_key.compare("CDTE1") == 0 || path_key.compare("CDTE2") == 0 || path_key.compare("CDTE3") == 0 || path_key.compare("CDTE4") == 0 || path_key.compare("CDTEDE") == 0 || path_key.compare("CMOS1") == 0 || path_key.compare("CMOS2") == 0) {
                    // handle commands for CDTE or CMOS (over SPW)

                    // make a new Command object to store this information
                    Command this_command = Command(this_name, this_hex, this_read, COMMAND_TYPE_OPTIONS::SPW);

                    // fill in protocol-specific details in the Command (for SpaceWire)
                    std::string this_spw_instr_str = this_entry.value()["instruction"];
                    std::string this_spw_addr_str = this_entry.value()["address"];
                    std::string this_spw_write_data_str = this_entry.value()["length"];

                    uint8_t this_spw_instr = strtol(this_spw_instr_str.c_str(), NULL, 16);
                    // TODO: add reply length explicitly to JSON file field.

                    std::vector<uint8_t> this_spw_write_data;
                    std::vector<uint8_t> this_spw_addr;

                    unsigned short this_spw_reply_length = 0;
                    if(this_read) {
                        this_spw_reply_length = strtol(this_spw_write_data_str.c_str(), NULL, 16);
                    } else {
                        this_spw_write_data = string_to_chars(this_spw_write_data_str);
                    }
                    

                    this_spw_addr = string_to_chars(this_spw_addr_str);

                    this_command.set_spw_options(this_spw_instr, this_spw_addr, this_spw_write_data, this_spw_reply_length);

                    // add the command to the deck for this subsystem
                    commands[CommandDeck::get_sys_code_for_name(path_key)].insert(std::make_pair(this_hex, this_command));

                } else if(path_key.compare("HOUSEKEEPING") == 0) {
                    // handle commands for HK (over SPI (via Ethernet))
                    Command this_command = Command(this_name, this_hex, this_read, COMMAND_TYPE_OPTIONS::SPI);

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

System& CommandDeck::get_sys_for_code(uint8_t code) {
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

uint8_t CommandDeck::get_sys_code_for_name(std::string name) {
    return CommandDeck::get_sys_for_name(name).hex;
}

std::string CommandDeck::get_sys_name_for_code(uint8_t code) {
    return CommandDeck::get_sys_for_code(code).name;
}

Command& CommandDeck::get_command_for_sys_for_code(uint8_t sys, uint8_t code) {
    // check for key `sys` in outer map
    if(commands.contains(sys)) {
        debug_print("\tcontains sys");
        // check for key `code` in inner map
        if(commands[sys].contains(code)) {
            debug_print("\tcontains code");
            debug_print("\tsystem: " + CommandDeck::get_sys_name_for_code(sys));
            return (commands[sys][code]);
        }
        // throw std::runtime_error("couldn't find " + std::to_string(code) + " in CommandDeck.commands\n");
        std::cout << "couldn't find " + std::to_string(code) + " in CommandDeck.commands\n";
        static Command null_command = Command();
        return null_command;
    }
    // throw std::runtime_error("couldn't find " + std::to_string(sys) + " in CommandDeck.systems\n");
    std::cout << "couldn't find " + std::to_string(sys) + " in CommandDeck.systems\n";
    static Command null_command = Command();
    return null_command;
    
}

std::vector<uint8_t> CommandDeck::get_command_bytes_for_sys_for_code(uint8_t sys, uint8_t code) {
    //  W command format:
        //      Target SpW address      [nB]...
        //      Target logical address  [1B]
        //      Protocol ID             [1B, = 0x01 for RMAP]
        //      Instruction             [1B, = Command::instruction]
        //      Key                     [1B]
        //      Reply address           [0B, 4B, 8B, or 12B, specified by Reply Addr Len Field in Instruction]
        //      Initiator logical addr  [1B]
        //      Transaction ID MSB      [1B]
        //      Transaction ID LSB      [1B]
        //      Extended address        [1B]
        //      Memory address (M->LSB) [4B]
        //      Data length    (M->LSB) [3B]
        //      Header CRC              [1B]
        //      Data                    [nB]
        //      Data CRC                [1B]
        //      EOP char                [1B]

        // read commands the same, but:
        //      Instruction should specify a read command
        //      Packet ends (EOP) after Header CRC.

    Command cmd = CommandDeck::get_command_for_sys_for_code(sys, code);
    std::vector<uint8_t> full_packet;
    

    if(cmd.type == COMMAND_TYPE_OPTIONS::SPW) {
        std::vector<uint8_t> ethernet_header;
        std::vector<uint8_t> rmap_packet;

        // std::vector<uint8_t> TARGET_PATH_ADDRESS_OUT = {0x07,0x02};        // todo: add this as attribute of System? this info from Nagasawa

        std::vector<uint8_t> TARGET_PATH_ADDRESS_OUT = {0x07, 0x02};
        uint8_t TARGET_LOGICAL_ADDRESS = 0xFE;     // todo: add this as attribute of System
        uint8_t KEY = 0x02;                        // todo: add this as attribute of System
        uint8_t protocol_id = 0x01;
        std::vector<uint8_t> REPLY_ADDRESS = {0x00,0x00,0x06,0x03};
        // std::vector<uint8_t> REPLY_ADDRESS = {0x00, 0x00, 0x06,0x03};              // todo: define this in Parameters or something
        // std::vector<uint8_t> REPLY_ADDRESS = {0x01,0x03};              // todo: for flight I think it will be {0x01, 0x03}
        uint8_t initiator_logical_address = 0xFE;  // todo: define this in Parameters or something
        uint8_t transaction_id_lsb = 0x00;         // todo: move this to a higher scope and increment
        uint8_t transaction_id_msb = 0x00;
        uint8_t extended_address = 0x00;

        if(cmd.read) {
            // SpW read commands

            /* to make a read command, would want to pass in read 
            address as argument. System/TransportLayerMachine could 
            store a property `data_address` (initialized to offset of 
            read address memory) and `frame_size`. Then each read 
            operation increments `frame_size`. */
            
            std::vector<uint8_t> header;

            uint8_t instruction = cmd.get_spw_instruction();
            std::cout << "\tinstruction: " << std::hex << (int)instruction << "\n";
            
            // TODO: un-hardcode the instructions (format read JSON correctly)
            instruction = 0x4d;

            // TODO: increment read address
            std::vector<uint8_t> memory_address = {0x00,0x00,0x00,0x00};
            // TODO: find correct frame size value
            // unsigned int read_data_length = 32000;
            unsigned short read_data_length = cmd.get_spw_reply_length();

            const uint8_t dl0 = read_data_length & 0xff;
            const uint8_t dl1 = (read_data_length >> 8) & 0xff;
            const uint8_t dl2 = (read_data_length >> 16) & 0xff;
            std::vector<uint8_t> data_length;
            data_length.push_back(dl2);
            data_length.push_back(dl1);
            data_length.push_back(dl0);
            
            header.push_back(TARGET_LOGICAL_ADDRESS);
            header.push_back(protocol_id);
            header.push_back(instruction);
            header.push_back(KEY);
            header.insert(header.end(), REPLY_ADDRESS.begin(), REPLY_ADDRESS.end());
            header.push_back(initiator_logical_address);
            header.push_back(transaction_id_msb);
            header.push_back(transaction_id_lsb);
            header.push_back(extended_address);
            header.insert(header.end(), memory_address.begin(), memory_address.end());
            header.insert(header.end(), data_length.begin(), data_length.end());
            
            uint8_t header_crc = spw_calculate_crc_uint_F(header);
            header.push_back(header_crc);

            rmap_packet.insert(rmap_packet.end(), TARGET_PATH_ADDRESS_OUT.begin(), TARGET_PATH_ADDRESS_OUT.end());

            rmap_packet.insert(rmap_packet.end(), header.begin(), header.end());

            

            // TODO: handle read address here

            // std::cerr << "unimplemesnted command type!\n";
        } else {
            // SpW write command

            // uint8_t instruction = cmd.get_spw_instruction();            
            // std::vector<uint8_t> memory_address = cmd.get_spw_address();
            // TODO: un-hardcode the instructions (format read JSON correctly)

            std::vector<uint8_t> header;

            uint8_t instruction = cmd.get_spw_instruction();
            std::cout << "\tinstruction: " << std::hex << (int)instruction << "\n";

            instruction = 0x7d;

            // TODO: un-hardcode the read address
            std::vector<uint8_t> memory_address = {0x00, 0x00, 0x00, 0x00};

            std::vector<uint8_t> write_data = cmd.get_spw_write_data();
            hex_print(write_data);
            debug_print("");

            unsigned int write_data_length = write_data.size();
            debug_print("write data size: " + std::to_string(write_data_length));
            const uint8_t dl0 = write_data_length & 0xff;
            const uint8_t dl1 = (write_data_length >> 8) & 0xff;
            const uint8_t dl2 = (write_data_length >> 16) & 0xff;
            std::vector<uint8_t> data_length;
            data_length.push_back(dl2);
            data_length.push_back(dl1);
            data_length.push_back(dl0);
            
            
            header.push_back(TARGET_LOGICAL_ADDRESS);
            header.push_back(protocol_id);
            header.push_back(instruction);
            header.push_back(KEY);
            header.insert(header.end(), REPLY_ADDRESS.begin(), REPLY_ADDRESS.end());
            header.push_back(initiator_logical_address);
            header.push_back(transaction_id_msb);
            header.push_back(transaction_id_lsb);
            header.push_back(extended_address);
            header.insert(header.end(), memory_address.begin(), memory_address.end());
            header.insert(header.end(), data_length.begin(), data_length.end());
            
            uint8_t header_crc = spw_calculate_crc_uint_F(header);
            header.push_back(header_crc);

            std::vector<uint8_t> data;
            data.insert(data.end(), write_data.begin(), write_data.end());
            uint8_t data_crc = spw_calculate_crc_uint_F(data);
            data.push_back(data_crc);

            std::cout << "rmap write data lookup:\t";
            hex_print(data);
            std::cout << "\n";
            
            rmap_packet.insert(rmap_packet.end(), TARGET_PATH_ADDRESS_OUT.begin(), TARGET_PATH_ADDRESS_OUT.end());
            rmap_packet.insert(rmap_packet.end(), header.begin(), header.end());
            rmap_packet.insert(rmap_packet.end(), data.begin(), data.end());

        }

        ethernet_header = CommandDeck::get_spw_ether_header(rmap_packet);
        full_packet.insert(full_packet.end(), ethernet_header.begin(), ethernet_header.end());
        full_packet.insert(full_packet.end(), rmap_packet.begin(), rmap_packet.end());

        return full_packet;

    } else {
        std::cerr << "unimplemented command type!\n";
        return full_packet;
    }
}

std::vector<uint8_t> CommandDeck::get_write_command_bytes_for_sys_for_HARDCODE(uint8_t sys, uint8_t code) {

    std::vector<uint8_t> rmap_packet;
    std::vector<uint8_t> path_address = {
        0x07,
        0x02
    };
    std::vector<uint8_t> header = {
        0xfe, // target logical address
        0x01, // protocol ID (RMAP)
        0x7D, // instruction field
        0x02, // key
        0x00, // reply path addr MS (min 4B)
        0x00,
        0x06,
        0x03, // reply path addr LS
        0xfe, // initiator logical address
        0x00, // transaction ID MS
        0x00, // transaction ID LS
        0x00, // ext address
        0x00, // address MS
        0x00,
        0x00,
        0x00, // address LS
        0x00, // data length MS
        0x00,
        0x0c  // data length LS
    };
    // CRC should be calculated EXCLUDING PATH ADDRESS
    uint8_t header_crc = spw_calculate_crc_uint_F(header);
    header.push_back(header_crc);
    std::vector<uint8_t> data = {
        0x3c,
        0x3c,
        0x01,
        0x00,
        0x03,
        0x03,
        0x03,
        0x03,
        0x3c,
        0x3c,
        0x3c,
        0x3c
    };
    // uint8_t data_crc = spw_calculate_crc_uint_F(data);
    // data.push_back(data_crc);
    // TODO: INVESTIGATE, POSSIBLY un-HARDCODE THIS:
    data.push_back(0x00);
    
    
    rmap_packet.insert(rmap_packet.end(), path_address.begin(), path_address.end());
    rmap_packet.insert(rmap_packet.end(), header.begin(), header.end());
    rmap_packet.insert(rmap_packet.end(), data.begin(), data.end());

    // TODO: SPMU-001 RMAP NEEDS A 12B ETHERNET PREFIX TO RECEIVE RMAP.
    
    std::vector<uint8_t> ether_prefix;
    const unsigned long rmap_packet_size = rmap_packet.size();
    std::cout << "\trmap packet size: " << std::to_string(rmap_packet_size) << "\n";
    const uint8_t dl0 = rmap_packet_size & 0xff;
    const uint8_t dl1 = (rmap_packet_size >> 8) & 0xff;
    const uint8_t dl2 = (rmap_packet_size >> 16) & 0xff;
    const uint8_t dl3 = (rmap_packet_size >> 24) & 0xff;

    ether_prefix.push_back(0x00);   // terminate packet with EOP
    ether_prefix.push_back(0x00);   // prefix byte 2 is ALWAYS 0x00
    ether_prefix.push_back(0x00);   // 10-B size field MSB
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(dl3);
    ether_prefix.push_back(dl2);
    ether_prefix.push_back(dl1);
    ether_prefix.push_back(dl0);    // 10-B size field LSB

    std::vector<uint8_t> full_packet;

    full_packet.insert(full_packet.end(), ether_prefix.begin(), ether_prefix.end());
    full_packet.insert(full_packet.end(), rmap_packet.begin(), rmap_packet.end());

    return full_packet;
}

std::vector<uint8_t> CommandDeck::get_read_command_bytes_for_sys_for_HARDCODE(uint8_t sys, uint8_t code) {

    std::vector<uint8_t> rmap_packet;
    std::vector<uint8_t> path_address = {
        0x07,
        0x02
    };
    std::vector<uint8_t> header = {
        0xfe, // target logical address
        0x01, // protocol ID (RMAP)
        0x4D, // instruction field
        0x02, // key
        0x00, // reply path addr MS (min 4B)
        0x00,
        0x06,
        0x03, // reply path addr LS
        0xfe, // initiator logical address
        0x00, // transaction ID MS
        0x00, // transaction ID LS
        0x00, // ext address
        0x00, // address MS
        0x00,
        0x00,
        0x00, // address LS
        0x00, // data length MS
        0x00,
        0x0c  // data length LS
    };
    // header CRC should be calculated EXCLUDING PATH ADDRESS
    uint8_t header_crc = spw_calculate_crc_uint_F(header);
    header.push_back(header_crc);
    
    rmap_packet.insert(rmap_packet.end(), path_address.begin(), path_address.end());
    rmap_packet.insert(rmap_packet.end(), header.begin(), header.end());

    // rmap_packet.insert(rmap_packet.end(), data.begin(), data.end());

    // TODO: SPMU-001 RMAP NEEDS A 12B ETHERNET PREFIX TO RECEIVE RMAP.
    
    std::vector<uint8_t> ether_prefix;
    const unsigned long rmap_packet_size = rmap_packet.size();
    std::cout << "\trmap packet size: " << std::to_string(rmap_packet_size) << "\n";
    const uint8_t dl0 = rmap_packet_size & 0xff;
    const uint8_t dl1 = (rmap_packet_size >> 8) & 0xff;
    const uint8_t dl2 = (rmap_packet_size >> 16) & 0xff;
    const uint8_t dl3 = (rmap_packet_size >> 24) & 0xff;

    ether_prefix.push_back(0x00);   // terminate packet with EOP
    ether_prefix.push_back(0x00);   // prefix byte 2 is ALWAYS 0x00
    ether_prefix.push_back(0x00);   // 10-B size field MSB
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(dl3);
    ether_prefix.push_back(dl2);
    ether_prefix.push_back(dl1);
    ether_prefix.push_back(dl0);    // 10-B size field LSB

    std::vector<uint8_t> full_packet;

    full_packet.insert(full_packet.end(), ether_prefix.begin(), ether_prefix.end());
    full_packet.insert(full_packet.end(), rmap_packet.begin(), rmap_packet.end());

    return full_packet;
}

std::vector<uint8_t> CommandDeck::get_spw_ether_header(std::vector<uint8_t> rmap_packet) {
    // NOTE: this method does not support RMAP packets with sizes that require more than 8B to represent (~1.84e19 B)
    std::vector<uint8_t> ether_prefix;
    const unsigned long long rmap_packet_size = rmap_packet.size();

    ether_prefix.push_back((uint8_t)SPACEWIRE_END_OPTIONS::EOP);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);
    ether_prefix.push_back(0x00);

    const unsigned long long mask = 0xff;
    for(int i = 7; i >= 0; --i) {
        const unsigned long long size_byte = (rmap_packet_size >> (8*i)) & mask;
        ether_prefix.push_back((uint8_t)size_byte);
    }

    return ether_prefix;
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