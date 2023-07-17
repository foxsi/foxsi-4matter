#include "Fragmenter.h"
#include "Utilities.h"

#include <iostream>


Fragmenter::Fragmenter() {
    fragment_size = 1;
    header_size = 3;
    make_max_message_size();
}


Fragmenter::Fragmenter(size_t new_fragment_size, size_t new_header_size, std::vector<uint8_t> new_prefix): fragment_prefix(new_prefix) {
    
    if(new_header_size%3 != 0) {
        std::cerr << "Fragmenter::header_size must be a multiple of 3.\n";
        throw "Fragmenter error";
    }
    // compute max allowable message size and store.
    fragment_size = new_fragment_size;
    header_size = new_header_size;

    make_max_message_size();

    // todo: instantiate this based on minimum of MTU fields in systems.json/gse or systems.json/evtm
}

Fragmenter::Fragmenter(size_t new_fragment_size, size_t new_header_size): fragment_prefix({}) {
    
    if(new_header_size%3 != 0) {
        std::cerr << "Fragmenter::header_size must be a multiple of 3.\n";
        throw "Fragmenter error";
    }
    
    
    // compute max allowable message size and store.
    fragment_size = new_fragment_size;
    header_size = new_header_size;

    make_max_message_size();

    // todo: instantiate this based on minimum of MTU fields in systems.json/gse or systems.json/evtm
}

Fragmenter::Fragmenter(const Fragmenter& other): fragment_size(other.get_fragment_size()), header_size(other.get_header_size()), fragment_prefix(other.get_fragment_prefix()) {
    Fragmenter::make_max_message_size();
}

Fragmenter& Fragmenter::operator=(const Fragmenter& other) {
    fragment_size = other.get_fragment_size();
    header_size = other.get_header_size();
    fragment_prefix = other.get_fragment_prefix();
    
    Fragmenter::make_max_message_size();

    return *this;
}

void Fragmenter::make_max_message_size() {
    size_t n_count = header_size/3;
    if(n_count > 8) {
        std::cerr << "header is too big\n";
        throw "Fragmenter error";
    }

    uint64_t result = 0;
    for(int i = 0; i < 8; i++) {
        result |= (0xff << i*8);
    }
    
    max_message_size = result;
    std::cout << "max_message_size: " << max_message_size << "\n";
}

std::vector<std::vector<uint8_t>> Fragmenter::fragment(std::vector<uint8_t> message_to_fragment) {
    // check message is not too big
    //  can the header size field and index field cover it?
    if(message_to_fragment.size() > max_message_size) {
        std::cerr << "message is too big for Fragmenter!\n";
        throw "Fragmenter error";
    }

    // find how many fragments are needed for this message:
    size_t n_fragments = message_to_fragment.size()/fragment_size;
    if(message_to_fragment.size()%fragment_size > 0) {
        ++n_fragments;
    }

    std::vector<uint8_t> head_packet_count = splat_to_nbytes(header_size/3, n_fragments);

    // build list of fragments
    // prepend header to each fragment

    std::vector<std::vector<uint8_t>> fragments;

    size_t packet_count = 0;
    for(size_t i=0; i < message_to_fragment.size(); i = i + fragment_size) {
        auto last = std::min(message_to_fragment.size(), i + fragment_size);
        
        std::vector<uint8_t> this_packet;
        std::vector<uint8_t> head_idx = splat_to_nbytes(header_size/3, packet_count);
        std::vector<uint8_t> head_byte_count = splat_to_nbytes(header_size/3, last - i);
        this_packet.insert(this_packet.end(), head_byte_count.begin(), head_byte_count.end());
        this_packet.insert(this_packet.end(), head_packet_count.begin(), head_packet_count.end());
        this_packet.insert(this_packet.end(), head_idx.begin(), head_idx.end());
        this_packet.insert(this_packet.end(), message_to_fragment.begin() + i, message_to_fragment.begin() + last);

        // std::vector<uint8_t> submessage(message_to_fragment.begin() + i, message_to_fragment.begin() + last);
        
        fragments.push_back(this_packet);
        ++packet_count;
    }

    return fragments;
}