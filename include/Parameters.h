#pragma once
#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>

// Versioning
static const unsigned short         MAJOR_VERSION   = 0;
static const unsigned short         MINOR_VERSION   = 0;
static const unsigned short         PATCH_VERSION   = 2;

// Debugging
static bool DEBUG = true;

// Timing
// DON'T CHANGE THIS WITHOUT EXTENSIVE TESTING
static const unsigned short         PERIOD          = 1;

// SpaceWire
static const unsigned short         SPACEWIRE_ADDRESS_LENGTH  = 4;

// UART
static const unsigned short         UART_ADDRESS_LENGTH = 4;

// SPI (forwarding to HK board)
static const unsigned short         SPI_ADDRESS_LENGTH  = 4;
static const unsigned short         SPI_INSTRUCTION_LENGTH = 4;

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

// I/O
static const unsigned long          RECV_BUFF_LEN   = 4096;
static const unsigned long          SEND_BUFF_LEN   = 4096;

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
enum class STATE_ORDER: unsigned short {
    CMD_SEND            = 0x00,
    DATA_REQ            = 0x01,
    DATA_RECV           = 0x02,
    DATA_CHECK          = 0x03,
    DATA_STORE          = 0x04,
    IDLE                = 0x05,
    STATE_COUNT         = 0x06
};

enum class COMMAND_TYPE_OPTIONS: uint8_t {
    NONE,
    SPW,
    UART,
    SPI
};

enum class SPACEWIRE_END_OPTIONS: uint8_t {
    EOP     = 0x00,
    EEP     = 0x01,
    JUMBO   = 0x02
};

#endif