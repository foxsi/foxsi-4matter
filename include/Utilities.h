/**
 * @file Utilities.h
 * @author Thanasi Pantazides, Yixian Zhang
 * @brief Assorted utility functions.
 * @version v1.0.1
 * @date 2024-03-12
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#pragma once

#include "Parameters.h"
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

/**
 * @brief Increment `SUBSYSTEM_ORDER` enum to next value.
 */
SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order);
/**
 * @brief Increment `SUBSYSTEM_ORDER` enum to next value.
 */
SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order, int);
/**
 * @brief Increment `STATE_ORDER` enum to next value.
 */
STATE_ORDER operator++(STATE_ORDER& order);
/**
 * @brief Increment `STATE_ORDER` enum to next value.
 */
STATE_ORDER operator++(STATE_ORDER& order, int);

uint16_t operator|(errors::system l, uint16_t r);
uint16_t operator|(uint16_t l, errors::system r);
uint16_t& operator|=(uint16_t& l, errors::system r);
errors::system& operator|=(errors::system& l, uint16_t r);

uint16_t operator&(errors::system l, uint16_t r);
uint16_t operator&(uint16_t l, errors::system r);
uint16_t& operator&=(uint16_t& l, errors::system r);
errors::system& operator&=(errors::system& l, uint16_t r);
uint16_t operator~(errors::system err);

/**
 * @brief Namespace for SpaceWire-related utilities.
 */
namespace config::spw{
    /**
     * @brief Calculate draft F CRC for a SpaceWire subpacket.
     * @param data the SpaceWire data
     * @return char the CRC byte.
     */
    char spw_calculate_crc_F(std::vector<char>& data);
    /**
     * @brief Calculate draft F CRC for a SpaceWire subpacket.
     * @param data the SpaceWire data
     * @return uint8_t the CRC byte.
     */
    uint8_t spw_calculate_crc_uint_F(std::vector<uint8_t>& data);
    /**
     * @brief Check if a received SpaceWire RMAP packet is complete (done sending).
     * Checks that the packet length reported in the RMAP packet matches the observed size of the packet.
     * @param data the RMAP packet to check.
     * @return true if `data.size()` matches the length field in the RMAP packet.
     * @return false otherwise.
     */
    bool check_packet_complete(std::vector<uint8_t>& data);
}

/**
 * @brief Namespace for general purpose utilities.
 */
namespace utilities{
    /**
     * @brief Return a string with the current datetime.
     * Returns string in the format `auto_yyyy-mm-dd_hh:mm:ss`, where `auto_` is a prefix.
     * @return std::string with current datetime.
     */
    std::string get_now_string();
    
    /**
     * @brief Set up logging file for errors and debug info.
     * @param prefix a path to the folder the log file should be saved to.
     */
    void setup_logs_nowtime(std::string prefix);

    /**
     * @brief Write a `std::string` to log with type `[debug]`. 
     * @param msg the string to log.
     */
    void debug_log(std::string msg);
    /**
     * @brief Write a `std::string` to log with type `[info]`. 
     * @param msg the string to log.
     */
    void info_log(std::string msg);
    /**
     * @brief Write a `std::string` to log with type `[trace]`. 
     * @param msg the string to log.
     */
    void trace_log(std::string msg);
    /**
     * @brief Write a `std::string` to log with type `[error]`. 
     * @param msg the string to log.
     */
    void error_log(std::string msg);
    /**
     * @brief Write raw data to log with type `[debug]`. 
     * @param data the data to log.
     */
    void debug_log(std::vector<uint8_t> data);
    /**
     * @brief Write raw data to log with type `[info]`. 
     * @param data the data to log.
     */
    void info_log(std::vector<uint8_t> data);
    /**
     * @brief Write raw data to log with type `[trace]`. 
     * @param data the data to log.
     */
    void trace_log(std::vector<uint8_t> data);
    /**
     * @brief Write raw data to log with type `[error]`. 
     * @param data the data to log.
     */
    void error_log(std::vector<uint8_t> data);

    /**
     * @brief Print a `std::string` to stdout, colored blue (debug).
     * @param msg the string to print.
     */
    void debug_print(std::string msg);
    /**
     * @brief Print a `std::string` to stdout, colored red (error).
     * @param msg the string to print.
     */
    void error_print(std::string msg);
    /**
     * @brief Print a list of bytes to stdout.
     * @param data the bytes to print.
     */
    void hex_print(std::vector<uint8_t>& data);
    /**
     * @brief Print a list of bytes to stdout.
     * @param data the bytes to print.
     */
    void hex_print(std::vector<char>& data);
    /**
     * @brief Print a single byte to stdout.
     * @param data the byte to print.
     */
    void hex_print(uint8_t data);

    /**
     * @brief Convert a list of bytes to a hex string, includes `0x` prefix.
     * @param data the bytes to stringify.
     */
    std::string bytes_to_string(std::vector<uint8_t> data);

    /**
     * @brief Increment `i`, rolling over at `n`.
     * @return `i + 1 == n ? 0: i + 1`.
     */
    int inc_mod(int i, int n);

    /**
     * @brief Convert a hex string (with `0x` prefix) to a list of bytes.
     * @param hex_str the string to convert.
     * @return std::vector<uint8_t> the list of bytes.
     */
    std::vector<uint8_t> string_to_chars(std::string hex_str);
    /**
     * @brief Convert a string to a single byte.
     * Uses `strtol` to parse, may or may not be `0x` prefixed.
     * @param hex_str the string to convert.
     * @return a single byte.
     */
    uint8_t string_to_byte(std::string hex_str);

    /**
     * @brief Change the endianness of a list of 4 bytes.
     * Does nothing to `data[n]` for `n > 3`. Requires at least 4 bytes in `data`.
     * @param data the data to swap endianness for.
     * @return std::vector<uint8_t> the value `{data[3], data[2], data[1], data[0]}`.
     */
    std::vector<uint8_t> swap_endian4(std::vector<uint8_t> data);

    /**
     * @brief Convert a `uint64_t` (or smaller) to a list of bytes. 
     * Output will be little-endian.
     * @param n the number of bytes to extract from `data`.
     * @param data the source data. Must be at least `n` bytes wide.
     * @return std::vector<uint8_t> byte-by-byte breakdown of `data`.
     */
    std::vector<uint8_t> splat_to_nbytes(size_t n, uint64_t data);

    /**
     * @brief Convert `data` to a `uint32_t`.
     * Output will be little-endian. `data` must be 4 bytes or longer.
     * @param data byte list to convert.
     * @return single integer from MSB 4 bytes of `data`.
     */
    uint32_t unsplat_from_4bytes(std::vector<uint8_t> data);
}

#endif