#include "RingBufferInterface.h"

#include <vector>

RingBufferInterface::RingBufferInterface() {
    start_address = 0x00;
    last_read_address = 0x00;
    size = 1;
    block_size = 1;
    read_count_blocks = 1;
}

RingBufferInterface::RingBufferInterface(uint32_t new_start_address, size_t new_size, size_t new_block_size, size_t new_read_count_blocks) {
    RingBufferInterface::set_start_address(new_start_address);
    RingBufferInterface::set_last_read_address(0x00);
    RingBufferInterface::set_size(new_size);
    RingBufferInterface::set_block_size(new_block_size);
    RingBufferInterface::set_read_count_blocks(new_read_count_blocks);
}

void RingBufferInterface::set_start_address(uint32_t new_start_address) {
    start_address = new_start_address;
}
void RingBufferInterface::set_size(size_t new_size) {
    size = new_size;
}
void RingBufferInterface::set_block_size(size_t new_block_size) {
    block_size = new_block_size;
}
void RingBufferInterface::set_read_count_blocks(size_t new_read_count_blocks) {
    read_count_blocks = new_read_count_blocks;
}
void RingBufferInterface::set_last_read_address(uint32_t new_last_read_address) {
    last_read_address = new_last_read_address;
}

uint32_t RingBufferInterface::read_block_from(uint32_t read_address) {
    // move `last_read_address` backward by `block_size`, wrapping around `start_address`

    long int dist_to_start = (long int)read_address - (long int)start_address;
    long int dist_to_end = (long int)start_address + (long int)size - (long int)read_address;
    if(dist_to_start < 0 || dist_to_end < 0) {
        std::cerr << "read address not in buffer!\n";
        throw "buffer overflow/underflow error";
    }
    uint32_t read_start_address = read_address - block_size*read_count_blocks;

    if(read_start_address >= start_address) {
        // can read without wraparound
        last_read_address = read_start_address;
        
    } else {
        // do wraparound
        uint32_t diff = start_address - read_start_address;
        last_read_address = start_address + size - last_read_address;
    }
    return last_read_address;
}

std::vector<uint32_t> RingBufferInterface::get_spw_data(uint32_t read_address) {
    std::vector<uint32_t> data;

    uint32_t last_address = RingBufferInterface::read_block_from(read_address);

    uint32_t start1 = 0;
    uint32_t length1 = 0;
    uint32_t start2 = 0;
    uint32_t length2 = 0;

    if(read_address > last_address) {
        // all within the buffer
        start1 = last_address;
        length1 = read_address - last_address;
    } else {
        // wraparound
        start1 = last_address;
        length1 = get_size() + get_start_address() - last_address;

        start2 = get_start_address();
        length2 = read_address - get_start_address();
    }

    data = {start1, length1, start2, length2};

    return data;
}