#include "Utilities.h"

#include <spdlog/fmt/bin_to_hex.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>

SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order) {
    // order = static_cast<SUBSYSTEM_ORDER>((order + 1) % SUBSYSTEM_ORDER::SUBSYSTEM_COUNT);
    order = static_cast<SUBSYSTEM_ORDER>( static_cast<unsigned short>(order) + 1 );
    if(order == SUBSYSTEM_ORDER::SUBSYSTEM_COUNT) {
        order = static_cast<SUBSYSTEM_ORDER>(0);
    }
    return order;
}

SUBSYSTEM_ORDER operator++(SUBSYSTEM_ORDER& order, int) {
    SUBSYSTEM_ORDER result = order;
    ++order;
    return result;  
}

STATE_ORDER operator++(STATE_ORDER& order) {
    order = static_cast<STATE_ORDER>( static_cast<unsigned short>(order) + 1 );
    if(order == STATE_ORDER::STATE_COUNT) {
        order = static_cast<STATE_ORDER>(0);
    }
    return order;
}

STATE_ORDER operator++(STATE_ORDER& order, int) {
    STATE_ORDER result = order;
    ++order;
    return result;  
}

uint16_t operator|(errors::system l, uint16_t r) {
    return static_cast<uint16_t>(l)|r;
}

uint16_t operator|(uint16_t l, errors::system r) {
    return l|static_cast<uint16_t>(r);
}

uint16_t& operator|=(uint16_t& l, errors::system r) {
    return l = l|r;
}

errors::system& operator|=(errors::system& l, uint16_t r) {
    return l = static_cast<errors::system>(l|r);
}

uint16_t operator&(errors::system l, uint16_t r) {
    return static_cast<uint16_t>(l) & r;
}

uint16_t operator&(uint16_t l, errors::system r) {
    return l & static_cast<uint16_t>(r);
}

uint16_t& operator&=(uint16_t& l, errors::system r) {
    return l = l & r;
}

errors::system& operator&=(errors::system& l, uint16_t r) {
    return l = static_cast<errors::system>(l&r);
}

uint16_t operator~(errors::system err) {
    return ~static_cast<uint16_t>(err);
}


namespace config::spw{
    char spw_calculate_crc_F(std::vector<char>& data) {
        /* From Takayuki Yuasa SpaceWireRMAP library https://github.com/yuasatakayuki/SpaceWireRMAPLibrary, 
            * Calculates a CRC code for an array of bytes. */

        static const unsigned char RMAPCRCTable[] = { 
            0x00, 0x91, 0xe3, 0x72, 0x07, 0x96, 0xe4, 0x75,
            0x0e, 0x9f, 0xed, 0x7c, 0x09, 0x98, 0xea, 0x7b,
            0x1c, 0x8d, 0xff, 0x6e, 0x1b, 0x8a, 0xf8, 0x69,
            0x12, 0x83, 0xf1, 0x60, 0x15, 0x84, 0xf6, 0x67,
            0x38, 0xa9, 0xdb, 0x4a, 0x3f, 0xae, 0xdc, 0x4d,
            0x36, 0xa7, 0xd5, 0x44, 0x31, 0xa0, 0xd2, 0x43,
            0x24, 0xb5, 0xc7, 0x56, 0x23, 0xb2, 0xc0, 0x51,
            0x2a, 0xbb, 0xc9, 0x58, 0x2d, 0xbc, 0xce, 0x5f,
            0x70, 0xe1, 0x93, 0x02, 0x77, 0xe6, 0x94, 0x05,
            0x7e, 0xef, 0x9d, 0x0c, 0x79, 0xe8, 0x9a, 0x0b,
            0x6c, 0xfd, 0x8f, 0x1e, 0x6b, 0xfa, 0x88, 0x19,
            0x62, 0xf3, 0x81, 0x10, 0x65, 0xf4, 0x86, 0x17,
            0x48, 0xd9, 0xab, 0x3a, 0x4f, 0xde, 0xac, 0x3d,
            0x46, 0xd7, 0xa5, 0x34, 0x41, 0xd0, 0xa2, 0x33,
            0x54, 0xc5, 0xb7, 0x26, 0x53, 0xc2, 0xb0, 0x21,
            0x5a, 0xcb, 0xb9, 0x28, 0x5d, 0xcc, 0xbe, 0x2f,
            0xe0, 0x71, 0x03, 0x92, 0xe7, 0x76, 0x04, 0x95,
            0xee, 0x7f, 0x0d, 0x9c, 0xe9, 0x78, 0x0a, 0x9b,
            0xfc, 0x6d, 0x1f, 0x8e, 0xfb, 0x6a, 0x18, 0x89,
            0xf2, 0x63, 0x11, 0x80, 0xf5, 0x64, 0x16, 0x87,
            0xd8, 0x49, 0x3b, 0xaa, 0xdf, 0x4e, 0x3c, 0xad,
            0xd6, 0x47, 0x35, 0xa4, 0xd1, 0x40, 0x32, 0xa3,
            0xc4, 0x55, 0x27, 0xb6, 0xc3, 0x52, 0x20, 0xb1,
            0xca, 0x5b, 0x29, 0xb8, 0xcd, 0x5c, 0x2e, 0xbf,
            0x90, 0x01, 0x73, 0xe2, 0x97, 0x06, 0x74, 0xe5,
            0x9e, 0x0f, 0x7d, 0xec, 0x99, 0x08, 0x7a, 0xeb,
            0x8c, 0x1d, 0x6f, 0xfe, 0x8b, 0x1a, 0x68, 0xf9,
            0x82, 0x13, 0x61, 0xf0, 0x85, 0x14, 0x66, 0xf7,
            0xa8, 0x39, 0x4b, 0xda, 0xaf, 0x3e, 0x4c, 0xdd,
            0xa6, 0x37, 0x45, 0xd4, 0xa1, 0x30, 0x42, 0xd3,
            0xb4, 0x25, 0x57, 0xc6, 0xb3, 0x22, 0x50, 0xc1,
            0xba, 0x2b, 0x59, 0xc8, 0xbd, 0x2c, 0x5e, 0xcf
        };

        char crc = 0x00;
        size_t length=data.size();
        for (size_t i = 0; i < length; i++) {
            crc = RMAPCRCTable[(crc ^ data[i]) & 0xff];
        }
        return crc;
    }

    uint8_t spw_calculate_crc_uint_F(std::vector<uint8_t>& data) {
        /* From Takayuki Yuasa SpaceWireRMAP library https://github.com/yuasatakayuki/SpaceWireRMAPLibrary, 
            * Calculates a CRC code for an array of bytes. */

        static const uint8_t RMAPCRCTable[] = { 
            0x00, 0x91, 0xe3, 0x72, 0x07, 0x96, 0xe4, 0x75,
            0x0e, 0x9f, 0xed, 0x7c, 0x09, 0x98, 0xea, 0x7b,
            0x1c, 0x8d, 0xff, 0x6e, 0x1b, 0x8a, 0xf8, 0x69,
            0x12, 0x83, 0xf1, 0x60, 0x15, 0x84, 0xf6, 0x67,
            0x38, 0xa9, 0xdb, 0x4a, 0x3f, 0xae, 0xdc, 0x4d,
            0x36, 0xa7, 0xd5, 0x44, 0x31, 0xa0, 0xd2, 0x43,
            0x24, 0xb5, 0xc7, 0x56, 0x23, 0xb2, 0xc0, 0x51,
            0x2a, 0xbb, 0xc9, 0x58, 0x2d, 0xbc, 0xce, 0x5f,
            0x70, 0xe1, 0x93, 0x02, 0x77, 0xe6, 0x94, 0x05,
            0x7e, 0xef, 0x9d, 0x0c, 0x79, 0xe8, 0x9a, 0x0b,
            0x6c, 0xfd, 0x8f, 0x1e, 0x6b, 0xfa, 0x88, 0x19,
            0x62, 0xf3, 0x81, 0x10, 0x65, 0xf4, 0x86, 0x17,
            0x48, 0xd9, 0xab, 0x3a, 0x4f, 0xde, 0xac, 0x3d,
            0x46, 0xd7, 0xa5, 0x34, 0x41, 0xd0, 0xa2, 0x33,
            0x54, 0xc5, 0xb7, 0x26, 0x53, 0xc2, 0xb0, 0x21,
            0x5a, 0xcb, 0xb9, 0x28, 0x5d, 0xcc, 0xbe, 0x2f,
            0xe0, 0x71, 0x03, 0x92, 0xe7, 0x76, 0x04, 0x95,
            0xee, 0x7f, 0x0d, 0x9c, 0xe9, 0x78, 0x0a, 0x9b,
            0xfc, 0x6d, 0x1f, 0x8e, 0xfb, 0x6a, 0x18, 0x89,
            0xf2, 0x63, 0x11, 0x80, 0xf5, 0x64, 0x16, 0x87,
            0xd8, 0x49, 0x3b, 0xaa, 0xdf, 0x4e, 0x3c, 0xad,
            0xd6, 0x47, 0x35, 0xa4, 0xd1, 0x40, 0x32, 0xa3,
            0xc4, 0x55, 0x27, 0xb6, 0xc3, 0x52, 0x20, 0xb1,
            0xca, 0x5b, 0x29, 0xb8, 0xcd, 0x5c, 0x2e, 0xbf,
            0x90, 0x01, 0x73, 0xe2, 0x97, 0x06, 0x74, 0xe5,
            0x9e, 0x0f, 0x7d, 0xec, 0x99, 0x08, 0x7a, 0xeb,
            0x8c, 0x1d, 0x6f, 0xfe, 0x8b, 0x1a, 0x68, 0xf9,
            0x82, 0x13, 0x61, 0xf0, 0x85, 0x14, 0x66, 0xf7,
            0xa8, 0x39, 0x4b, 0xda, 0xaf, 0x3e, 0x4c, 0xdd,
            0xa6, 0x37, 0x45, 0xd4, 0xa1, 0x30, 0x42, 0xd3,
            0xb4, 0x25, 0x57, 0xc6, 0xb3, 0x22, 0x50, 0xc1,
            0xba, 0x2b, 0x59, 0xc8, 0xbd, 0x2c, 0x5e, 0xcf
        };

        uint8_t crc = 0x00;
        size_t length=data.size();
        for (size_t i = 0; i < length; i++) {
            crc = RMAPCRCTable[(crc ^ data[i]) & 0xff];
        }
        return crc;
    }

    bool check_packet_complete(std::vector<uint8_t> &data) {
        if (data.size() < 12) {
            return false;
        }

        uint64_t length_remain = utilities::unsplat_from_4bytes(std::vector<uint8_t>(data.begin() + 8, data.begin() + 12));
        std::vector<uint8_t> remain(data.begin() + 12, data.end());

        if (length_remain < remain.size()) {
            return false;
        } else {
            return true;
        }
    }
}

namespace utilities{
    std::string get_now_string() {
        char time_format[std::size("auto_yyyy-mm-dd_hh:mm:ss")];
        auto start_time = std::time({});

        std::strftime(std::data(time_format), std::size(time_format), "auto_%F_%T", std::gmtime(&start_time));

        return std::string(time_format);
    }

    // add spdlog setup files function here (filename with "log-today-time.log")
    // add spdlog write function (error, debug, info) here
    
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<spdlog::logger> prelogger;
    void setup_logs_nowtime(std::string prefix)
    {
        // char time_fmt[std::size("auto_yyyy-mm-dd_hh:mm:ss")];
        // auto start_time = std::time({});

        // std::strftime(std::data(time_fmt), std::size(time_fmt), "auto_%F_%T", std::gmtime(&start_time));

        std::string time_fmt = get_now_string();

        std::string file_name = prefix + std::string(time_fmt);
        std::string prefile_name = prefix + std::string(time_fmt) + "_pre";
        std::string extension = ".log";
        debug_print("log file: " + file_name + extension + "\n");
        
        auto logger_temp = spdlog::basic_logger_mt<spdlog::async_factory>(file_name, file_name + extension);
        auto prelogger_temp = spdlog::basic_logger_mt<spdlog::async_factory>(prefile_name, prefile_name + extension);
        logger_temp->set_level(spdlog::level::trace);
        prelogger_temp->set_level(spdlog::level::trace);
        logger = spdlog::get(file_name);
        prelogger = spdlog::get(prefile_name);
    }

    void debug_log(std::string msg) {
        utilities::logger->debug(msg);
    }
    void info_log(std::string msg) {
        utilities::logger->info(msg);
    }
    void trace_log(std::string msg) {
        utilities::logger->trace(msg);
    }
    void trace_prelog(std::string msg) {
        utilities::prelogger->trace(msg);
    }
    void error_log(std::string msg) {
        utilities::logger->error(msg);
    }
    void debug_log(std::vector<uint8_t> data) {
        utilities::logger->debug("{:spn}", spdlog::to_hex(data));
    }
    void info_log(std::vector<uint8_t> data) {
        utilities::logger->info("{:spn}", spdlog::to_hex(data));
    }
    void trace_log(std::vector<uint8_t> data) {
        utilities::logger->trace("{:spn}", spdlog::to_hex(data));
    }
    void trace_prelog(std::vector<uint8_t> data) {
        utilities::prelogger->trace("{:spn}", spdlog::to_hex(data));
    }
    void error_log(std::vector<uint8_t> data) {
        utilities::logger->error("{:spn}", spdlog::to_hex(data));
    }


    void debug_print(std::string msg) {
        if(DEBUG) {
            std::cout << "\033[1;34m" << msg << "\033[0m";
        }
    }

    void error_print(std::string msg) {
        std::cout << "\033[1;31m" << msg << "\033[0m";
    }

    void hex_print(std::vector<uint8_t>& data) {
        std::cout << "0x";
        for(auto& c: data) {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)(c & 0xff) << " ";
        }
        std::cout << "\n";
    }

    void hex_print(std::vector<char>& data) {
        std::cout << "0x";
        for(auto& c: data) {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)(c & 0xff) << " ";
        }
        std::cout << "\n";
    }

    void hex_print(uint8_t data) {
        if(DEBUG) {
            std::cout << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int)(data & 0xff);
        }
    }

    std::string bytes_to_string(std::vector<uint8_t> data) {
        std::stringstream result;
        result << "0x";
        for (uint8_t byte: data) {
            result << std::setw(2) << std::setfill('0') << std::hex << byte + 0;
        }
        return result.str();
    }

    int inc_mod(int i, int n) {
        return i = (i + 1 == n ? 0: i + 1);
    }

    std::vector<uint8_t> string_to_chars(std::string hex_str) {
        // will slice 0x or 0X prefix off hex string

        std::vector<uint8_t> bytes;
        // prepend to bytes to make it even-length
        
        if(hex_str.substr(0,2).compare("0x") == 0 || hex_str.substr(0,2).compare("0X") == 0) {
            hex_str.erase(0,2);
        }
        if((hex_str.length() % 2) != 0) {
            hex_str.insert(0, "0");
        }

        for (unsigned int i = 0; i < hex_str.length(); i += 2) {
            std::string byte_str = hex_str.substr(i, 2);
            uint8_t byte = (uint8_t) strtol(byte_str.c_str(), NULL, 16);
            bytes.push_back(byte);
        }
        return bytes;
    }

    uint8_t string_to_byte(std::string hex_str) {
        return strtol(hex_str.c_str(), NULL, 16) & 0xff;
    }

    std::vector<uint8_t> swap_endian4(std::vector<uint8_t> data) {
        std::vector<uint8_t> out;
        out.push_back(data.at(3));
        out.push_back(data.at(2));
        out.push_back(data.at(1));
        out.push_back(data.at(0));

        return out;
    }

    std::vector<uint8_t> splat_to_nbytes(size_t n, uint64_t data) {
        std::vector<uint8_t> result;
        for(int i=n-1; i>=0; --i) {
            const uint8_t d = (data >> i*8) & 0xff;
            result.push_back(d);
        }
        return result;
    }

    uint32_t unsplat_from_4bytes(std::vector<uint8_t> data) {
        uint32_t result = 0;
        if (data.size() < 4) {
            // error_print("data too short to unsplat 4 bytes!\n");
            error_log("utilities::unsplat_from_4bytes()\tdata too short to unsplat 4 bytes.");
            return 0;
        }
        for(int i=0; i < 4; ++i) {
            result |= (data.at(i) << (3-i)*8);
        }
        return result;
    }
}