#ifndef UTILITIES_H
#define UTILITIES_H

#include "Parameters.h"
#include <vector>

// deprecate these later if they are not used
SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order);
SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order, int);
STATE_ORDER operator++(STATE_ORDER& order);
STATE_ORDER operator++(STATE_ORDER& order, int);

// increment i mod n (loop through enum?)
int inc_mod(int i, int n);

void debug_print(std::string msg);
void hex_print(std::vector<uint8_t>& data);
void hex_print(std::vector<char>& data);

// find a better place to put this
uint8_t spw_calculate_crc_F(std::vector<char>& data);
uint8_t spw_calculate_crc_uint_F(std::vector<uint8_t>& data);

std::vector<char> string_to_chars(std::string hex_str);


#endif