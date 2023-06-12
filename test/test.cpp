#include "Commanding.h"
#include "LineInterface.h"
#include "Utilities.h"
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <string>
#include <iostream>

// namespace {

// // TEST(System, get_command_bytes_for_sys_for_code_old) {
    
// // }

// boost::asio::io_context context;

// std::string cmd_arg = " --config foxsi4-commands/systems.json";
// char* cmd_arg_c = cmd_arg.c_str();
// LineInterface lif = LineInterface(2, &cmd_arg_c, &context);

// }

int main(int argc, char** argv) {
    boost::asio::io_context context;

    LineInterface lif = LineInterface(argc, argv, context);
    
    std::cout << lif.local_address << "\n";
    std::cout << "\n\n";
    for(auto& ept: lif.unique_endpoints) {
        std::cout << ept.as_string() <<"\n";
    }
    for(auto& ept: lif.local_endpoints) {
        std::cout << ept.as_string() <<"\n";
    }
    for(auto& sys: lif.systems) {
        std::cout << sys.name << ",\t\t";
        hex_print(sys.hex);
        std::cout << ",\ttype: ";
        hex_print(static_cast<uint8_t>(sys.type));
        std::cout << "\n";
    }
    
    std::cout << "\n\nconstructing deck...\n";

    CommandDeck deck = CommandDeck(lif.systems, lif.lookup_command_file);

    deck.print();

    std::vector<uint8_t> cmd_old = deck.get_command_bytes_for_sys_for_code_old(0x08, 0x11);
    // should be set_de_setup_done, 0x3c 3c 01 00 04 04 04 04 3c 3c 3c 3c
    std::cout << "got old command:\t";
    hex_print(cmd_old);

    std::vector<uint8_t> cmd_new = deck.get_command_bytes_for_sys_for_code(0x08, 0x11);

    std::cout << "got new command:\t";
    hex_print(cmd_new);

    return 0;
}