#ifndef RINGBUFFERINTERFACE_H
#define RINGBUFFERINTERFACE_H

#include <iostream>
#include <vector>

/**
 * @deprecated Superseded by `FramePacketizer`.
 */
class RingBufferInterface {
    
    public:
        RingBufferInterface();
        RingBufferInterface(uint32_t new_start_address, size_t new_size, size_t new_block_size, size_t new_read_count_blocks);

        const uint32_t get_start_address() const {return start_address;}
        const uint32_t get_last_read_address() const {return last_read_address;}
        const size_t get_size() const {return size;}
        const size_t get_block_size() const {return block_size;}
        const size_t get_read_count_blocks() const {return read_count_blocks;}

        void set_start_address(uint32_t new_start_address);
        void set_size(size_t new_size);
        void set_block_size(size_t new_block_size);
        void set_read_count_blocks(size_t new_read_count_blocks);
        
        uint32_t read_block_from(uint32_t read_address);
        
        /**
         * @brief get SpaceWire command data and update `RingBufferInterface::last_read_address`.
         * 
         * @return a `std::vector<uint32_t>` with four values: start address in the remote ring buffer and data length for region one of memory, start address in the remote ring before and data length for region two of memory. If the ring buffer does not wrap around, the data length value for region two will be zero.
        */
        std::vector<uint32_t> get_spw_data(uint32_t read_address);

    private:
        
        /**
         * @brief address where ring buffer memory starts
        */
        uint32_t start_address;
        
        /**
         * @brief last address in buffer that was read (by calling `read_block_from()`).
        */
        uint32_t last_read_address;
        
        /**
         * @brief total ring buffer size, in bytes.
        */
        size_t size;

        /**
         * @brief size of a block, in bytes.
        */
        size_t block_size;

        /**
         * @brief number of blocks to read at once.
        */
        size_t read_count_blocks;

        void set_last_read_address(uint32_t new_last_read_address);
};

#endif