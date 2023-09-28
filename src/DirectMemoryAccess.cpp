#include "DirectMemoryAccess.h"

RingBufferParameters::RingBufferParameters() {
    frame_size_bytes = 0;
    start_address = 0;
    frames_per_ring = 0;
    write_pointer_address = 0;
    write_pointer_width_bytes = 0;
}

RingBufferParameters::RingBufferParameters(const RingBufferParameters &other):
    frame_size_bytes(other.frame_size_bytes),
    start_address(other.start_address),
    frames_per_ring(other.frames_per_ring),
    write_pointer_address(other.write_pointer_address),
    write_pointer_width_bytes(other.write_pointer_width_bytes) {}

RingBufferParameters::RingBufferParameters(size_t new_frame_size_bytes, uint32_t new_start_address, size_t new_frames_per_ring, uint32_t new_write_pointer_address, size_t new_write_pointer_width_bytes): 
    frame_size_bytes(new_frame_size_bytes),
    start_address(new_start_address),
    frames_per_ring(new_frames_per_ring),
    write_pointer_address(new_write_pointer_address),
    write_pointer_width_bytes(new_write_pointer_width_bytes) {}

bool RingBufferParameters::operator==(RingBufferParameters &other) {
    if( frame_size_bytes == other.frame_size_bytes
        && start_address == other.start_address
        && frames_per_ring == other.frames_per_ring
        && write_pointer_address == other.write_pointer_address
        && write_pointer_width_bytes == other.write_pointer_width_bytes
    ) {
        return true;
    } else {
        return false;
    }
}
