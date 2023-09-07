#pragma once
#ifndef BUFFERS_H
#define BUFFERS_H

#include "Commanding.h"

#include <queue>

/**
 * @brief A data element in the downlink queue.
 * 
 * This object stores all the information needed to build a downlink packet in the format: `<source system [1B]><total number of packets in this frame [2B]><index of this packet in frame [2B]><0x00><0x00><0x00><payload [max_packet_size - 7B]>`. This object should be stored in the global downlink queue and removed and serialized prior to sending.
 */
class DownlinkBufferElement {
    public:
        DownlinkBufferElement(System& new_system, size_t max_packet_size);
        DownlinkBufferElement(const DownlinkBufferElement& other);

        /**
         * @brief Get byte string to send over physical interface to the ground.
         * This method prepends a header (`DownlinkBufferElement::get_header()`) to `DownlinkBufferElement::payload` and returns the total list of bytes to transmit.
         * @return std::vector<uint8_t> The byte stream to transmit.
         */
        std::vector<uint8_t> get_packet();

        const System get_system() const {return system;};
        const uint8_t get_system_hex() const {return system.hex;};
        const std::vector<uint8_t> get_payload() const {return payload;};
        const size_t get_max_packet_size() const {return max_packet_size;};
        const uint16_t get_num_packets_in_frame() const {return num_packets_in_frame;};
        const uint16_t get_this_packet_index() const {return this_packet_index;};
        
        void set_system(System& new_system);
        void set_payload(std::vector<uint8_t> new_payload);
        void set_max_packet_size(size_t new_max_packet_size);
        void set_num_packets_in_frame(uint16_t new_num_packets_in_frame);
        void set_this_packet_index(uint16_t new_this_packet_index);

    private:

        /**
         * @brief Check if the provided `new_payload` data would fit inside a packet of size `DownlinkBufferElement::max_packet_size`. 
         * 
         * Assumes eight additional bytes will be prepended to `new_payload` in a header.
         * 
         * @param new_payload The data payload to check length for.
         * @return true If `new_payload` will fit in packet.
         * @return false 
         */
        bool check_payload_fits(std::vector<uint8_t> new_payload);
        
        /**
         * @brief Get the header object for the downlink packet.
         * Downlink data is `<header><payload>`, where `<header>` comprises `<system id><num_packets_in_frame><this_packet_index><0x00><0x00><0x00>`.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_header();

        /**
         * @brief The maximum size of one packet in the downlink queue. 
         * This is related to the MTU of the downlink interface.
         */
        size_t max_packet_size;
        /**
         * @brief The packet to place in the downlink queue.
         */
        std::vector<uint8_t> payload;

        /**
         * @brief The onboard system which produced the data in the `payload`.
         */
        System& system;
        /**
         * @brief The total number of transmit packets in this data frame. 
         * This will be used on the ground to assemble one parse-able data frame.
         */
        uint16_t num_packets_in_frame;
        /**
         * @brief The index of this packet in the overall frame.
         * This will be used on the ground to assemble one data frame.
         */
        uint16_t this_packet_index;
};



/**
 * @brief A data element in the uplink command queue for a specific `System`.
 * 
 * This object stores the `System` being targeted, the `Command` to send to that system, and any optional arguments uplinked as well. This object should be stored in a queue specific to one `System`, and removed for use in a lookup in `CommandDeck`.
 */
class UplinkBufferElement {
    public:
        UplinkBufferElement(System& system, Command& command, std::vector<uint8_t> varargs);
        UplinkBufferElement(std::vector<uint8_t> raw_data, CommandDeck& deck);

        const System& get_system() const {return system;};
        const Command& get_command() const {return command;};
        const std::vector<uint8_t> get_varargs() const {return varargs;};

        void set_system(System& new_system);
        void set_command(Command& new_command);
        void set_varargs(std::vector<uint8_t> varargs);

    private:
        System& system;
        Command& command;
        std::vector<uint8_t> varargs;
};



/*
    Add `RingBufferInterface`-like object (but with better name) that is simplified for reading and reassembing one large frame over a packet size-constrained link. Should get instantiated with packet size constraints, expected frame size, initial header length, subsequent header lengths (think SPMU-001 first packet w/spw header, then no header after). Then has a trio of methods for interface:
        void ::add_packet(std::vector<uint8_t> new_packet) to trim header then append
        int ::check_complete() to respond regarding empty, part full, overflowed
        std::vector<uint8_t> get_packet() to return assembled packet.

    Store this in a map keyed by `System`.
*/

class PacketFramer{
    public:
        PacketFramer(size_t new_packets_per_frame);
        PacketFramer(PacketFramer& other);
        
        /**
         * @brief Construct a new `PacketFramer` object from a `System` by inferring header/footer sizes from communication type.
         * @param system
         */
        PacketFramer(System& system);

        /**
         * @brief Empty the stored `frame` (to reuse this object to build a new frame).
         */
        void clear_frame();
        /**
         * @brief Push a packet onto the `frame`.
         */
        void push_to_frame();
        /**
         * @brief Check if the `frame` has been fully built.
         * 
         * @return true 
         * @return false 
         */
        bool check_frame_done();

        /**
         * @brief Pull a block of data off of the `frame`.
         * 
         * @param block_size 
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_from_frame(size_t block_size);
        const std::vector<uint8_t> get_frame() const {return frame;};

    private:
        /**
         * @brief Length of the header on every packet to remove when reassembling frame.
         */
        size_t static_strip_header_size;
        /**
         * @brief Length of header on *first packet only* to remove when reassembling frame.
         */
        size_t initial_strip_header_size;
        /**
         * @brief Length of header on subsequent (non-initial) packets to remove when reassembling frame.
         */
        size_t subsequent_strip_header_size;
        /**
         * @brief Length of the footer on every packet to remove when reassembling frame.
         */
        size_t static_strip_footer_size;
        /**
         * @brief Length of footer on *first packet only* to remove when reassembling frame.
         */
        size_t initial_strip_footer_size;
        /**
         * @brief Length of footer on subsequent (non-initial) packets to remove when reassembling frame.
         */
        size_t subsequent_strip_footer_size;

        /**
         * @brief Number of packets used to build one frame.
         */
        size_t packets_per_frame;

        /**
         * @brief Used to track progress on frame under assembly.
         */
        size_t packet_counter;
        /**
         * @brief Frame under assembly. 
         * `PacketFormatter::frame_done` will raise when the frame is completed.
         */
        std::vector<uint8_t> frame;
        /**
         * @brief Indicator that frame has been fully assembled.
         */
        bool frame_done;

        /**
         * @brief The `System` that produced the `frame` data.
         */
        System& system;
        /**
         * @brief The `Command` (if any) that prompted this `frame` response. 
         */
        Command& command;

};



/*
    Add `Fragmenter`-like object (but with better name, `Downlink`-specific) that is specialized to slice apart large packets for downlink.
        - instantiate with a reference to a `System`, MTU information.
    Has some fields to track slicing:
        size_t ::header_size
        size_t ::mtu
        size_t ::current_index that gets updated 
        DownlinkBufferElement get_slice() that cuts slab off of large stored packet, populates header info (`DownlinkBufferElement::num_packets_in_frame`, `DownlinkBufferElement::this_packet_index`) and packet.
*/

class FramePacketizer{
    public:
        FramePacketizer();

        /**
         * @brief Remove a packet from the `frame`.
         * 
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_packet();

    private:
        /**
         * @brief Get the packet header.
         * 
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_header();

        std::vector<uint8_t> frame;
        size_t max_packet_length;

        /**
         * @brief The number of packets remaining in `frame` .
         * 
         * This is updated whe `FramePacketizer::get_packet()` is called to account for the remaining packets that could be pulled from `frame`.
         * 
         */
        size_t packets_remaining_in_frame;
        
        /**
         * @brief The `System` that originated the data in `frame`.
         */
        System& system;
        /**
         * @brief The `Command` (if any) that prompted the acquisition of `frame`.
         * 
         */
        Command& command;
};

#endif