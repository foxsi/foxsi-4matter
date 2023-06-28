#ifndef RINGBUFFERINTERFACE_H
#define RINGBUFFERINTERFACE_H

#include <iostream>

/**
 * @brief An interface to a ring buffer of data stored in blocks, in a system with 32 bit word length.
 * 
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

    private: 
        uint32_t start_address;
        uint32_t last_read_address;
        size_t size;
        size_t block_size;
        size_t read_count_blocks;

        void set_last_read_address(uint32_t new_last_read_address);
};

#endif