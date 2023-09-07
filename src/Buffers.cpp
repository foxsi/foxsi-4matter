#include "Buffers.h"
#include <iostream>

#include "Utilities.h"
#include "Parameters.h"

// ====================== DownlinkBufferElement ====================== //

DownlinkBufferElement::DownlinkBufferElement(System& new_system, size_t max_packet_size) : system(new_system) {
    DownlinkBufferElement::set_max_packet_size(max_packet_size);

    // set these these to default for now:
    DownlinkBufferElement::set_num_packets_in_frame(0);
    DownlinkBufferElement::set_this_packet_index(0);
    std::vector<uint8_t> temp_payload = {};
    DownlinkBufferElement::set_payload(temp_payload);
}

DownlinkBufferElement::DownlinkBufferElement(const DownlinkBufferElement &other) : system(other.system) {
    DownlinkBufferElement::set_max_packet_size(other.get_max_packet_size());
    DownlinkBufferElement::set_num_packets_in_frame(other.get_num_packets_in_frame());
    DownlinkBufferElement::set_this_packet_index(other.get_num_packets_in_frame());
    DownlinkBufferElement::set_payload(other.get_payload());

}

void DownlinkBufferElement::set_system(System &new_system) {
    system = new_system;
}

void DownlinkBufferElement::set_payload(std::vector<uint8_t> new_payload) {
    if(check_payload_fits(new_payload)) {
        payload = new_payload;
    } else {
        std::cout << "DownlinkBufferElement payload doesn't fit!\n";
        // todo: raise except
    }
    
}

void DownlinkBufferElement::set_max_packet_size(size_t new_max_packet_size) {
    max_packet_size = new_max_packet_size;
}

void DownlinkBufferElement::set_num_packets_in_frame(uint16_t new_num_packets_in_frame) {
    num_packets_in_frame = new_num_packets_in_frame;
}

void DownlinkBufferElement::set_this_packet_index(uint16_t new_this_packet_index) {
    if(this_packet_index >= num_packets_in_frame) {
        // todo: raise except
    } 
    this_packet_index = new_this_packet_index;
}

bool DownlinkBufferElement::check_payload_fits(std::vector<uint8_t> new_payload) {
    if(new_payload.size() + 8 <= get_max_packet_size()) {
        return true;
    } else {
        return false;
    }
}

std::vector<uint8_t> DownlinkBufferElement::get_header() {
    std::vector<uint8_t> header;
    header.push_back(system.hex);
    std::vector<uint8_t> bytes_num_packets_in_frame = splat_to_nbytes(2, get_num_packets_in_frame());
    std::vector<uint8_t> bytes_packet_index = splat_to_nbytes(2, get_this_packet_index());
    std::vector<uint8_t> reserved_pad = {0x00, 0x00, 0x00};
    header.insert(header.end(), bytes_num_packets_in_frame.begin(), bytes_num_packets_in_frame.end());
    header.insert(header.end(), bytes_packet_index.begin(), bytes_packet_index.end());
    header.insert(header.end(), reserved_pad.begin(), reserved_pad.end());

    return header;
}

std::vector<uint8_t> DownlinkBufferElement::get_packet() {
    std::vector<uint8_t> header = get_header();
    std::vector<uint8_t> this_payload = get_payload();
    std::vector<uint8_t> packet;
    packet.insert(packet.end(), header.begin(), header.end());
    packet.insert(packet.end(), this_payload.begin(), this_payload.end());

    if(packet.size() > get_max_packet_size()) {
        // todo: throw
        std::cout << "DownlinkBufferElement packet size too big.\n";
        return {};
    }
    return packet;
}

// ====================== UplinkBufferElement ====================== //