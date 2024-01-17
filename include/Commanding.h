#ifndef COMMANDING_H
#define COMMANDING_H

#include "Parameters.h"
#include "Systems.h"
#include <unordered_map>
#include <vector>
#include <fstream>

// object to hold command metadata (TODO: add more attributes)
class Command {
    public:
        /* Public properties */
        std::string name;
        uint8_t hex;
        bool read;

        COMMAND_TYPE_OPTIONS type;
        
        /* Public methods */
        Command(std::string new_name, uint8_t new_hex, bool new_read, COMMAND_TYPE_OPTIONS new_type);
        Command(const Command& other);
        Command& operator=(const Command& other);
        Command();

        const std::string to_string();
        
        void set_type(COMMAND_TYPE_OPTIONS type);
        
        void set_spw_options(
            uint8_t new_spw_instruction,
            std::vector<uint8_t> new_spw_address,
            std::vector<uint8_t> new_spw_write_data,
            unsigned long new_spw_reply_length
        );

        void set_uart_options(std::vector<uint8_t> new_uart_instruction, unsigned long new_uart_reply_length);

        void set_eth_options(std::vector<uint8_t> new_eth_packet, size_t new_eth_reply_len);
        
        void set_spi_options(
            std::vector<uint8_t> new_spi_address,
            std::vector<uint8_t> new_spi_write_data,
            unsigned short new_spi_reply_length
        );
        
        std::vector<uint8_t> get_command_bytes();
        uint8_t* get_command_bytes_raw();
        std::vector<uint8_t> get_command_bytes(uint8_t arg);

        // getter methods for private variables
        const uint8_t get_spw_instruction() const {return spw_instruction;}
        const std::vector<uint8_t> get_spw_write_data() const {return spw_write_data;}
        const std::vector<uint8_t> get_spw_address()  const {return spw_address;}
        const unsigned long get_spw_reply_length() const {return spw_reply_length;}
        const std::vector<uint8_t> get_full_command() const {return full_command;}

        const std::vector<uint8_t> get_uart_instruction() const {return uart_instruction;}
        const unsigned long get_uart_reply_length() const {return uart_reply_length;}

        const unsigned long get_eth_reply_length() const {return eth_reply_length;}
        const std::vector<uint8_t> get_eth_packet() const {return eth_packet;}

        const std::vector<uint8_t> get_spi_address() const {return spi_address;}
        const std::vector<uint8_t> get_spi_write_data() const {return spi_write_data;}
        const unsigned short get_spi_reply_length() const {return spi_reply_length;}

    private:
        // SpaceWire-related data:
        uint8_t spw_instruction;
        std::vector<uint8_t> spw_address;
        std::vector<uint8_t> spw_write_data;
        unsigned long spw_reply_length;
        std::vector<uint8_t> full_command;

        // UART-related data:
        std::vector<uint8_t> uart_instruction;
        unsigned long uart_reply_length;

        // Ethernet-related data:
        std::vector<uint8_t> eth_packet;
        unsigned long eth_reply_length;

        // SPI-related data:
        std::vector<uint8_t> spi_address;
        std::vector<uint8_t> spi_write_data;
        unsigned short spi_reply_length;
};



class CommandDeck {
    public:
        std::vector<System> systems;

        // first key is hex code for system, second key is hex code for command. 
        std::unordered_map<uint8_t, std::unordered_map<uint8_t, Command>> commands;
        // more on this: will define command maps for each system (CDTE1, 2, etc; CMOS1, 2...) then key into each map by the hex code for each system

        // construct from a map pairing system text names ("CDTE") with json command file paths

        /**
         * @deprecated the old way
         * Construct a `CommandDeck` from a `std::map` of `System` text names and their command file paths.
         * @param named_paths 
         */
        CommandDeck(std::unordered_map<std::string, std::string> named_paths);
        // construct from a list of Systems and a map from System::hex to command file name

        /**
         * @brief Construct a new `CommandDeck` object from a list of `System`s, and a `std::map` of their hex codes onto command file paths.
         * 
         * @param new_systems 
         * @param command_paths 
         */
        CommandDeck(std::vector<System> new_systems, std::unordered_map<System, std::string> command_paths);
        CommandDeck() = default;

        // add systems definition to object from JSON file
        void            add_systems(std::string sys_filename);
        // add commands to object from JSON file (MUST have added systems first!)
        void            add_commands(std::unordered_map<std::string, std::string> named_paths);

        // getter methods/conversion/convenience
        std::string     get_sys_name_for_code(uint8_t code);
        uint8_t         get_sys_code_for_name(std::string name);
        System&         get_sys_for_code(uint8_t code);
        System&         get_sys_for_name(std::string name);

        Command&        get_command_for_sys_for_code(uint8_t sys, uint8_t code);

        // for the dirty work
        std::vector<uint8_t> make_spw_packet_for_sys_for_command(System sys, Command cmd);
        std::vector<uint8_t> make_eth_packet_for_sys_for_command(System sys, Command cmd);
        std::vector<uint8_t> make_uart_packet_for_sys_for_command(System sys, Command cmd);

        /**
         * @brief use to obtain full byte list to transmit to remote system as command.
        */
        std::vector<uint8_t> get_command_bytes_for_sys_for_code(uint8_t sys, uint8_t cmd);

        /**
         * @deprecated retained for debugging use and comparison.
        */
        std::vector<uint8_t> get_command_bytes_for_sys_for_code_old(uint8_t sys, uint8_t code);

        /**
         * @brief uses provided `sys`, `cmd` to create a template command, into which the specified memory address `addr` and read length `read_len` are inserted.
         * 
         * Calls `CommandDeck::get_command_bytes_for_sys_for_code(uint8_t sys, uint8_t cmd)` to 
         * provide template. Use case driving implementation: remote ring buffer read based on 
         * dynamic write pointer position. For that use case, advise to use ring buffer write pointer 
         * read command as template command.
        */
        std::vector<uint8_t> get_read_command_from_template(uint8_t sys, uint8_t cmd, std::vector<uint8_t> addr, size_t read_len);

        /**
         * @brief uses provided `sys` to header a read command accessing `read_len` of data at the specified `read_addr`.
         * 
         * @param sys the `System::hex` used to header the command.
         * @param addr the address to read at.
         * @param read_len the amount of data to read from the address.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_read_command_for_sys_at_address(uint8_t sys, std::vector<uint8_t> read_addr, size_t read_len);

        std::vector<uint8_t> get_read_command_for_sys_at_address_and_transaction_id(uint8_t sys, std::vector<uint8_t> read_addr, size_t read_len, uint16_t transaction_id);


        std::vector<uint8_t> get_spw_ether_header(std::vector<uint8_t> rmap_packet);

        std::vector<uint8_t> get_write_command_bytes_for_sys_for_HARDCODE(uint8_t sys, uint8_t code);
        std::vector<uint8_t> get_read_command_bytes_for_sys_for_HARDCODE(uint8_t sys, uint8_t code);
        

        // print commands tree
        void            print();
    
    private:
        // track whether systems list has been added yet
        bool did_add_systems_;
        std::vector<uint8_t> make_spw_header_(System sys, Command cmd);

};

#endif