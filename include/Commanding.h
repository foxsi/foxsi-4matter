#ifndef COMMANDING_H
#define COMMANDING_H

#include "Parameters.h"
#include <unordered_map>
#include <vector>
#include <fstream>

// object to hold command metadata (TODO: add more attributes)
class Command {
    public:
        /* Public properties */
        std::string name;
        char hex;
        bool read;

        COMMAND_TYPE_OPTIONS type;
        
        /* Public methods */
        Command(std::string new_name, char new_hex, bool new_read, COMMAND_TYPE_OPTIONS new_type);
        Command(const Command& other);
        Command& operator=(const Command& other);
        Command();
        
        void set_type(COMMAND_TYPE_OPTIONS type);
        
        void set_spw_options(
            char new_spw_instruction,
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

        // getter methods for private variables
        const char get_spw_instruction() const {return spw_instruction;}
        const std::vector<char> get_spw_write_data() const {return spw_write_data;}
        const std::vector<char> get_spw_address()  const {return spw_address;}
        const unsigned short get_spw_reply_length() const {return spw_reply_length;}
        const std::vector<char> get_full_command() const {return full_command;}

        const std::vector<char> get_uart_instruction() const {return uart_instruction;}
        const std::vector<char> get_spi_address() const {return spi_address;}
        const std::vector<char> get_spi_write_data() const {return spi_write_data;}
        const unsigned short get_spi_reply_length() const {return spi_reply_length;}

    private:
        // SpaceWire-related data:
        char spw_instruction;
        std::vector<char> spw_address;
        std::vector<char> spw_write_data;
        unsigned short spw_reply_length;
        std::vector<char> full_command;

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
        
        System(std::string new_name, char new_hex);
        System(const System& other);
        System& operator=(const System& other);
};


class CommandDeck {
    public:
        std::vector<System> systems;

        // first key is hex code for system, second key is hex code for command. 
        std::unordered_map<char, std::unordered_map<char, Command>> commands;
        // more on this: will define command maps for each system (CDTE1, 2, etc; CMOS1, 2...) then key into each map by the hex code for each system

        // pass in a map pairing system text names ("CDTE") with json command file paths
        CommandDeck(std::unordered_map<std::string, std::string> named_paths);
        CommandDeck() = default;

        // add systems definition to object from JSON file
        void            add_systems(std::string sys_filename);
        // add commands to object from JSON file (MUST have added systems first!)
        void            add_commands(std::unordered_map<std::string, std::string> named_paths);

        // getter methods/conversion/convenience
        std::string     get_sys_name_for_code(char code);
        char            get_sys_code_for_name(std::string name);
        System&         get_sys_for_code(char code);
        System&         get_sys_for_name(std::string name);

        Command&        get_command_for_code(char code);
        Command&        get_command_for_name(std::string name);

        Command&        get_command_for_sys_for_code(char sys, char code);

        std::vector<char> get_command_bytes_for_sys_for_code(char sys, char code);
        // add a version of the above that takes arguments

        // print commands tree
        void            print();
    
    private:
        // track whether systems list has been added yet
        bool did_add_systems_;

};

#endif