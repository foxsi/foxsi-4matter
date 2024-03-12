/**
 * @file Commanding.h
 * @author Thanasi Pantazides
 * @brief Functionality for converting uplink commands to onboard serial format.
 * @version v1.0.1
 * @date 2024-03-12
 */

#ifndef COMMANDING_H
#define COMMANDING_H

#include "Parameters.h"
#include "Systems.h"
#include <unordered_map>
#include <vector>
#include <fstream>

/**
 * @brief Object to store constant command attributes, for transmitting to remote `System`s.
 * 
 * This object was designed before the `DataLinkLayer` types, which actually do inheritance correctly. This object just has a mush of `UART`, `Ethernet`, and `SpaceWire` interfaces and it is up to *you* to use the right one for your object! 
 * 
 * This object is intended to be used in conjunction with the `DataLinkLayer`-derived objects. `Command` objects define fields unique to each `Command` that will be sent, and `DataLinkLayer` objects define the underlying communication interface (which is constant for each separate `Command`).
 * 
 * @note `Command` objects can be defined manually, but they are intended to be populated using `CommandDeck` methods from source JSON configuration files.
 */
class Command {
    public:

        /**
         * @brief A unique (among `Command`s for a given `System`) name for this object.
         */
        std::string name;
        /**
         * @brief A unique (among `Command`s for a given `System`) ID for this object.
         */
        uint8_t hex;
        /**
         * @brief Identifies whether this is a read command.
         * 
         * `Command::read` is `true` if it is a read command, and `false` if it is a write command. This is a little deceptive for SpaceWire.
         * @note For SpaceWire commands, a write command (`Command::read == false`) may also receive a reply confirming the write operation, if the `reply` bit is set in the instruction field.
         */
        bool read;

        /**
         * @brief The communication medium used to send this command.
         */
        COMMAND_TYPE_OPTIONS type;
        
        /**
         * @brief Construct a generic `Command` object.
         * @param new_name the name for this `Command`.
         * @param new_hex the ID for this `Command`.
         * @param new_read indicate if this is a read (or write) `Command`.
         * @param new_type the communication medium used to send this `Command`.
         */
        Command(std::string new_name, uint8_t new_hex, bool new_read, COMMAND_TYPE_OPTIONS new_type);
        
        /**
         * @brief Copy-construct a `Command` object.
         * @param other the object to copy.
         */
        Command(const Command& other);
        Command& operator=(const Command& other);
        Command();

        const std::string to_string();
        
        /**
         * @brief Set communication medium used to send this object.
         * @param type the type communication interface to use.
         */
        void set_type(COMMAND_TYPE_OPTIONS type);
        
        /**
         * @brief Add SpaceWire RMAP fields to this `Command`. 
         * @note Even if `Command::read` is not set (a write command), the SpaceWire instruction field may still set a `reply` bit (to confirm the write command). Nominally, this causes the remote system to send a response.
         * @param new_spw_instruction RMAP instruction field.
         * @param new_spw_address remote memory address for the `Command`.
         * @param new_spw_write_data data to write to remote memory, if a write `Command`. Unused if a read `Command`.
         * @param new_spw_reply_length total length of RMAP reply (including preheader). Can be zero if no reply is expected.
         */
        void set_spw_options(
            uint8_t new_spw_instruction,
            std::vector<uint8_t> new_spw_address,
            std::vector<uint8_t> new_spw_write_data,
            unsigned long new_spw_reply_length
        );

        /**
         * @brief Add UART fields to this `Command`.
         * @note Now that I'm writing this, I realize this implementation for `UART` `Command`ing assumes the underlying port is configured for 8 data bits.
         * @param new_uart_instruction message to send to remote.
         * @param new_uart_reply_length the expected reply length from remote (in bytes).
         */
        void set_uart_options(std::vector<uint8_t> new_uart_instruction, unsigned long new_uart_reply_length);

        /**
         * @brief Add Ethernet fields to this `Command`.
         * @param new_eth_packet message to send to remote.
         * @param new_eth_reply_len the expected reply length from remote (in bytes). Describes payload size, not packet size.
         */
        void set_eth_options(std::vector<uint8_t> new_eth_packet, size_t new_eth_reply_len);
        
        void set_spi_options(
            std::vector<uint8_t> new_spi_address,
            std::vector<uint8_t> new_spi_write_data,
            unsigned short new_spi_reply_length
        );
        
        /**
         * @deprecated Never implemented.
         */
        std::vector<uint8_t> get_command_bytes();
        /**
         * @deprecated Never implemented.
         */
        uint8_t* get_command_bytes_raw();
        /**
         * @deprecated Never implemented.
         */
        std::vector<uint8_t> get_command_bytes(uint8_t arg);

        /**
         * @brief Checks the SpaceWire instruction byte to see if a reply is requested.
         * @return `true` if the `reply` bit is set.
         * @return `false` if the `reply` bit is not set.
         */
        bool check_spw_replies();

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

/**
 * @brief `CommandDeck` provides capability to lookup commands for each `System`, and synthesizes the raw command to transmit. 
 * This class supports the design for uplink commands:
 * 
 * ```
 *  [1B] System ID byte (unique for each System)
 *  [1B] Command ID byte (unique for each Command for a System)
 * ```
 * 
 * You can set this object up manually, but it is designed to be instantiated using JSON configuration files defining each `System`'s `Command` list.
 */
class CommandDeck {
    public:

        /**
         * @brief A list of all `System` objects that can be commanded.
         */
        std::vector<System> systems;

        /**
         * @brief A map storing all `Command`s for all `System`s.
         * 
         * To retrieve a `Command` object, you can do this:
         * ```C++
         *  Command cmd = commands[System::hex][Command::hex];
         * ```
         * if you know the `System::hex` and `Command::hex` values are correct. To safely get a `Command`, use `CommandDeck::get_command_for_sys_for_code()`, which will return a default null `Command` object if the lookup fails.
         */
        std::unordered_map<uint8_t, std::unordered_map<uint8_t, Command>> commands;

        /**
         * @deprecated Old constructor.
         * Construct a `CommandDeck` from a `std::map` of `System` text names and their command file paths.
         * @param named_paths 
         */
        CommandDeck(std::unordered_map<std::string, std::string> named_paths);
        // construct from a list of Systems and a map from System::hex to command file name

        /**
         * @brief Construct a new `CommandDeck` object from a list of `System`s, and map of JSON files.
         * 
         * The `std::vector<System>` should contain all `System`s that need commanding, and the `std::unordered_map` should contain `System::hex` keys for each provided `System`. The values in the map should be paths to JSON files. Each JSON file stores a list of command data for a given `System`.
         * 
         * See `foxsi4-commands` for the JSON setup.
         * 
         * The argument `new_systems` should not contain any duplicate entries.
         * 
         * @param new_systems all `System` objects that require commanding.
         * @param command_paths a map of each `System::hex` value into a path to a JSON command file for that system.
         */
        CommandDeck(std::vector<System> new_systems, std::unordered_map<System, std::string> command_paths);
        CommandDeck() = default;

        /**
         * @deprecated Use `CommandDeck(std::vector<System> new_systems, std::unordered_map<System, std::string> command_paths)` instead.
         */
        void            add_systems(std::string sys_filename);
        
        /**
         * @deprecated Use `CommandDeck(std::vector<System> new_systems, std::unordered_map<System, std::string> command_paths)` instead.
         */
        void            add_commands(std::unordered_map<std::string, std::string> named_paths);

        /**
         * @brief Get `System::name` given `System::hex`.
         */
        std::string     get_sys_name_for_code(uint8_t code);
        /**
         * @brief Get `System::hex` given `System::name`.
         */
        uint8_t         get_sys_code_for_name(std::string name);
        /**
         * @brief Get `System` given `System::hex`.
         */
        System&         get_sys_for_code(uint8_t code);
        /**
         * @brief Get `System` given `System::name`.
         */
        System&         get_sys_for_name(std::string name);
        /**
         * @brief Get `Command` given `System::hex` and the desired `Command`'s `::hex` value.
         */
        Command&        get_command_for_sys_for_code(uint8_t sys, uint8_t code);

        /**
         * @brief Assemble a SpaceWire RMAP packet for the provided `System` and `Command`.
         * @note `CommandDeck::get_command_bytes_for_sys_for_code()` does the same thing, but can be called generically (for any communication interface, not just SpaceWire).
         * @note This method will always prepend a 12 byte header to the RMAP packet, to be consumed by the SPMU-001.
         * @param sys the `System` to send the `Command` to.
         * @param cmd the `Command` to send. This function will take fields from both `System` and `Command` to build the raw packet to transmit.
         * @return std::vector<uint8_t> the raw SpaceWire RMAP packet to transmit.
         */
        std::vector<uint8_t> make_spw_packet_for_sys_for_command(System sys, Command cmd);
        /**
         * @brief Assemble an Ethernet payload for the provided `System` and `Command`.
         * @note `CommandDeck::get_command_bytes_for_sys_for_code()` does the same thing, but can be called generically (for any communication interface, not just Ethernet).
         * @param sys the `System` to send the `Command` to.
         * @param cmd the `Command` to send. This function will take fields from both `System` and `Command` to build the raw packet to transmit.
         * @return std::vector<uint8_t> the Ethernet payload to transmit.
         */
        std::vector<uint8_t> make_eth_packet_for_sys_for_command(System sys, Command cmd);
        /**
         * @brief Assemble an UART packet for the provided `System` and `Command`.
         * @note `CommandDeck::get_command_bytes_for_sys_for_code()` does the same thing, but can be called generically (for any communication interface, not just Ethernet).
         * @param sys the `System` to send the `Command` to.
         * @param cmd the `Command` to send. This function will take fields from both `System` and `Command` to build the raw packet to transmit.
         * @return std::vector<uint8_t> the UART packet to transmit.
         */
        std::vector<uint8_t> make_uart_packet_for_sys_for_command(System sys, Command cmd);

        /**
         * @brief use to obtain full byte list to transmit to remote system as command.
         * @param sys
         * @param cmd
         * @return std::vector<uint8_t>
        */
        std::vector<uint8_t> get_command_bytes_for_sys_for_code(uint8_t sys, uint8_t cmd);

        /**
         * @deprecated retained for debugging use and comparison.
        */
        std::vector<uint8_t> get_command_bytes_for_sys_for_code_old(uint8_t sys, uint8_t code);

        /**
         * @brief Uses provided `sys`, `cmd` to create a template command, into which the specified memory address `addr` and read length `read_len` are inserted.
         * 
         * Calls `CommandDeck::get_command_bytes_for_sys_for_code(uint8_t sys, uint8_t cmd)` to 
         * provide template. Use case driving implementation: remote ring buffer read based on 
         * dynamic write pointer position. For that use case, advise to use ring buffer write pointer 
         * read command as template command.
        */
        std::vector<uint8_t> get_read_command_from_template(uint8_t sys, uint8_t cmd, std::vector<uint8_t> addr, size_t read_len);

        /**
         * @brief Creates a read command accessing `read_len` bytes of data at the specified `read_addr`.
         * This is used along with `RingBufferParameters` and `PacketFramer` to access blocks of remote memory larger than the network MTU.
         * @param sys the `System::hex` used to header the command.
         * @param addr the address to read at.
         * @param read_len the amount of data to read from the address.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_read_command_for_sys_at_address(uint8_t sys, std::vector<uint8_t> read_addr, size_t read_len);
        /**
         * @brief Creates a read command with specified `transaction_id` accessing `read_len` bytes of data at the specified `read_addr`.
         * This is used along with `RingBufferParameters` and `PacketFramer` to access blocks of remote memory larger than the network MTU.
         * @param sys the `System::hex` used to header the command.
         * @param addr the address to read at.
         * @param read_len the amount of data to read from the address.
         * @param transaction_id the transaction ID to use.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_read_command_for_sys_at_address_and_transaction_id(uint8_t sys, std::vector<uint8_t> read_addr, size_t read_len, uint16_t transaction_id);

        /**
         * @brief Create the Ethernet header for SPMU-001 communication.
         * This is a 12 byte header prepended to every SpaceWire RMAP packet sent to the SPMU-001. It specifies the end-of-packet character and the total length of the SpaceWire part of the message.
         * @param rmap_packet the bare RMAP packet that will get the header.
         * @return Ethernet header prepended to `rmap_packet`.
         */
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