#ifndef COMMANDING_H
#define COMMANDING_H

#include "Parameters.h"
#include <unordered_map>
#include <vector>
#include <fstream>

// object to hold command metadata (TODO: add more attributes)
class Command {
    public:
        std::string name;
        char hex;
        bool read;

        COMMAND_TYPE_OPTIONS type;
        
        Command(std::string name, char hex, bool read, COMMAND_TYPE_OPTIONS);
        
        void set_type(COMMAND_TYPE_OPTIONS type);
        
        void set_spw_options(
            std::vector<char> new_spw_instruction,
            std::vector<char> new_spw_address,
            std::vector<char> new_spw_write_data,
            unsigned short new_spw_reply_length
        );

        void set_uart_options(std::vector<char> new_uart_instruction);
        
        void set_spi_options(
            std::vector<char> new_spi_address,
            std::vector<char> new_spi_write_data,
            unsigned short new_spi_reply_length
        );
        
        std::vector<char> get_command_bytes();
        char* get_command_bytes_raw();
        std::vector<char> get_command_bytes(char arg);
    
    private:
        // SpaceWire-related data:
        std::vector<char> spw_instruction;
        std::vector<char> spw_address;
        std::vector<char> spw_write_data;
        unsigned short spw_reply_length;
        char* full_command;

        // UART-related data:
        std::vector<char> uart_instruction;

        // SPI-related data:
        std::vector<char> spi_address;
        std::vector<char> spi_write_data;
        unsigned short spi_reply_length;
};

// object to hold system metadata
class System {
    public:
        std::string name;
        char hex;
        
        System(std::string name, char hex);
};


class CommandDeck {
    public:
        enum FILES {
            SYSTEMS,
            TIMEPIX,
            CDTE,
            CDTE_OFFSET,
            CMOS,
            HK
        };

        std::vector<System> systems;

        // first key is hex code for system, second key is hex code for command. 
        std::unordered_map<char, std::unordered_map<char, Command>> commands;
        // more on this: will define command maps for each system (CdTe 1, 2, etc; CMOS 1, 2...) then key into each map by the hex code for each system 

        // pass in a map pairing system text names ("CDTE") with json command file paths
        // CommandDeck(std::unordered_map<std::string, std::string> named_paths);
        CommandDeck(std::unordered_map<FILES, std::string> named_paths);
        
        int             validate(void);

        std::string     get_sys_name_for_code(char code);
        char            get_sys_code_for_name(std::string name);
        System&         get_sys_for_code(char code);
        System&         get_sys_for_name(std::string name);

        Command*        get_command_for_sys_for_cmd(char sys, char code);
    
    private:
        
};

#endif