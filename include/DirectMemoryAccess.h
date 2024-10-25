/**
 * @file DirectMemoryAccess.h
 * @author Thanasi Pantazides
 * @brief Configuration data for remote memory interfaces.
 * @version v1.0.1
 * @date 2024-03-08
 * 
 */
#ifndef DIRECTMEMORYACCESS_H
#define DIRECTMEMORYACCESS_H

#include "Parameters.h"
#include <cstdlib>
#include <string>
#include <stdint.h>

/**
 * @brief Lists configuration data for interacting with remote memory.
 * 
 * Includes fields to track remote buffer frame size, ring buffer start addresses, size of ring buffer, recent write pointers. 
 * 
 * This was initially developed to support SpaceWire RMAP calls, and facilitate transfer of bulk data with the help of `PacketFramer`.
 * 
 * @note Since the initial design of this object, I have also used it to configure interfaces that do not interact with remote memory, such as UART calls or single-packet Ethernet transactions. It now provides a uniform interface for `SystemManager` objects to set packet and frame sizes. 
 * 
 * This object critically depends on `RING_BUFFER_TYPE_OPTIONS` to abstractly refer to the type of data stored in remote memory. A given `System` uses `RING_BUFFER_TYPE_OPTIONS` as a unique key into the type of `RingBufferParameters` it can access. In other words, a `System` can store several `RingBufferParameters`, but each must have a unique value of `RingBufferParameters::type`.
 */
class RingBufferParameters {
    public:
        /**
         * @brief Default constructor.
         * Populates zeros for all numeric fields, and `RING_BUFFER_PARAMETERS::NONE` for `::type.
         */
        RingBufferParameters();
        /**
         * @brief Copy constructor.
         * @param other the object to copy.
         */
        RingBufferParameters(const RingBufferParameters& other);
        /**
         * @brief Detailed constructor.
         * @param new_frame_size_bytes the size of a remote frame (in bytes).
         * @param new_start_address the memory address (in remote memory) where the ring buffer starts.
         * @param new_frames_per_ring the number off frames that can fit in the remote ring buffer.
         * @param new_write_pointer_address the address in remote memory where the ring buffer's write pointer can be found.
         * @param new_write_pointer_width_bytes the size of the write pointer. Implicitly 4 bytes.
         * @param new_type the type of data stored in this remote ring buffer.
         */
        RingBufferParameters(
            size_t new_frame_size_bytes,
            uint32_t new_start_address,
            size_t new_frames_per_ring,
            uint32_t new_write_pointer_address,
            size_t new_write_pointer_width_bytes,
            RING_BUFFER_TYPE_OPTIONS new_type
        );
        /**
         * @brief Comparison operator
         * 
         * @param other the object to compare to.
         * @return bool true if all members are the same.
         */
        bool operator==(RingBufferParameters& other);

        /**
         * @brief Size of a complete raw data frame, in bytes.
         * Assumes the remote ring buffer stores raw data in blocks, each with size `frame_size_bytes`.
         */
        size_t frame_size_bytes;
        /**
         * @brief The start address in remote memory of the ring buffer.
         */
        uint32_t start_address;
        /**
         * @brief The total number of frames in the remote ring buffer.
         */
        size_t frames_per_ring;
        /**
         * @brief The address in remote memory where the ring buffer's write pointer is found.
         * 
         * This value is queried by callers of `RingBufferParameters` to retrieve the remote write pointer, which is then used to query the remote ring buffer.
         */
        uint32_t write_pointer_address;
        /**
         * @brief The size of the field (located in remote memory) storing the remote write pointer, in bytes.
         */
        size_t write_pointer_width_bytes;

        /**
         * @brief The type of data stored in the remote buffer.
         */
        RING_BUFFER_TYPE_OPTIONS type;

        const std::string to_string();

    private:
};

#endif