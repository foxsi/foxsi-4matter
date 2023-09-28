#ifndef DIRECTMEMORYACCESS_H
#define DIRECTMEMORYACCESS_H

#include <cstdlib>

class RingBufferParameters {
    public:
        RingBufferParameters();
        RingBufferParameters(const RingBufferParameters& other);
        RingBufferParameters(
            size_t new_frame_size_bytes,
            uint32_t new_start_address,
            size_t new_frames_per_ring,
            uint32_t new_write_pointer_address,
            size_t new_write_pointer_width_bytes
        );
        bool operator==(RingBufferParameters& other);

        size_t frame_size_bytes;
        uint32_t start_address;
        size_t frames_per_ring;
        uint32_t write_pointer_address;
        size_t write_pointer_width_bytes;

    private:
};

#endif