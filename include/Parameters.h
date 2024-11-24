/**
 * @file Parameters.h
 * @author Yixian Zhang, Thanasi Pantazides
 * @brief Global constant values.
 * @version v1.0.1
 * @date 2024-03-11
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

/**
 * @brief Semantic versioning.
 */
static const unsigned short         MAJOR_VERSION   = 1;
static const unsigned short         MINOR_VERSION   = 2;
static const unsigned short         PATCH_VERSION   = 2;

/**
 * @brief Controls behavior of the `utilities::debug_print()` function.
 */
static bool DEBUG = true;

/**
 * @brief Static system configuration data.
 */
namespace config{
    /**
     * @deprecated Unused.
     */
    namespace timing{
        static const unsigned short         PERIOD          = 1;
    }

    /**
     * @deprecated Unused.
     */
    namespace spw{
        // SpaceWire
        static const unsigned short         SPACEWIRE_ADDRESS_LENGTH  = 4;
    }

    /**
     * @deprecated Unused.
     */
    namespace uart{
        static const unsigned short         UART_ADDRESS_LENGTH = 4;
    }

     /**
     * @deprecated Unused.
     */
    namespace spi{
        // SPI (forwarding to HK board)
        static const unsigned short         SPI_ADDRESS_LENGTH  = 4;
        static const unsigned short         SPI_INSTRUCTION_LENGTH = 4;
    }

    /**
     * @brief Fallback values for Ethernet connections if JSON import fails.
     */
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

    /**
     * @brief Initial values for I/O buffers in `TransportLayer.h`.
     */
    namespace buffer{
        // I/O
        static const unsigned long          RECV_BUFF_LEN   = 1024;
        static const unsigned long          SEND_BUFF_LEN   = 1024;
    }
}

namespace errors {
    enum class system: uint16_t {
        reading_packet      = 0x01 << 0,        // can't read some packet
        reading_frame       = 0x01 << 1,        // can't read whole frame
        reading_invalid     = 0x01 << 2,        // response from system failed some checks
        writing_invalid     = 0x01 << 3,        // response to write command is bad
        frame_packetizing   = 0x01 << 4,        // can't make packets from frame
        packet_framing      = 0x01 << 5,        // can't make frame from packet
        commanding          = 0x01 << 6,        // can't send a command (any reason)
        uplink_forwarding   = 0x01 << 7,        // can't send an uplink command (any reason)
        downlink_buffering  = 0x01 << 8,        // can't create DownlinkBufferElement for packet or queue it
        command_lookup      = 0x01 << 9,        // lookup command code in deck gets no result
        buffer_lookup       = 0x01 << 10,        // lookup ring_buffer_params gets no object
        spw_vcrc            = 0x01 << 11,       // SpaceWire CRC version is not `f`
        spw_length          = 0x01 << 12,       // SpaceWire message is too short to parse
    };
}

/**
 * @brief List of ring buffer commands that read ring buffer for detector systems.
 * The values are the second byte of an uplink command for the given system.
 */
enum class RING_READ_CMD: uint8_t {
    CDTE_1              = 0x8e,
    CDTE_2              = 0x8e,
    CDTE_3              = 0x8e,
    CDTE_4              = 0x8e,
    CMOS_1              = 0x8e,
    CMOS_2              = 0x8e
};

/**
 * @deprecated Not used.
 */
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

/**
 * @brief Used to track the state of `System`s in the main loop.
 * @warning This ought to be deprecated. It is used as part of the loop driver in `Circle`, but the actual `enum` value is never made use of. All the work just happens in `STATE_ORDER::CMD_SEND`.
 */
enum class STATE_ORDER: unsigned short {
    CMD_SEND            = 0x00,
    DATA_RECV           = 0x01,
    IDLE                = 0x02,
    STATE_COUNT         = 0x03
};

/**
 * @brief Specify the preferred communication interface for a `System` using this.
 * These values are used throughout the code to determine how to communicate for a given `System` object, which may have more than one valid `DataLinkLayer` communication interface available.
 */
enum class COMMAND_TYPE_OPTIONS: uint8_t {
    NONE                = 0x00, /*!< Use if there is no known command interface. */
    SPW                 = 0x01, /*!< Prefer a `SpaceWire` RMAP interface. */
    UART                = 0x02, /*!< Prefer a `UART` interface. */
    SPI                 = 0x03, /*!< Prefer a SPI interface @warning SPI is unimplemented. */
    ETHERNET            = 0x05  /*!< Prefer a `Ethernet` interface. */
};

/**
 * @brief Specify the type of data product in a buffer (see `Buffers.h`).
 * All of the following fields should store a value from this `enum`:
 *  * `DownlinkBufferElement::type` (via `DownlinkBufferElement::set_type`)
 *  * `RingBufferParameters::type`
 *  * `PacketFramer::type` (via `PacketFramer` constructor)
 *  * `FramePacketizer::type` (via `FramePacketizer` constructor)
 * 
 * @note While `SystemManager` may own interfaces to buffer objects with different types, you need to provide a `RING_BUFFER_TYPE_OPTIONS` key to lookup a specific buffer object for a `SystemManager`.
 */
enum class RING_BUFFER_TYPE_OPTIONS: uint8_t {
    PC                  = 0x00, /*!< Photon counting data. Used for CMOS and CdTe detectors. */
    QL                  = 0x01, /*!< Quick-look data. Used for CMOS detectors. */
    HK                  = 0x10, /*!< Housekeeping data. Used for CMOS and CdTe detectors, and CdTe DE. */
    TPX                 = 0x02, /*!< Timepix data. Used for Timepix detector. */
    POW                 = 0x11, /*!< Power (housekeeping) data. Used for dedicated Housekeeping board. */
    RTD                 = 0x12, /*!< Temperature sensor (housekeeping) data. Used for dedicated Housekeeping board. */
    INTRO               = 0x13, /*!< Software (housekeeping) data. Used for dedicated Housekeeping board. */
    PING                = 0x20, /*!< Formatter ping. Used to flag health of software system. */
    REPLY               = 0x30, /*!< Reply data. Used to indicate response to an uplink command. */
    NONE                = 0xff  /*!< No known data type. */
};

/**
 * @brief Map to convert `RING_BUFFER_TYPE_OPTIONS` into `std::string` labels (for printing etc).
 * Should be inverse map of `RING_BUFFER_TYPE_OPTIONS_INV_NAMES`.
 */
static const std::unordered_map<RING_BUFFER_TYPE_OPTIONS, std::string> RING_BUFFER_TYPE_OPTIONS_NAMES = {
    {RING_BUFFER_TYPE_OPTIONS::PC,      "pc"},
    {RING_BUFFER_TYPE_OPTIONS::QL,      "ql"},
    {RING_BUFFER_TYPE_OPTIONS::TPX,     "tpx"},
    {RING_BUFFER_TYPE_OPTIONS::HK,      "hk"},
    {RING_BUFFER_TYPE_OPTIONS::POW,     "pow"},
    {RING_BUFFER_TYPE_OPTIONS::RTD,     "rtd"},
    {RING_BUFFER_TYPE_OPTIONS::INTRO,   "intro"},
    {RING_BUFFER_TYPE_OPTIONS::REPLY,   "reply"},
    {RING_BUFFER_TYPE_OPTIONS::PING,    "ping"},
    {RING_BUFFER_TYPE_OPTIONS::NONE,    "none"}
};

/**
 * @brief Map to convert `std::string` labels into `RING_BFFER_TYPE_OPTIONS`.
 * Should be inverse map of `RING_BUFFER_TYPE_OPTIONS_NAMES`.
 */
static const std::unordered_map<std::string, RING_BUFFER_TYPE_OPTIONS> RING_BUFFER_TYPE_OPTIONS_INV_NAMES = {
    {"pc",      RING_BUFFER_TYPE_OPTIONS::PC},
    {"ql",      RING_BUFFER_TYPE_OPTIONS::QL},
    {"tpx",     RING_BUFFER_TYPE_OPTIONS::TPX},
    {"hk",      RING_BUFFER_TYPE_OPTIONS::HK},
    {"pow",     RING_BUFFER_TYPE_OPTIONS::POW},
    {"rtd",     RING_BUFFER_TYPE_OPTIONS::RTD},
    {"intro",   RING_BUFFER_TYPE_OPTIONS::INTRO},
    {"reply",   RING_BUFFER_TYPE_OPTIONS::REPLY},
    {"ping",    RING_BUFFER_TYPE_OPTIONS::PING},
    {"none",    RING_BUFFER_TYPE_OPTIONS::NONE},
};

/**
 * @brief Types of SpaceWire end-of-packet characters.
 * Used by the SPMU-001 Ethernet header when building the FPGA-side SpaceWire packet to send.
 */
enum class SPACEWIRE_END_OPTIONS: uint8_t {
    EOP                 = 0x00, /*!< Nominal end of packet. */
    EEP                 = 0x01, /*!< Error end of packet. */
    JUMBO               = 0x02  /*!< Indicate a multi-packet message, not sure how this is used. */
};

/**
 * @brief Milestones during the flight.
 * @note This is a member of `SystemManager`, but it is unused.
 */
enum class FLIGHT_STATE: uint8_t {
    AWAIT               = 0x00,     /*!< Waiting before launch. */
    PRELAUNCH           = 0x01,     /*!< In pre-launch count. */
    LAUNCH              = 0x02,     /*!< Vehicle has launched. */
    SHUTTER             = 0x03,     /*!< Shutter door has opened. */
    END                 = 0x04,     /*!< End of flight. */
    INVALID             = 0xff
};

/**
 * @brief States a given `SystemManager` can be in.
 * @note These are not intended to align one-to-one with state representation in the implemented remote system (e.g. there is no CdTe `LOOP` state).
 * 
 * Most of these states are not used currently in the software. Currently, `SYSTEM_STATE::AWAIT` and `SYSTEM_STATE::LOOP` are used in `Circle` to indicate nominal behavior, and `SYSTEM_STATE::DISCONNECT` and `SYSTEM_STATE::ABANDON` are used to track error states.
 */
enum class SYSTEM_STATE: uint8_t {
    OFF                 = 0x00,     /*!< System is off. */
    AWAIT               = 0x01,     /*!< System is on and waiting. */
    STARTUP             = 0x02,     /*!< System is starting up. */
    INIT                = 0x03,     /*!< System is initializing. */
    LOOP                = 0x04,     /*!< System is running main data collection loop. */
    END                 = 0x05,     /*!< System is preparing for power off. */
    DISCONNECT          = 0x06,     /*!< System is on but has communication disconnect. */
    ABANDON             = 0x07,     /*!< System is on but has been abandoned in the loop. */
    INVALID             = 0xff
};

#endif