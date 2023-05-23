#include "Commanding.h"
#include "LineInterface.h"
#include "Utilities.h"
#include <iostream>

int main(int argc, char* argv[]) {
    boost::asio::io_context context;
    LineInterface lif(argc, argv, context);

    CommandDeck deck = lif.get_command_deck();

    std::vector<uint8_t> hardcode = deck.get_write_command_bytes_for_sys_for_HARDCODE(1,1);
    int head_crci = 33;
    int ethernet_len = 12;
    uint8_t head_hardcode_crc = hardcode[head_crci - 1];

    std::vector<uint8_t> reference = {0x02, 0xfe, 0x01, 0x7d, 0x02, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x3a, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x00};
    
    uint8_t head_reference_crc = reference[head_crci - ethernet_len - 1];


    std::cout << "deck header crc:\t0x" << std::hex << (int)head_hardcode_crc << "\n";
    std::cout << "ref header crc:\t0x" << std::hex << (int)head_reference_crc << "\n";

    std::cout << "deck data crc:\t0x" << std::hex << (int)hardcode.back() << "\n";
    std::cout << "ref data crc:\t0x" << std::hex << (int)reference.back() << "\n";

    std::vector<char> cdtede_w_cmd;
    cdtede_w_cmd = deck.get_command_bytes_for_sys_for_code(0x08, 0x18);

    std::vector<char> cdtede_r_cmd;
    cdtede_r_cmd = deck.get_command_bytes_for_sys_for_code(0x08, 0x9a);

    std::cout << "ASIC params hex:\t";
    hex_print(cdtede_w_cmd);
    std::cout << "\n";

    std::cout << "Read mode hex:\t\t";
    hex_print(cdtede_r_cmd);
    std::cout << "\n";

    std::vector<char> rmap_part = {cdtede_w_cmd.begin()+12, cdtede_w_cmd.end()};
    std::vector<char> ether_head = deck.get_spw_ether_header(rmap_part);
    std::cout << "ether head: ";
    hex_print(ether_head);
    std::cout << "\n";
    std::cout << "original: ";
    hex_print(cdtede_w_cmd);
    std::cout << "\n";

    return 0;
}