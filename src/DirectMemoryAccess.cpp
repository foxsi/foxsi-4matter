#include "DirectMemoryAccess.h"

RingBufferParameters::RingBufferParameters() {
    frame_size_bytes = 0;
    start_address = 0;
    frames_per_ring = 0;
    write_pointer_address = 0;
    write_pointer_width_bytes = 0;
    type = RING_BUFFER_TYPE_OPTIONS::NONE;
}

RingBufferParameters::RingBufferParameters(const RingBufferParameters &other):
    frame_size_bytes(other.frame_size_bytes),
    start_address(other.start_address),
    frames_per_ring(other.frames_per_ring),
    write_pointer_address(other.write_pointer_address),
    write_pointer_width_bytes(other.write_pointer_width_bytes),
    type(other.type) {}

RingBufferParameters::RingBufferParameters(size_t new_frame_size_bytes, uint32_t new_start_address, size_t new_frames_per_ring, uint32_t new_write_pointer_address, size_t new_write_pointer_width_bytes, RING_BUFFER_TYPE_OPTIONS new_type): 
    frame_size_bytes(new_frame_size_bytes),
    start_address(new_start_address),
    frames_per_ring(new_frames_per_ring),
    write_pointer_address(new_write_pointer_address),
    write_pointer_width_bytes(new_write_pointer_width_bytes),
    type(new_type) {}

bool RingBufferParameters::operator==(RingBufferParameters &other) {
    if( frame_size_bytes == other.frame_size_bytes
        && start_address == other.start_address
        && frames_per_ring == other.frames_per_ring
        && write_pointer_address == other.write_pointer_address
        && write_pointer_width_bytes == other.write_pointer_width_bytes
        && type == other.type
    ) {
        return true;
    } else {
        return false;
    }
}

const std::string RingBufferParameters::to_string() {
    std::string result;
    result.append("RingBufferParameters::");
    result.append("\n\tframe_size_bytes \t= " + std::to_string(frame_size_bytes));
    result.append("\n\tstart_address \t\t= " + std::to_string(start_address));
    result.append("\n\tframes_per_ring \t= " + std::to_string(frames_per_ring));
    result.append("\n\twrite_pointer_address \t= " + std::to_string(write_pointer_address));
    result.append("\n\twrite_pointer_width_bytes \t= " + std::to_string(write_pointer_width_bytes));
    result.append("\n");

    return result;
}
