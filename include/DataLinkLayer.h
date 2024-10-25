/**
 * @file DataLinkLayer.h
 * @author Thanasi Pantazides
 * @brief Objects defining the OSI model's data link layer, for different protocols (Ethernet, SpaceWire, UART).
 * @version v1.0.1
 * @date 2024-03-08
 * 
 */
#ifndef DATALINKLAYER_H
#define DATALINKLAYER_H

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

/**
 * @brief Tracks the transport layer type of a `DataLinkLayer` object.
 */
enum class TransportLayerProtocol: uint8_t {
    NONE    = 0x00, /** no transport layer type */
    TCP     = 0x01, /** TCP data on the transport layer */
    UDP     = 0x02, /** UDP data on the transport layer */
};

/**
 * @brief Generic base class for a protocol definition iin the data link layer.
 * 
 * Describes packet framing, headers, footers, and maximum transmission unit (MTU). Used elsewhere in codebase by `SystemManager` to populate fields in `FramePacketizer` and `PacketFramer`. When not indicated otherwise, all units in class methods are bytes.
 * 
 * Crudely inspired by OSI network layers, see https://en.wikipedia.org/wiki/Data_link_layer.
 * 
 * Regarding the `initial_` vs `subsequent_` vs `static_` header sizes: `PacketFramer` infers how to get to the payload of a given packet using these fields according to this policy:
 * 
 * | Packet index | Bytes of header removed | Bytes of footer removed |
 * |--------------|-------------------------|-------------------------|
 * | `0`          | `initial_header_size + static_header_size` | `initial_footer_size + static_footer_size` |
 * | `1` and up   | `subsequent_header_size + static_header_size` | `subsequent_footer_size + static_footer_size` |
 * 
 */
class DataLinkLayer {
    public:
        /**
         * @brief Default constructor. 
         * Assumes a 1 byte frame without header or footers.
         */
        DataLinkLayer();
        /**
         * @brief Copy constructor.
         * @param other the object to copy.
         */
        DataLinkLayer(const DataLinkLayer& other);
        /**
         * @brief Simple constructor, which assumes only static-sized headers and footers.
         */
        DataLinkLayer(
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_static_footer_size
        );
        /**
         * @brief Fully detailed constructor, which can take dynamically-sized headers and footers.
         */
        DataLinkLayer(
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_initial_header_size,
            size_t new_subsequent_header_size,
            size_t new_static_footer_size,
            size_t new_initial_footer_size,
            size_t new_subsequent_footer_size
        );
        /**
         * @brief Copy-assignment operator.
         * 
         * @param other the object to copy.
         * @return DataLinkLayer
         */
        DataLinkLayer& operator=(const DataLinkLayer& other);

        /**
         * @brief The average speed of the link, in bits per second.
         */
        uint32_t mean_speed_bps;
        /**
         * @brief The maximum payload size allowed on the link, in bytes.
         * @note This is not the same as the maximum packet size on the link. You should account for header size when calculating this field.
         */
        size_t max_payload_size;
        /**
         * @brief The size of a data frame on this link.
         */
        size_t frame_size;
        /**
         * @brief The base header size of every packet in a frame, in bytes.
         * @note If there are different `initial_` or `subsequence_` header sizes, they are accounted for *on top* of the `static_header_size`. Reduce `static_header_size` as needed to accommodate.
         */
        size_t static_header_size;
        /**
         * @brief The header size of the first packet per frame, in bytes.
         * @note This is accounted for *on top* of the `static_header_size`. Reduce `static_header_size` as needed to accommodate.
         */
        size_t initial_header_size;
        /**
         * @brief The header size of all but the first packet per frame, in bytes.
         * @note This is accounted for *on top* of the `static_header_size`. Reduce `static_header_size` as needed to accommodate.
         */
        size_t subsequent_header_size;
        /**
         * @brief The base footer size of every packet on this link, in bytes.
         * @note If there are different `initial_` or `subsequence_` footer sizes, they are accounted for *on top* of the `static_footer_size`. Reduce `static_footer_size` as needed to accommodate.
         */
        size_t static_footer_size;
        /**
         * @brief The footer size of the first packet per frame, in bytes.
         * @note This is accounted for *on top* of the `static_footer_size`. Reduce `static_footer_size` as needed to accommodate.
         */
        size_t initial_footer_size;
        /**
         * @brief The footer size of all but the first packet per frame, in bytes.
         * @note This is accounted for *on top* of the `static_footer_size`. Reduce `static_footer_size` as needed to accommodate.
         */
        size_t subsequent_footer_size;
};

/**
 * @brief Description of an Ethernet interface with limited MTU. 
 * 
 * An address and port are associated. Does not do any actual communication, just provides configuration data for dynamic objects.
 */
class Ethernet: public DataLinkLayer {
    public:
        Ethernet();
        Ethernet(const Ethernet& other);
        Ethernet(const DataLinkLayer& link_layer);
        Ethernet(
            std::string new_address,
            uint16_t new_port,
            TransportLayerProtocol new_protocol,
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_static_footer_size
        );
        Ethernet(
            std::string new_address,
            uint16_t new_port,
            std::string new_protocol,
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_static_footer_size
        );
        Ethernet(
            std::string new_address,
            uint16_t new_port,
            TransportLayerProtocol new_protocol,
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_initial_header_size,
            size_t new_subsequent_header_size,
            size_t new_static_footer_size,
            size_t new_initial_footer_size,
            size_t new_subsequent_footer_size
        );
        Ethernet(
            std::string new_address,
            uint16_t new_port,
            std::string new_protocol,
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_initial_header_size,
            size_t new_subsequent_header_size,
            size_t new_static_footer_size,
            size_t new_initial_footer_size,
            size_t new_subsequent_footer_size
        );
        bool operator==(Ethernet& other);

        /**
         * @brief Check if this object describes the same Ethernet endpoint as `other`.
         * @return bool true if they are the same.
         */
        bool is_same_endpoint(Ethernet& other);
        std::string to_string();

        /**
         * @brief `std::string` representation of the IP address of this endpoint.
         */
        std::string address;
        /**
         * @brief Port number for this endpoint.
         */
        uint16_t port;
        /**
         * @brief Type of transport layer protocol used.
         */
        TransportLayerProtocol protocol;
};

/**
 * @brief Description of a SpaceWire interface.
 * 
 * Target and source logical and path addresses are provided. And `key` value. For a static network configuration, these should be runtime constants. Does not do any actual communication, just provides configuration data for dynamic objects.
 */
class SpaceWire: public DataLinkLayer {
    public:
        SpaceWire();
        SpaceWire(const SpaceWire& other);
        SpaceWire(const DataLinkLayer& link_layer);
        SpaceWire(
            std::vector<uint8_t> new_target_path_address,
            std::vector<uint8_t> new_reply_path_address,
            uint8_t new_target_logical_address,
            uint8_t new_source_logical_address,
            uint8_t new_key,
            char new_crc_version,
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_static_footer_size
        );
        SpaceWire(
            std::vector<uint8_t> new_target_path_address,
            std::vector<uint8_t> new_reply_path_address,
            uint8_t new_target_logical_address,
            uint8_t new_source_logical_address,
            uint8_t new_key,
            char new_crc_version,
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_initial_header_size,
            size_t new_subsequent_header_size,
            size_t new_static_footer_size,
            size_t new_initial_footer_size,
            size_t new_subsequent_footer_size
        );

        bool operator==(SpaceWire& other);

        /**
         * @brief Calculate CRC for the SpaceWire packet using "draft F" calculation from SpaceWire standard.
         * @note For write and reply packets, the header CRC should be calculated only on the header, excluding the target path address, and the data CRC should be calculated only on the data field.
         * @param data the packet data to CRC. 
         * @return uint8_t the CRC value.
         */
        uint8_t crc(std::vector<uint8_t> data);

        /**
         * @brief Extract the `data` field from a SpaceWire reply packet. 
         * @note assumes the source device is SPMU-001.
         * If the packet is malformed, will return empty.
         * @param spw_reply the complete SpaceWire packet 
         * @return std::vector<uint8_t> the `data` field from the reply.
         */
        std::vector<uint8_t> get_reply_data(std::vector<uint8_t> spw_reply);

        /**
         * @brief The target path address (remote end of this link).
         */
        std::vector<uint8_t> target_path_address;
        /**
         * @brief The reply path address (local end of this link, from remote).
         */
        std::vector<uint8_t> reply_path_address;
        /**
         * @brief The logical address of the remote node in the link.
         */
        uint8_t target_logical_address;
        /**
         * @brief The logical address of the local node in the link.
         */
        uint8_t source_logical_address;
        /**
         * @brief SpaceWire `key` field.
         */
        uint8_t key;
        /**
         * @brief The CRC draft version.
         * @warning only `f` is supported.
         */
        char crc_version;

    private:
        const uint8_t RMAPCRCTable[256] = {
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
        // void construct_crc();
};

/**
 * @brief A simple UART link configuration.
 * 
 * Port device (`tty` file), baud rate, number of data bits, stop bits, and parity can be provided.
 */
class UART: public DataLinkLayer {
    public:
        UART();
        UART(const UART& other);
        UART(const DataLinkLayer& link_layer);
        UART(
            std::string new_tty_path,
            uint32_t new_baud_rate,
            uint8_t new_parity,
            uint8_t new_stop_bits,
            uint8_t new_data_bits,
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_static_footer_size
        );
        UART(
            std::string new_tty_path,
            uint32_t new_baud_rate,
            uint8_t new_parity,
            uint8_t new_stop_bits,
            uint8_t new_data_bits,
            uint32_t new_mean_speed_bps,
            size_t new_max_payload_size,
            size_t new_frame_size,
            size_t new_static_header_size,
            size_t new_initial_header_size,
            size_t new_subsequent_header_size,
            size_t new_static_footer_size,
            size_t new_initial_footer_size,
            size_t new_subsequent_footer_size
        );

        bool operator==(UART& other);

        std::string to_string();

        /**
         * @brief Path to the port file (`tty*`) to use.
         */
        std::string tty_path;
        /**
         * @brief Baud rate for the link.
         */
        uint32_t baud_rate;
        /**
         * @brief Indicate no parity with `0`, odd parity with `1`, even parity with `2`.
         */
        uint8_t parity;
        /**
         * @brief Number of stop bits to use.
         */
        uint8_t stop_bits;
        /**
         * @brief Number of data bits to use.
         */
        uint8_t data_bits;
};

namespace utilities {
    /**
     * @brief Pretty-print SpaceWire RMAP packets for SPMU-001.
     * 
     * Provide a pointer to the `SpaceWire` interface object which transmitted the packet. This is needed for outgoing packets in order to scan the variable-length target path address. If a `nullptr` is provided for `spw`, the packet will scanned as a SpaceWire RMAP reply packet. Otherwise the instruction field will be inspected to determine read/write command.
     * 
     * @param data The byte vector to print.
     * @param spw A pointer to a `SpaceWire` interface that generated the packet.
    */
    void spw_print(std::vector<uint8_t> data, SpaceWire* spw);
};

#endif