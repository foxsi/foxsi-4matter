/**
 * @file Buffers.h
 * @author Thanasi Pantazides
 * @brief Buffer objects for managing uplink, downlink, and segmentation of onboard data.
 * @version v1.0.1
 * @date 2024-03-07
 */

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
 * This object stores all the information needed to build a downlink packet in the format: 
 * 
 * ```
 * [1B] source system
 * [2B] total number of packets in this frame
 * [2B] index of this packet in frame
 * [1B] type of data
 * [1B] 0x00
 * [1B] 0x00
 * [nB] payload
 * ```
 * 
 * This object should be stored in the global downlink queue, then removed and serialized prior to sending. On the ground, the header data can be used to re-sequence a complete frame from packets.
 * 
 * @warning Packets are 1-indexed, not 0-indexed.
 */
class DownlinkBufferElement {
    public:
        /**
         * @brief Construct a new `DownlinkBufferElement` object from a source system and maximum allowable downlink packet size.
         * 
         * @param new_system the `System` object which generated the data for downlink.
         * @param new_max_packet_size the maximum allowable size for a downlink packet (includes header). Related to downlink MTU.
         */
        DownlinkBufferElement(System* new_system, size_t new_max_packet_size);

        /**
         * @brief Construct a new `DownlinkBufferElement` object from two `System`s, specifying the type of ring buffer interface to use from the sending `System`.
         * 
         * The field `DownlinkBufferElement::system` is assigned a reference to the `from_system` constructor argument (instead of `to_system`). This preserves frame sourcing information during buffer handoff.
         * 
         * @param from_system the `System` object which generated the data for downlink.
         * @param to_system the `System` object which is responsible for transmitting the data.
         * @param type an index into `from_system::ring_params` specifying the buffer parameters to use.
         */
        DownlinkBufferElement(System* from_system, System* to_system, RING_BUFFER_TYPE_OPTIONS new_type);

        DownlinkBufferElement& operator=(const DownlinkBufferElement& other);

        /**
         * @brief Copy-construct a new `DownlinkBufferElement` object.
         * 
         * @param other the object to copy.
         */
        DownlinkBufferElement(const DownlinkBufferElement& other);

        DownlinkBufferElement();

        /**
         * @brief Get byte string to send over physical interface to the ground.
         * This method prepends a header (`DownlinkBufferElement::get_header()`) to `DownlinkBufferElement::payload` and returns the total list of bytes to transmit. 
         * @warning The packet index value starts at 1, not 0.
         * @return std::vector<uint8_t> The byte stream to transmit.
         */
        std::vector<uint8_t> get_packet();

        const System get_system() const {return *system;};
        const uint8_t get_system_hex() const {return system->hex;};
        const std::vector<uint8_t> get_payload() const {return payload;};
        const size_t get_max_packet_size() const {return max_packet_size;};
        const uint16_t get_packets_per_frame() const {return packets_per_frame;};
        const uint16_t get_this_packet_index() const {return this_packet_index;};
        const RING_BUFFER_TYPE_OPTIONS get_type() const {return type;};
        
        /**
         * @brief Set the payload of the `DownlinkBufferElement`.
         * Will check if the payload will fit in max packet size.
         * 
         * @param new_payload the payload to set. 
         */
        void set_payload(std::vector<uint8_t> new_payload);

        /**
         * @brief Set the number of packets per frame.
         * 
         * @param new_packets_per_frame the number of packets that make up a frame.
         */
        void set_packets_per_frame(uint16_t new_packets_per_frame);
        
        /**
         * @brief Set the index of this packet in the frame.
         * @warning Indexed from 1, not 0.
         * @param new_this_packet_index the index of this packet in the frame.
         */
        void set_this_packet_index(uint16_t new_this_packet_index);
        /**
         * @brief Set the data type transmitted in this frame.
         * @param new_type type of data.
         */
        void set_type(RING_BUFFER_TYPE_OPTIONS new_type);

        /**
         * @brief Create a `std::string` representation of this object.
         * @return const std::string 
         */
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
         * @warning `this_packet_index` is 1-indexed, not 0-indexed.
         */
        uint16_t this_packet_index;
        /**
         * @brief The data type transferred in this packet.
         * This will be used on the ground to log data to the appropriate file.
         */
        RING_BUFFER_TYPE_OPTIONS type;
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



/**
 * @brief An class to iteratively assemble complete frames, packet by packet.
 * 
 * The use-case for this class is to retrieve a full data frame from a remote `System` over a link with limited packet size—i.e. such that the full data frame cannot be retrieved in one packet.
 * @relates FramePacketizer
 */
class PacketFramer{
    friend class FramePacketizer;
    public:
        /**
         * @brief Copy-construct a new `PacketFramer` object.
         * 
         * @param other the object to copy.
         */
        PacketFramer(PacketFramer& other);
        
        /**
         * @brief Construct a new `PacketFramer` object from a `System` by inferring header/footer sizes from communication type.
         * @param new_system the `System` used to determine frame/packet size.
         * @param new_type the type of data in the frame/packets.
         */
        PacketFramer(System& new_system, RING_BUFFER_TYPE_OPTIONS new_type);

        System& get_system() {return system;};
        RING_BUFFER_TYPE_OPTIONS get_type() const {return type;};

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

        /**
         * @brief Make a 2B Transaction ID for an RMAP command. 
         * First four bytes are unique system ID, last twelve bytes are packet counter.
         * @return uint16_t 
         */
        uint16_t get_spw_transaction_id();

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
         * `PacketFormatter::frame_done` will be `true` when the frame is completed.
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
         * @brief The type of data in the frame.
         * 
         */
        RING_BUFFER_TYPE_OPTIONS type;
};



/**
 * @brief Provides a mechanism to segment a complete frame of data into smaller packets for downlink. 
 * This is useful if a raw frame is larger than the downlink MTU.
 * 
 * @relates PacketFramer
 */
class FramePacketizer{
    friend class PacketFramer;
    public:
        /**
         * @brief Construct a new `FramePacketizer` from a `System`, maximum packet length, and number of packets per frame.
         * 
         * @param new_system the `System` that produced the data in the frame.
         * @param new_max_packet_size the maxumum allowable packet size.
         * @param new_packets_per_frame the number of packets per frame.
         */
        FramePacketizer(System& new_system, size_t new_max_packet_size, size_t new_packets_per_frame);

        /**
         * @brief Construct a new `FramePacketizer` using a source and sink `System`.
         * 
         * The `System` `to_system` defines the the maximum allowable packet size, under the assumption packets will be sent to `to_system`. The `System` `from_system` defines the size of the frame being packetized.
         * 
         * @param from_system the `System` that generated the data being packetized.
         * @param to_system the `System` which will receive the packets.
         * @param new_type the type of data in the frame.
         */
        FramePacketizer(System& from_system, System& to_system, RING_BUFFER_TYPE_OPTIONS new_type);

        /**
         * @brief Construct a new `FramePacketizer` from a `PacketFramer`.
         * This constructor infers max sizes and header contents from members of `PacketFramer`.
         * @param pf the object to infer sizes from.
         */
        FramePacketizer(PacketFramer& pf);

        /**
         * @brief Copy-construct a new `FramePacketizer` object.
         * 
         * @param fp the object to copy.
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
         * @brief Set a new `frame` to be packetized.
         * 
         * @param new_frame the frame to set.
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
         * @brief Length of header data, in bytes.
         */
        size_t header_size;

        /**
         * @brief The raw data frame which will be sliced into packets.
         */
        std::vector<uint8_t> frame;

        /**
         * @brief The maximum length of one packet to remove from `frame`, in bytes.
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
         * @brief The `System` that produced the data in `frame`.
         */
        System& system;

        /**
         * @brief The type of data in the frame.
         */
        RING_BUFFER_TYPE_OPTIONS type;
};


/**
 * @brief `SystemManager` is intended to handle dynamic behavior of `System`s.
 * 
 * `System` objects are `const`, once initialized, and contain physical and interface configuration data for a `System`. `SystemManager` is intended as a dynamic wrapper around the `System` object which handles packet/frame flow, commanding, timing, errors checking and handling, etc.
 * 
 * @note There are a few places in this codebase that use `System` in places that `SystemManager` would be more convenient. Heads up.
 */
class SystemManager {
    public:
        /**
         * @brief Construct a new `SystemManager` from a source `System`, and provide a `std::queue` of `UplinkBufferElements` for commanding.
         * @param new_system the `System` to be managed by this object.
         * @param new_uplink_buffer a queue for storing uplink commands to this `System`.
         */
        SystemManager(
            System& new_system,
            std::queue<UplinkBufferElement>& new_uplink_buffer
        );

        void clear_errors();

        /**
         * @brief Add a `FramePacketizer` object for handling `new_type`s of data.
         * Some `System`s have frames that require packetization, if they are larger than the network MTU. In that case, an appropriate `FramePacketizer` (for the `System`, data type, frame size, and packet size) should be created and added to this `SystemManager`. 
         * 
         * In many places in the code, we want to pass `SystemManager`s around and let them internally handle data flow/packetization/framing. So this method composes `System` configuration with frame packetization to enable that.
         * 
         * Internally, `FramePacketizers`s are stored in a `std::unordered_map<RING_BUFFER_TYPE_OPTIONS>`, so each `FramePacketizer` will be identified uniquely by the type of data it handles.
         * 
         * @param new_type the type of data the `FramePacketizer` will handle.
         * @param new_frame_packetizer a raw pointer to the `FramePacketizer` object.
         */
        void add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS new_type, FramePacketizer* new_frame_packetizer);
        /**
         * @brief Add a `PacketFramer` object for handling `new_type`s of data.
         * Some `System`s have frames that require framing, if the frame arrives in pieces. In that case, an appropriate `PacketFramer` (for the `System`, data type, frame size, and packet size) should be created and added to this `SystemManager`.
         * 
         * In many places in the code, we want to pass `SystemManager`s around and let them internally handle data flow/packetization/framing. So this method composes `System` configuration with packet framing to enable that.
         * 
         * Internally, `PacketFramer`s are stored in a `std::unordered_map<RING_BUFFER_TYPE_OPTIONS>`, so each `PacketFramer` will be identified uniquely by the type of data it handles.
         * 
         * @param new_type the type of data the `PacketFramer` will handle.
         * @param new_packet_framer a raw pointer to the `PacketFramer` object.
         */
        void add_packet_framer(RING_BUFFER_TYPE_OPTIONS new_type, PacketFramer* new_packet_framer);

        /**
         * @brief Add `Timing` data to the `SystemManager`.
         * The `Timing` object holds information about the time allocation for this `System` in the main loop, and information about the timeout behavior of this `System` (in the case of failure to receive a reply).
         * @param new_timing a raw pointer to the timing data to add.
         */
        void add_timing(Timing* new_timing);

        /**
         * @brief Getter for `FramePacketizer` object for the provided data `type`.
         * @param type data type the `FramePacketizer` is used on.
         * @return FramePacketizer*
         */
        FramePacketizer* get_frame_packetizer(RING_BUFFER_TYPE_OPTIONS type);
        /**
         * @brief Getter for `PacketFramer` object for the provided data `type`.
         * @param type data type the `PacketFramer` is used on.
         * @return PacketFramer*
         */
        PacketFramer* get_packet_framer(RING_BUFFER_TYPE_OPTIONS type);
        
        /**
         * @brief The underlying `System` configuration information. 
         * @note This object should be used as a runtime constant, once initialized.
         */
        System& system;

        /**
         * @brief The buffer used to store commands which are sent to this `System`.
         */
        std::queue<UplinkBufferElement>& uplink_buffer;
        /**
         * @brief The timing configuration data for this `System`.
         */
        Timing* timing;

        /**
         * @brief The current state of flight this `System` is in.
         */
        FLIGHT_STATE flight_state;
        /**
         * @brief The local state of this `System`.
         */
        SYSTEM_STATE system_state;

        uint16_t errors;

        /**
         * @brief Track which type of remote memory this `System` is currently reading.
         */
        RING_BUFFER_TYPE_OPTIONS active_type;
        
        /**
         * @brief Track the most recent write pointer location in remote memory, for ring buffer access.
         * Can use to prevent repeated reads of same location in memory, if the memory has not been updated.
         */
        std::unordered_map<RING_BUFFER_TYPE_OPTIONS, size_t> last_write_pointer;

        /**
         * @brief General-purpose counter for this `System`, for tracking error accumulation etc.
         */
        size_t counter;
        uint8_t enable;
    
    private:
        std::unordered_map<RING_BUFFER_TYPE_OPTIONS, FramePacketizer*> lookup_frame_packetizer;
        std::unordered_map<RING_BUFFER_TYPE_OPTIONS, PacketFramer*> lookup_packet_framer;
        
};

#endif