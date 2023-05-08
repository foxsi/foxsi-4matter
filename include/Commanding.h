#ifndef COMMANDING_H
#define COMMANDING_H

#include "Parameters.h"
#include <boost/json.hpp>
#include <unordered_map>
#include <vector>
#include <fstream>

class Command {
    public:
        std::string name;
        char hex;
        bool read;
        
        Command(std::string name, char hex, bool read);
};

class System {
    public:
        std::string name;
        char hex;
        
        System(std::string name, char hex);
};


class SpaceWireCommand: Command {
    public:
        char instruction;
        char[SPACEWIRE_ADDRESS_LENGTH] address;
        char* arg;

        
        SpaceWireCommand(
            std::string name, 
            char hex, 
            bool read, 
            char instruction, 
            char[SPACEWIRE_ADDRESS_LENGTH] address, 
            char* arg
        );
};

// TODO: fix interface for UART once it's more defined by Timepix
class UARTCommand: Command {
    public:
        char[UART_ADDRESS_LENGTH] instruction;
        
        UARTCommand(
            std::string name, 
            char hex, 
            bool read,  
            char[UART_ADDRESS_LENGTH] instruction
        );
};

// TODO: fix interface for SPI once it's more defined by HK SPI peripherals
class SPICommand: Command {
    public:
        char[SPI_ADDRESS_LENGTH] address;
        char instruction;
        
        SPICommand(
            std::string name, 
            char hex, 
            bool read,  
            char[SPI_ADDRESS_LENGTH] instruction,
            char[instruction]
        );
};



class CommandDeck {
    public:

        std::vector<System> systems;

        // first key is hex code for system, second key is hex code for command. 
        std::unordered_map<char, std::unordered_map<char, Command*>> commands;
        // more on this: will define command maps for each system (CdTe 1, 2, etc; CMOS 1, 2...) then key into each map by the hex code for each system 

        // pass in a map pairing system text names ("CDTE") with json command file paths
        CommandDeck(std::unordered_map<std::string, std::string> named_paths);
        
        std::string     get_sys_name_for_code(char code);
        char            get_sys_code_for_name(std::string name);
        System&         get_sys_for_code(char code);
        System&         get_sys_for_name(std::string name);

        Command*        get_command_for_sys_for_cmd(char sys, char code);
    
    private:
        
};

#endif