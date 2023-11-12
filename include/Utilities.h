#ifndef UTILITIES_H
#define UTILITIES_H

#pragma once

#include "Parameters.h"
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

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

    std::string get_now_string();
    // std::shared_ptr<spdlog::logger> debug_logger;
    // possibly group here
    void setup_logs_nowtime(std::string prefix);

    void debug_log(std::string msg);
    void info_log(std::string msg);
    void trace_log(std::string msg);
    void trace_prelog(std::string msg);
    void error_log(std::string msg);
    void debug_log(std::vector<uint8_t> data);
    void info_log(std::vector<uint8_t> data);
    void trace_log(std::vector<uint8_t> data);
    void trace_prelog(std::vector<uint8_t> data);
    void error_log(std::vector<uint8_t> data);

    void debug_print(std::string msg);
    void error_print(std::string msg);
    void hex_print(std::vector<uint8_t>& data);
    void hex_print(std::vector<char>& data);
    void hex_print(uint8_t data);
    // through here in a new namespace `log`


    // increment i mod n (loop through enum?)
    int inc_mod(int i, int n);

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