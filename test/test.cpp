#include "Commanding.h"
#include "LineInterface.h"
#include "Utilities.h"
#include "RingBufferInterface.h"
// #include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <iomanip>

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
    // raw: 0x00 00 00 00 00 00 00 00 00 00 00 23 07 02 fe 01 7d 02 00 00 06 03 fe 00 00 00 00 00 00 00 00 00 0c 9a 3c 3c 01 00 04 04 04 04 3c 3c 3c 3c 45
    std::cout << "got old command:\t";
    hex_print(cmd_old);

    std::vector<uint8_t> cmd_new = deck.get_command_bytes_for_sys_for_code(0x08, 0x11);

    std::cout << "got new command:\t";
    hex_print(cmd_new);

    std::cout << "diff:\t\t\t0x";
    for(int i=0; i<cmd_new.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)((cmd_new[i] - cmd_old[i]) & 0xff) << " ";
    }
    std::cout << "\n";

    std::cout << "checking crcs...\n";
    std::vector<uint8_t> old_segment = std::vector<uint8_t>(cmd_old.begin() + 12, cmd_old.end() - 14);
    std::vector<uint8_t> new_segment = std::vector<uint8_t>(cmd_new.begin() + 12, cmd_new.end() - 14);
    uint8_t old_crc = spw_calculate_crc_uint_F(old_segment);
    uint8_t new_crc = spw_calculate_crc_uint_F(new_segment);

    std::cout << "old crc: ";
    hex_print(old_crc);
    std::cout << ", new crc: ";
    hex_print(new_crc);
    std::cout << "\n";

    std::vector<uint8_t> cmos2_train = deck.get_command_bytes_for_sys_for_code(0x0f, 0x1f);
    std::cout << "got cmos2 train command:\t";
    hex_print(cmos2_train);


    std::cout << "checking ring buffer interface:\n";
    RingBufferInterface rbf = RingBufferInterface(0x00400000, 32124400, 32780, 2);

    uint32_t last_read0 = rbf.read_block_from(0x00400000 + 32780*0);
    uint32_t last_read1 = rbf.read_block_from(0x00400000 + 32780*1);
    uint32_t last_read2 = rbf.read_block_from(0x00400000 + 32785);
    uint32_t last_read3 = rbf.read_block_from(0x00400000 + 32780*2);
    uint32_t last_read4 = rbf.read_block_from(0x00400000 + 32780*3);
    uint32_t last_read5 = rbf.read_block_from(0x00400000 + 32780*980);

    std::cout << last_read0 << std::endl;
    std::cout << last_read1 << std::endl;
    std::cout << last_read2 << std::endl;
    std::cout << last_read3 << std::endl;
    std::cout << last_read4 << std::endl;
    std::cout << last_read5 << std::endl;
    
    // RingBufferInterface rbf = RingBufferInterface();

    std::cout << "testing byte splat\n";

    uint64_t splat_val = 0x00400003200;
    std::vector bs = splat_to_nbytes(4,splat_val);
    for(auto& b: bs) {
        std::cout << (int)b << "\n";
    }

    return 0;
}