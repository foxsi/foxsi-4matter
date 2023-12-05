#ifndef PARAMETERS_H
#define PARAMETERS_H

#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

// Versioning
static const unsigned short         MAJOR_VERSION   = 0;
static const unsigned short         MINOR_VERSION   = 0;
static const unsigned short         PATCH_VERSION   = 4;

// Debugging
static bool DEBUG = true;

namespace config{
    namespace timing{
        // Timing
        // DON'T CHANGE THIS WITHOUT EXTENSIVE TESTING
        static const unsigned short         PERIOD          = 1;
    }

    namespace spw{
        // SpaceWire
        static const unsigned short         SPACEWIRE_ADDRESS_LENGTH  = 4;
    }

    namespace uart{
        // UART
        static const unsigned short         UART_ADDRESS_LENGTH = 4;
    }

    namespace spi{
        // SPI (forwarding to HK board)
        static const unsigned short         SPI_ADDRESS_LENGTH  = 4;
        static const unsigned short         SPI_INSTRUCTION_LENGTH = 4;
    }

    namespace ethernet{
        // IP addresses
        static std::string                  LOOPBACK_IP     = "127.0.0.1";

        static std::string                  LOCAL_IP        = "192.168.1.8";
        static const unsigned short         LOCAL_PORT      = 9999;

        static std::string                  GSE_IP          = "192.168.1.100";
        static const unsigned short         GSE_PORT        = 9999;

        static const std::string            TLM_IP          = "127.0.0.1";
        static const unsigned short         TLM_PORT        = 0;

        static const std::string            SPMU_IP         = "127.0.0.1";
        static const unsigned short         SPMU_PORT       = 0;

        static const std::string            PLENUM_IP       = "127.0.0.1";
        static const unsigned short         PLENUM_PORT     = 0;
    }

    namespace buffer{
        // I/O
        static const unsigned long          RECV_BUFF_LEN   = 1024;
        static const unsigned long          SEND_BUFF_LEN   = 1024;
    }
}

enum class RING_READ_CMD: uint8_t {
    CDTE_1              = 0x8e,
    CDTE_2              = 0x8e,
    CDTE_3              = 0x8e,
    CDTE_4              = 0x8e,
    CMOS_1              = 0x8e,
    CMOS_2              = 0x8e
};

// loop order for subsystems:
enum class SUBSYSTEM_ORDER: unsigned short {
    HOUSEKEEPING        = 0x00,
    CDTE_1              = 0x01,
    CMOS_1              = 0x02,
    CDTE_2              = 0x03,
    TIMEPIX             = 0x04,
    CDTE_3              = 0x05,
    CMOS_2              = 0x06,
    CDTE_4              = 0x07,
    SUBSYSTEM_COUNT     = 0x08
};

// loop order for states:
// enum class STATE_ORDER: unsigned short {
//     CMD_SEND            = 0x00,
//     DATA_REQ            = 0x01,
//     DATA_RECV           = 0x02,
//     DATA_CHECK          = 0x03,
//     DATA_STORE          = 0x04,
//     IDLE                = 0x05,
//     STATE_COUNT         = 0x06
// };
enum class STATE_ORDER: unsigned short {
    CMD_SEND            = 0x00,
    DATA_RECV           = 0x01,
    IDLE                = 0x02,
    STATE_COUNT         = 0x03
};

enum class COMMAND_TYPE_OPTIONS: uint8_t {
    NONE                = 0x00,
    SPW                 = 0x01,
    UART                = 0x02,
    SPI                 = 0x03,
    ETHERNET            = 0x05
};

enum class RING_BUFFER_TYPE_OPTIONS: uint8_t {
    PC                  = 0x00,
    QL                  = 0x01,
    NONE                = 0xff
};

static const std::unordered_map<RING_BUFFER_TYPE_OPTIONS, std::string> RING_BUFFER_TYPE_OPTIONS_NAMES = {
    {RING_BUFFER_TYPE_OPTIONS::PC,      "pc"},
    {RING_BUFFER_TYPE_OPTIONS::QL,      "ql"},
    {RING_BUFFER_TYPE_OPTIONS::NONE,    "none"},
};

enum class SPACEWIRE_END_OPTIONS: uint8_t {
    EOP                 = 0x00,
    EEP                 = 0x01,
    JUMBO               = 0x02
};

enum class FLIGHT_STATE: uint8_t {
    AWAIT               = 0x00,
    PRELAUNCH           = 0x01,
    LAUNCH              = 0x02,
    SHUTTER             = 0x03,
    END                 = 0x04,
    INVALID             = 0xff
};

enum class SYSTEM_STATE: uint8_t {
    OFF                 = 0x00,
    AWAIT               = 0x01,
    STARTUP             = 0x02,
    INIT                = 0x03,
    LOOP                = 0x04,
    END                 = 0x05,
    DISCONNECT          = 0x06,
    ABANDON             = 0x07,
    INVALID             = 0xff
};

#endif