#ifndef UTILITIES_H
#define UTILITIES_H

#include "Parameters.h"
#include <vector>

// deprecate these later if they are not used
SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order);
SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order, int);
STATE_ORDER operator++(STATE_ORDER& order);
STATE_ORDER operator++(STATE_ORDER& order, int);

namespace config::spw{
    // more Spacewire stuff
    char spw_calculate_crc_F(std::vector<char>& data);
    uint8_t spw_calculate_crc_uint_F(std::vector<uint8_t>& data);
}

namespace utilities{
    // increment i mod n (loop through enum?)
    int inc_mod(int i, int n);

    void debug_print(std::string msg);
    void error_print(std::string msg);
    void hex_print(std::vector<uint8_t>& data);
    void hex_print(std::vector<char>& data);
    void hex_print(uint8_t data);

    std::vector<uint8_t> string_to_chars(std::string hex_str);
    uint8_t string_to_byte(std::string hex_str);

    // to switch endianness of a 4byte vector
    std::vector<uint8_t> swap_endian4(std::vector<uint8_t> data);

    // to extract single value to n-byte vector:
    std::vector<uint8_t> splat_to_nbytes(size_t n, uint64_t data);

    // convert 4 bytes to uint32_t
    uint32_t unsplat_from_4bytes(std::vector<uint8_t> data);
}

#endif