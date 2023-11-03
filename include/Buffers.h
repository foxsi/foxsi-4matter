#pragma once
#ifndef BUFFERS_H
#define BUFFERS_H

#include "Systems.h"
#include "Timing.h"
#include "Commanding.h"
#include "Parameters.h"

#include <string>
#include <queue>

/**
 * @brief A data element in the downlink queue.
 * 
 * This object stores all the information needed to build a downlink packet in the format: `<source system [1B]><total number of packets in this frame [2B]><index of this packet in frame [2B]><0x00><0x00><0x00><payload [max_packet_size - 8B]>`. This object should be stored in the global downlink queue and removed and serialized prior to sending.
 */
class DownlinkBufferElement {
    public:
        DownlinkBufferElement(System* new_system, size_t new_max_packet_size);

        /**
         * @brief Construct a new `DownlinkBufferElement` object from two `System`s, specifying the type of ring buffer interface to use from the sending `System`.
         * 
         * The field `DownlinkBufferElement::system` is assigned a reference to the `from_system` constructor argument (instead of `to_system`). This preserves frame sourcing information during buffer handoff.
         * 
         * @param from_system The `System` object which generated the data for downlink.
         * @param to_system The `System` object which is responsible for transmitting the data.
         * @param type An index into `from_system.ring_params` specifying the buffer parameters to use.
         */
        DownlinkBufferElement(System* from_system, System* to_system, RING_BUFFER_TYPE_OPTIONS type);

        DownlinkBufferElement& operator=(const DownlinkBufferElement& other);

        /**
         * @brief Copy-construct a new `DownlinkBufferElement` object.
         * 
         * @param other 
         */
        DownlinkBufferElement(const DownlinkBufferElement& other);

        DownlinkBufferElement();

        /**
         * @brief Get byte string to send over physical interface to the ground.
         * This method prepends a header (`DownlinkBufferElement::get_header()`) to `DownlinkBufferElement::payload` and returns the total list of bytes to transmit.
         * @return std::vector<uint8_t> The byte stream to transmit.
         */
        std::vector<uint8_t> get_packet();

        const System get_system() const {return *system;};
        const uint8_t get_system_hex() const {return system->hex;};
        const std::vector<uint8_t> get_payload() const {return payload;};
        const size_t get_max_packet_size() const {return max_packet_size;};
        const uint16_t get_packets_per_frame() const {return packets_per_frame;};
        const uint16_t get_this_packet_index() const {return this_packet_index;};
        
        void set_payload(std::vector<uint8_t> new_payload);
        void set_packets_per_frame(uint16_t new_packets_per_frame);
        void set_this_packet_index(uint16_t new_this_packet_index);

        const std::string to_string();

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
         * Downlink data is `<header><payload>`, where `<header>` comprises `<system id><packets_per_frame><this_packet_index><0x00><0x00><0x00>`.
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
        System* system;
        /**
         * @brief The total number of transmit packets in this data frame. 
         * This will be used on the ground to assemble one parse-able data frame.
         */
        uint16_t packets_per_frame;
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
 * Assumes that uplink data takes the form `<System::hex [1B]><Command::hex [1B]><varargs [nB]>`.
 */
class UplinkBufferElement {
    public:
        /**
         * @brief Construct a new `UplinkBufferElement` object by populating its members directly.
         * 
         * @param system The `System` targeted by the uplink command.
         * @param command The specific `Command` uplinked.
         * @param varargs Optional list of bytes after the first two (`System::hex` and `Command::hex`).
         */
        UplinkBufferElement(System* system, Command* command, std::vector<uint8_t> varargs);
        /**
         * @brief Construct a new `UplinkBufferElement` object from an incoming (uplinked) byte stream.
         * 
         * @param raw_data The raw byte stream that was uplinked.
         * @param deck A `CommandDeck` used to decode the byte stream.
         */
        UplinkBufferElement(std::vector<uint8_t> raw_data, CommandDeck& deck);

        /**
         * @brief Copy-construct a new `UplinkBufferElement` object
         * 
         * @param other 
         */
        UplinkBufferElement(const UplinkBufferElement& other);

        UplinkBufferElement();

        void set_varargs(std::vector<uint8_t> new_varargs);

        System* get_system() const {return system;};
        Command* get_command() const {return command;};
        std::vector<uint8_t> get_varargs() const {return varargs;};

        const std::string to_string();

    private:
        /**
         * @brief The `System` referenced in the first byte of the uplink packet.
         */
        System* system;
        /**
         * @brief The `Command` referenced in the second byte of the uplink packet.
         */
        Command* command;
        /**
         * @brief Any contents of the uplink packet following the second byte.
         */
        std::vector<uint8_t> varargs;
};



/*
    Add `RingBufferInterface`-like object (but with better name) that is simplified for reading and reassembing one large frame over a packet size-constrained link. Should get instantiated with packet size constraints, expected frame size, initial header length, subsequent header lengths (think SPMU-001 first packet w/spw header, then no header after). Then has a trio of methods for interface:
        void ::add_packet(std::vector<uint8_t> new_packet) to trim header then append
        int ::check_complete() to respond regarding empty, part full, overflowed
        std::vector<uint8_t> get_packet() to return assembled packet.

    Store this in a map keyed by `System`.
*/

/**
 * @brief An class to iteratively assemble complete frames, packet by packet.
 * 
 * The target application for this class is to retrieve a full data frame from a remote `System` over a link with limited packet size—i.e. such that the full data frame cannot be retrieved in one packet.
 * @relates FramePacketizer
 */
class PacketFramer{
    friend class FramePacketizer;
    public:
        /**
         * @brief Copy-construct a new `PacketFramer` object.
         * 
         * @param other 
         */
        PacketFramer(PacketFramer& other);
        
        /**
         * @brief Construct a new `PacketFramer` object from a `System` by inferring header/footer sizes from communication type.
         * @param system
         */

        // todo: reimplement with new System model
        PacketFramer(System& new_system, RING_BUFFER_TYPE_OPTIONS type);

        System& get_system() {return system;};

        /**
         * @brief Empty the stored `frame` (to reuse this object to build a new frame).
         */
        void clear_frame();
        /**
         * @brief Push a packet (removable headers and all) onto the `frame`.
         */
        void push_to_frame(std::vector<uint8_t> new_packet);
        /**
         * @brief Check if the `frame` has been fully built.
         * 
         * @return true If the `frame` has been completed.
         * @return false If the `frame` is incomplete.
         */
        bool check_frame_done();

        /**
         * @brief Pull a block of data off of the `frame`.
         * 
         * @param block_size 
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> pop_from_frame(size_t block_size);
        /**
         * @brief Get the `frame`.
         * 
         * @return const std::vector<uint8_t> 
         */
        const std::vector<uint8_t> get_frame() const {return frame;};

        const std::string to_string();

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
         * @brief Size of a finished frame.
         */
        size_t frame_size;
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
};



/*
    Add `Fragmenter`-like object (but with better name, `Downlink`-specific) that is specialized to slice apart large packets for downlink.
        - instantiate with a reference to a `System`, MTU information.
    Has some fields to track slicing:
        size_t ::header_size
        size_t ::mtu
        size_t ::current_index that gets updated 
        DownlinkBufferElement get_slice() that cuts slab off of large stored packet, populates header info (`DownlinkBufferElement::packets_per_frame`, `DownlinkBufferElement::this_packet_index`) and packet.
*/


/**
 * @brief A class to produce downlink-ready packets from a complete data frame.
 * 
 * @relates PacketFramer
 */
class FramePacketizer{
    friend class PacketFramer;
    public:
        /**
         * @brief Construct a new `FramePacketizer` from a `System` and a maximum packet length.
         * 
         * @param new_system 
         * @param new_max_packet_size 
         */
        FramePacketizer(System& new_system, size_t new_max_packet_size, size_t new_packets_per_frame);

        // todo: implement
        FramePacketizer(System& from_system, System& to_system, RING_BUFFER_TYPE_OPTIONS type);

        /**
         * @brief Construct a new `FramePacketizer` from a `PacketFramer`.
         * This constructor infers max sizes and header contents from members of `PacketFramer`.
         * @param pf 
         */
        FramePacketizer(PacketFramer& pf);

        /**
         * @brief Copy-construct a new `FramePacketizer` object.
         * 
         * @param fp 
         */
        FramePacketizer(FramePacketizer& other);

        /**
         * @brief Remove a packet from the `frame`.
         * 
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> pop_packet();

        /**
         * @brief Remove only payload data (headerless) from `frame`.
         * 
         * Size of payload data returned leaves room for header data to be passed.
         * 
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> pop_payload();
    
        /**
         * @brief Remove a `DownlinkBufferElement` from the `frame`.
         * 
         * @return DownlinkBufferElement 
         */
        DownlinkBufferElement pop_buffer_element();

        /**
         * @brief Populate a new `frame` object.
         * 
         * @param new_frame 
         */
        void set_frame(std::vector<uint8_t> new_frame);
        
        /**
         * @brief Remove all contents of `frame`.
         * 
         */
        void clear_frame();

        /**
         * @brief Check if the `frame` has been fully packetized and is now empty.
         * 
         * @return true The `frame` is empty.
         * @return false The `frame` is nonempty.
         */
        bool frame_emptied();

        const std::string to_string();

    private:
        /**
         * @brief Build the packet header.
         * 
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> get_header();

        /**
         * @brief Length of header data in bytes.
         * 
         */
        size_t header_size;

        /**
         * @brief The raw data frame which will be sliced into packets.
         */
        std::vector<uint8_t> frame;

        /**
         * @brief The maximum length of one packet to remove from `frame`.
         * This is intended to be used as the maximum payload size of a data packet over some communication protocol. This includes data header and raw frame data, but not additional headers that will be prepended by the hardware driver.
         */
        size_t max_packet_size;
        /**
         * @brief The number of packets remaining in `frame` .
         * 
         * This is updated whe `FramePacketizer::get_packet()` is called to account for the remaining packets that could be pulled from `frame`.
         */
        size_t packets_remaining_in_frame;
        
        /**
         * @brief The total number of packets in a frame.
         * 
         * Packets assumed to be of maximum length (`max_packet_size`).
         */
        size_t packets_per_frame;
        
        /**
         * @brief The `System` that originated the data in `frame`.
         */
        System& system;
};



class SystemManager {
    public:
        SystemManager(
            System& new_system,
            std::queue<UplinkBufferElement>& new_uplink_buffer
        );
        // SystemManager(SystemManager& other);
        // SystemManager(SystemManager&& other);

        void add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS new_type, FramePacketizer* new_frame_packetizer);
        void add_packet_framer(RING_BUFFER_TYPE_OPTIONS new_type, PacketFramer* new_packet_framer);

        void add_timing(Timing* new_timing);

        FramePacketizer* get_frame_packetizer(RING_BUFFER_TYPE_OPTIONS type);
        PacketFramer* get_packet_framer(RING_BUFFER_TYPE_OPTIONS type);
        
        System& system;
        std::queue<UplinkBufferElement>& uplink_buffer;
        Timing* timing;

        FLIGHT_STATE flight_state;
        SYSTEM_STATE system_state;

    private:
        std::unordered_map<RING_BUFFER_TYPE_OPTIONS, FramePacketizer*> lookup_frame_packetizer;
        std::unordered_map<RING_BUFFER_TYPE_OPTIONS, PacketFramer*> lookup_packet_framer;
};

#endif