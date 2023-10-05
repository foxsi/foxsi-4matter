#include "Buffers.h"
#include <iostream>

#include "Utilities.h"
#include "Parameters.h"
#include <algorithm>

// ====================== DownlinkBufferElement ====================== //

DownlinkBufferElement::DownlinkBufferElement(System& new_system, size_t new_max_packet_size) : system(new_system) {
    max_packet_size = new_max_packet_size;

    // set these these to default for now:
    DownlinkBufferElement::set_packets_per_frame(0);
    DownlinkBufferElement::set_this_packet_index(0);
    std::vector<uint8_t> temp_payload = {};
    DownlinkBufferElement::set_payload(temp_payload);
}

DownlinkBufferElement::DownlinkBufferElement(System &from_system, System &to_system, RING_BUFFER_TYPE_OPTIONS type): system(from_system) {

    if (!to_system.ethernet) {
        // todo: throw
        utilities::error_print("DownlinkBufferElement must use non-null System::ethernet interface\n");
    }

    // packet size is limited by the outgoing interface
    max_packet_size = to_system.ethernet->max_payload_size;
    
    // determine frame size by the remote memory access system used
    uint16_t source_frame_size = 0;
    switch (from_system.type) {
        case COMMAND_TYPE_OPTIONS::SPW:
            if (from_system.ring_params.size() > 0) {
                source_frame_size = from_system.get_frame_size(type);
            } else {
                source_frame_size = from_system.get_frame_size();
            }
            break;
        default:
            source_frame_size = from_system.get_frame_size();
            break;
    }
    size_t last_packet_size = source_frame_size % max_packet_size;
    packets_per_frame = source_frame_size / max_packet_size + std::min(last_packet_size, (size_t)1);

    this_packet_index = 0;
    payload.reserve(max_packet_size - 8);
    payload.resize(0);
}

DownlinkBufferElement::DownlinkBufferElement(const DownlinkBufferElement &other) : system(other.system)
{
    max_packet_size = other.max_packet_size;
    DownlinkBufferElement::set_packets_per_frame(other.get_packets_per_frame());
    DownlinkBufferElement::set_this_packet_index(other.get_this_packet_index());
    DownlinkBufferElement::set_payload(other.get_payload());
}

void DownlinkBufferElement::set_payload(std::vector<uint8_t> new_payload) {
    if(check_payload_fits(new_payload)) {
        payload = new_payload;
    } else {
        std::cout << "DownlinkBufferElement payload doesn't fit!\n";
        // todo: raise except
    }
    
}

void DownlinkBufferElement::set_packets_per_frame(uint16_t new_packets_per_frame) {
    packets_per_frame = new_packets_per_frame;
}

void DownlinkBufferElement::set_this_packet_index(uint16_t new_this_packet_index) {
    if(new_this_packet_index > packets_per_frame) {
        // todo: raise except
        std::cout << "future exception in DownlinkBufferElement::set_this_packet_index\n";
    }
    this_packet_index = new_this_packet_index;
}

const std::string DownlinkBufferElement::to_string() {
    std::string result;
    result.append("DownlinkBufferElement::");
    result.append("\n\tmax_packet_size \t= " + std::to_string(max_packet_size));
    result.append("\n\tpackets_per_frame \t= " + std::to_string(packets_per_frame));
    result.append("\n\tthis_packet_index \t= " + std::to_string(this_packet_index));
    result.append("\n\tpayload.size() \t\t= " + std::to_string(payload.size()));
    result.append("\n\tsystem.name, .hex \t= " + system.name + ", " + std::to_string(system.hex));
    result.append("\n");

    return result;
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
    std::vector<uint8_t> bytes_packets_per_frame = utilities::splat_to_nbytes(2, get_packets_per_frame());
    std::vector<uint8_t> bytes_packet_index = utilities::splat_to_nbytes(2, get_this_packet_index());
    std::vector<uint8_t> reserved_pad = {0x00, 0x00, 0x00};
    header.insert(header.end(), bytes_packets_per_frame.begin(), bytes_packets_per_frame.end());
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

UplinkBufferElement::UplinkBufferElement(System &new_system, Command &new_command, std::vector<uint8_t> new_varargs): system(new_system), command(new_command) {
    varargs = new_varargs;
}

UplinkBufferElement::UplinkBufferElement(std::vector<uint8_t> raw_data, CommandDeck &deck): system(deck.get_sys_for_code(raw_data[0])), command(deck.get_command_for_sys_for_code(raw_data[0], raw_data[1])) {
    // varargs is all the raw data except the first two bytes.
    varargs.resize(raw_data.size() - 2);
    for (int i = 2; i < raw_data.size(); ++i) {
        varargs.push_back(raw_data[i]);
    }
}

void UplinkBufferElement::set_varargs(std::vector<uint8_t> new_varargs) {
    varargs = new_varargs;
}

const std::string UplinkBufferElement::to_string() {
    std::string result;
    result.append("UplinkBufferElement::");
    result.append("\n\tsystem.name, .hex \t= " + system.name + ", " + std::to_string(system.hex));
    result.append("\n\tcommand.name, .hex \t= " + command.name + ", " + std::to_string(command.hex));
    result.append("\n\tvarargs.size() \t\t= " + std::to_string(varargs.size()));
    result.append("\n");

    return result;
}

// ========================== PacketFramer ========================= //

PacketFramer::PacketFramer(PacketFramer &other): system(other.system) {
    // todo: do other setup
    static_strip_footer_size = other.static_strip_footer_size;
    initial_strip_footer_size = other.initial_strip_footer_size;
    subsequent_strip_footer_size = other.subsequent_strip_footer_size;

    static_strip_header_size = other.static_strip_header_size;
    initial_strip_header_size = other.initial_strip_header_size;
    subsequent_strip_header_size = other.subsequent_strip_header_size;

    packets_per_frame = other.packets_per_frame;
    frame_size = other.frame_size;
    
    // don't copy frame from the other object.
    packet_counter = 0;
    frame.resize(0);
    frame_done = false;

    // set frame_size
    frame.reserve(frame_size);
    frame_done = false;
}

PacketFramer::PacketFramer(System& new_system, RING_BUFFER_TYPE_OPTIONS type): system(new_system) {
    // todo: do other setup
    DataLinkLayer* system_if;
    frame_size = new_system.get_frame_size();
    size_t packet_size = 0;


    switch (new_system.type) {
        case COMMAND_TYPE_OPTIONS::ETHERNET:
            system_if = new_system.ethernet;
            packet_size = new_system.ethernet->max_payload_size;
            break;
        case COMMAND_TYPE_OPTIONS::SPW:
            system_if = new_system.spacewire;
            packet_size = new_system.spacewire->max_payload_size;
            if (new_system.ring_params.size() > 0) {  // if possible, overwrite source_frame_size with ring buffer data
                frame_size = new_system.get_frame_size(type);
            }
            break;
        case COMMAND_TYPE_OPTIONS::UART:
            system_if = new_system.uart;
            packet_size = new_system.uart->max_payload_size;
            break;
        default:
            packet_size = 0;
            break;
    }

    if (system_if) {
        static_strip_footer_size = system_if->static_footer_size;
        initial_strip_footer_size = system_if->initial_footer_size;
        subsequent_strip_footer_size = system_if->subsequent_footer_size;
        static_strip_header_size = system_if->static_header_size;
        initial_strip_header_size = system_if->initial_header_size;
        subsequent_strip_header_size = system_if->subsequent_header_size;
    } else {
        utilities::debug_print("Found nullptr to system interface in PacketFramer. Initializing fields as zero.\n");
        static_strip_footer_size = 0;
        initial_strip_footer_size = 0;
        subsequent_strip_footer_size = 0;
        static_strip_header_size = 0;
        initial_strip_header_size = 0;
        subsequent_strip_header_size = 0;
    }

    size_t last_packet_size = frame_size % packet_size;
    packets_per_frame = frame_size / packet_size + std::min(packet_size, (size_t)1);
    packet_counter = 0;
    frame.reserve(frame_size);
    frame_done = false;
}

void PacketFramer::clear_frame() {
    frame.resize(0);
    frame_done = false;
}

void PacketFramer::push_to_frame(std::vector<uint8_t> new_packet) {

    if (frame.size() > 0) { // frame already has data in it
        size_t heads_and_feet_size = static_strip_footer_size 
            + subsequent_strip_footer_size
            + static_strip_header_size
            + subsequent_strip_header_size;
        
        if (frame.size() + new_packet.size() - heads_and_feet_size > frame_size) {
            // todo: throw
            // todo: THIS SHOULD NOT EXCEPT. The condition should be checked on a new_packet **post-header/footer removal**.
            std::cout << "future exception in PacketFramer::push_to_frame()\n";
            std::cout << "\tgot packet of size " << std::to_string(new_packet.size()) << " for frame of size " << std::to_string(frame_size) << " with real size " << std::to_string(frame.size()) << "\n";
        } else if (frame.size() + new_packet.size() - heads_and_feet_size == frame_size) {
            // erase static and subsequent headers and footers
            new_packet.erase(new_packet.begin(), new_packet.begin() + static_strip_header_size);
            new_packet.erase(new_packet.begin(), new_packet.begin() + subsequent_strip_header_size);

            new_packet.erase(new_packet.end() - static_strip_footer_size, new_packet.end());
            new_packet.erase(new_packet.end() - subsequent_strip_footer_size, new_packet.end());
            
            // mark frame as done
            frame_done = true;
        } else {
            // erase static and subsequent headers and footers
            new_packet.erase(new_packet.begin(), new_packet.begin() + static_strip_header_size);
            new_packet.erase(new_packet.begin(), new_packet.begin() + subsequent_strip_header_size);

            new_packet.erase(new_packet.end() - static_strip_footer_size, new_packet.end());
            new_packet.erase(new_packet.end() - subsequent_strip_footer_size, new_packet.end());
        }

    } else {    // no packets in frame
        // todo: also check this will not overflow frame. could just be adding an enormous packet to a small frame.
        size_t heads_and_feet_size = static_strip_footer_size 
            + initial_strip_footer_size
            + static_strip_header_size
            + initial_strip_header_size;
        
        if (new_packet.size() - heads_and_feet_size > frame_size) {
            // todo: throw
            std::cout << "future exception in PacketFramer::push_to_frame()\n";
        }

        // erase static and initial headers and footers
        new_packet.erase(new_packet.begin(), new_packet.begin() + initial_strip_header_size);
        new_packet.erase(new_packet.begin(), new_packet.begin() + static_strip_header_size);
        
        new_packet.erase(new_packet.end() - initial_strip_footer_size, new_packet.end());
        new_packet.erase(new_packet.end() - static_strip_footer_size, new_packet.end()); 
    }

    frame.insert(frame.end(), new_packet.begin(), new_packet.end());
    ++packet_counter;
}

bool PacketFramer::check_frame_done() {
    if (frame.size() == frame_size) {
        return true;
    } else {
        return false;
    }
}

std::vector<uint8_t> PacketFramer::pop_from_frame(size_t block_size) {
    std::vector<uint8_t> result(block_size);
    size_t start_size = frame.size();
    for (int i = 0; i < block_size; --i) {
        // todo: verify this is not off-by-one
        result[i] = frame[i + start_size - block_size - 1];
        frame.pop_back();
    }

    if (frame.size() == 0) {
        frame_done = true;
    }
    
    return result;
}

const std::string PacketFramer::to_string() {
    std::string result;
    result.append("PacketFramer::");
    result.append("\n\tframe_size \t\t\t= " + std::to_string(frame_size));
    result.append("\n\tframe.size() \t\t\t= " + std::to_string(frame.size()));
    result.append("\n\tframe_done \t\t\t= " + std::to_string(frame_done));
    result.append("\n\tpackets_per_frame \t\t= " + std::to_string(packets_per_frame));
    result.append("\n\tpacket_counter \t\t\t= " + std::to_string(packet_counter));
    result.append("\n\tsystem.name, system.hex \t= " + system.name + ", " + std::to_string(system.hex));
    result.append("\n\tstatic_strip_header_size \t= " + std::to_string(static_strip_header_size));
    result.append("\n\tstatic_strip_footer_size \t= " + std::to_string(static_strip_footer_size));
    result.append("\n\tinitial_strip_header_size \t= " + std::to_string(initial_strip_header_size));
    result.append("\n\tinitial_strip_footer_size \t= " + std::to_string(initial_strip_footer_size));
    result.append("\n\tsubsequent_strip_header_size \t= " + std::to_string(subsequent_strip_header_size));
    result.append("\n\tsubsequent_strip_footer_size \t= " + std::to_string(subsequent_strip_footer_size));
    result.append("\n");
    
    return result;
}

// ========================== FramePacketizer ========================= //

FramePacketizer::FramePacketizer(System &new_system, size_t new_max_packet_size, size_t new_packets_per_frame): system(new_system) {
    max_packet_size = new_max_packet_size;
    packets_per_frame = new_packets_per_frame;
    
    // setup other fields to defaults:
    packets_remaining_in_frame = 0;
    header_size = 8;
    frame.resize(0);
}

FramePacketizer::FramePacketizer(System &from_system, System &to_system, RING_BUFFER_TYPE_OPTIONS type): system(from_system) {
    if (!to_system.ethernet) {
        // todo: throw
        utilities::error_print("FramePacketizer must use non-null System::ethernet interface\n");
    }

    size_t frame_size = 0;
    switch(from_system.type) {
        case (COMMAND_TYPE_OPTIONS::SPW):
            if (from_system.ring_params.size() > 0) {
                frame_size = from_system.get_frame_size(type);
            } else {
                frame_size = from_system.get_frame_size();
            }
            break;
        default:
            frame_size = from_system.get_frame_size();
            break;
    }
    max_packet_size = to_system.ethernet->max_payload_size;

    size_t last_packet_size = frame_size % max_packet_size;
    packets_per_frame = frame_size / max_packet_size + std::min(last_packet_size, (size_t)1);

    packets_remaining_in_frame = packets_per_frame;
    header_size = 8;
    frame.resize(0); 
    frame.reserve(max_packet_size*packets_per_frame);
}

FramePacketizer::FramePacketizer(PacketFramer &pf): system(pf.get_system()) {
    auto biggest_header = std::max(pf.initial_strip_header_size, pf.subsequent_strip_header_size);
    auto biggest_footer = std::max(pf.initial_strip_footer_size, pf.subsequent_strip_footer_size);
    
    // max length packet sent to `PacketFramer` includes headers and footers.
    max_packet_size = pf.frame_size/pf.packets_per_frame;
            // - biggest_header 
            // - biggest_footer 
            // - pf.static_strip_header_size 
            // - pf.static_strip_footer_size;

    packets_per_frame = pf.packets_per_frame;
    packets_remaining_in_frame = pf.packets_per_frame;
    header_size = 8;
    frame.resize(0);
}

std::vector<uint8_t> FramePacketizer::pop_packet() {
    
    std::vector<uint8_t> result;
    std::vector<uint8_t> header = FramePacketizer::get_header();
    std::vector<uint8_t> payload = FramePacketizer::pop_payload();
    result.insert(result.end(), header.begin(), header.end());
    result.insert(result.end(), payload.begin(), payload.end());
    
    return result;
}


std::vector<uint8_t> FramePacketizer::pop_payload() {
    size_t pop_size = std::min(frame.size(), max_packet_size - header_size);
    // if (frame.size() < max_packet_size + header_size) {
    //     //todo: throw error
    //     std::cout << "future exception in FramePacketizer::pop_payload()\n";
    // }

    std::vector<uint8_t> payload;
    payload.insert(payload.end(), frame.begin(), frame.begin() + pop_size);

    frame.erase(frame.begin(), frame.begin() + pop_size);
    --packets_remaining_in_frame;
    return payload;
}

DownlinkBufferElement FramePacketizer::pop_buffer_element() {
    DownlinkBufferElement dbel(system, max_packet_size);
    std::vector<uint8_t> header = FramePacketizer::get_header();
    std::vector<uint8_t> payload = FramePacketizer::pop_payload();

    dbel.set_packets_per_frame(packets_per_frame);
    dbel.set_this_packet_index(packets_per_frame - packets_remaining_in_frame);
    dbel.set_payload(payload);
    return dbel;
}

void FramePacketizer::set_frame(std::vector<uint8_t> new_frame) {
    frame.resize(new_frame.size());
    frame = new_frame;
}

void FramePacketizer::clear_frame() {
    frame.resize(0);
}

bool FramePacketizer::frame_emptied() {
    // todo: is there any scenario where there are packets left in frame but it is also empty?
    if (frame.size() == 0) {
        return true;
    } else {
        return false;
    }
}

const std::string FramePacketizer::to_string() {
    std::string result;
    result.append("FramePacketizer::");
    result.append("\n\theader_size \t\t\t= " + std::to_string(header_size));
    result.append("\n\tframe.size() \t\t\t= " + std::to_string(frame.size()));
    result.append("\n\tmax_packet_size \t\t= " + std::to_string(max_packet_size));
    result.append("\n\tpackets_per_frame \t\t= " + std::to_string(packets_per_frame));
    result.append("\n\tpackets_remaining_in_frame \t= " + std::to_string(packets_remaining_in_frame));
    result.append("\n\tsystem.name, system.hex \t= " + system.name + ", " + std::to_string(system.hex));
    result.append("\n");

    return result;
}

std::vector<uint8_t> FramePacketizer::get_header() {
    // header: <System::hex> <count> <count> <index> <index> <0x00> <0x00> <0x00>
    std::vector<uint8_t> header;

    uint8_t count_msb = (packets_per_frame >> 8) & 0xff;
    uint8_t count_lsb = packets_per_frame & 0xff;

    uint8_t remain_msb = (packets_remaining_in_frame >> 8) & 0xff;
    uint8_t remain_lsb = packets_per_frame & 0xff;

    header.push_back(system.hex);
    header.push_back(count_msb);
    header.push_back(count_lsb);
    header.push_back(remain_msb);
    header.push_back(remain_lsb);
    header.push_back(0x00);
    header.push_back(0x00);
    header.push_back(0x00);

    return header;
}
