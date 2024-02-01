#include "Buffers.h"
#include <iostream>

#include "Utilities.h"
#include "Parameters.h"
#include <algorithm>
#include <cmath>

// ====================== DownlinkBufferElement ====================== //

DownlinkBufferElement::DownlinkBufferElement(System* new_system, size_t new_max_packet_size) : system(new_system) {
    max_packet_size = new_max_packet_size;

    // set these these to default for now:
    DownlinkBufferElement::set_packets_per_frame(1);
    DownlinkBufferElement::set_this_packet_index(1);
    std::vector<uint8_t> temp_payload = {};
    DownlinkBufferElement::set_payload(temp_payload);
    type = RING_BUFFER_TYPE_OPTIONS::NONE;
}

DownlinkBufferElement::DownlinkBufferElement(System *from_system, System *to_system, RING_BUFFER_TYPE_OPTIONS new_type): system(from_system) {

    if (!to_system->ethernet) {
        // todo: throw
        utilities::error_print("DownlinkBufferElement must use non-null System::ethernet interface\n");
    }

    // packet size is limited by the outgoing interface
    max_packet_size = to_system->ethernet->max_payload_size;
    
    // determine frame size by the remote memory access system used
    uint16_t source_frame_size = 0;
    switch (from_system->type) {
        case COMMAND_TYPE_OPTIONS::SPW:
            if (from_system->ring_params.size() > 0) {
                source_frame_size = from_system->get_frame_size(new_type);
            } else {
                source_frame_size = from_system->get_frame_size();
            }
            break;
        default:
            source_frame_size = from_system->get_frame_size();
            break;
    }
    size_t last_packet_size = source_frame_size % max_packet_size;
    packets_per_frame = source_frame_size / max_packet_size + std::min(last_packet_size, (size_t)1);
    utilities::debug_print("creating DBE:");
    utilities::debug_print("\n\tlast_packet_size:\t" + std::to_string(last_packet_size));
    utilities::debug_print("\n\tsource_frame_size:\t" + std::to_string(source_frame_size));
    utilities::debug_print("\n\tmax_packet_size:\t" + std::to_string(max_packet_size));
    utilities::debug_print("\n\tpackets_per_frame:\t" + std::to_string(packets_per_frame) + "\n");
    this_packet_index = 1;
    payload.reserve(max_packet_size - 8);
    payload.resize(0);
    type = new_type;
}

DownlinkBufferElement &DownlinkBufferElement::operator=(const DownlinkBufferElement &other) {
    max_packet_size = other.max_packet_size;
    payload = other.payload;
    system = other.system;
    packets_per_frame = other.packets_per_frame;
    this_packet_index = other.this_packet_index;
    type = other.type;

    return *this;
}

DownlinkBufferElement::DownlinkBufferElement(const DownlinkBufferElement &other) : system(other.system)
{
    max_packet_size = other.max_packet_size;
    DownlinkBufferElement::set_type(other.get_type());
    DownlinkBufferElement::set_packets_per_frame(other.get_packets_per_frame());
    DownlinkBufferElement::set_this_packet_index(other.get_this_packet_index());
    DownlinkBufferElement::set_payload(other.get_payload());
}

DownlinkBufferElement::DownlinkBufferElement() {
    system = nullptr;
    max_packet_size = 0;
    DownlinkBufferElement::set_packets_per_frame(1);
    DownlinkBufferElement::set_this_packet_index(1);
    std::vector<uint8_t> temp_payload = {};
    DownlinkBufferElement::set_payload(temp_payload);
    DownlinkBufferElement::set_type(RING_BUFFER_TYPE_OPTIONS::NONE);
}

void DownlinkBufferElement::set_payload(std::vector<uint8_t> new_payload) {
    if(check_payload_fits(new_payload)) {
        payload = new_payload;
    } else {
        utilities::error_print("DownlinkBufferElement payload "); 
        utilities::hex_print(new_payload);
        utilities::error_print(" doesn't fit in " + std::to_string(get_max_packet_size()) + "-sized packet!\n");
        // std::cout << "DownlinkBufferElement payload doesn't fit!\n";
        // todo: raise except
    }
    
}

void DownlinkBufferElement::set_packets_per_frame(uint16_t new_packets_per_frame) {
    packets_per_frame = new_packets_per_frame;
}

void DownlinkBufferElement::set_this_packet_index(uint16_t new_this_packet_index) {
    if(new_this_packet_index > packets_per_frame) {
        // todo: raise except
        utilities::error_print("future exception in DownlinkBufferElement::set_this_packet_index for desired index " + std::to_string(new_this_packet_index) + ". State: ");
        utilities::error_print(to_string() + "\n");

    }
    this_packet_index = new_this_packet_index;
}

void DownlinkBufferElement::set_type(RING_BUFFER_TYPE_OPTIONS new_type) {
    type = new_type;
}

const std::string DownlinkBufferElement::to_string() {
    std::string result;
    result.append("DownlinkBufferElement::");
    result.append("\n\tmax_packet_size \t= " + std::to_string(max_packet_size));
    result.append("\n\tpackets_per_frame \t= " + std::to_string(packets_per_frame));
    result.append("\n\tthis_packet_index \t= " + std::to_string(this_packet_index));
    result.append("\n\ttype \t\t\t= " + RING_BUFFER_TYPE_OPTIONS_NAMES.at(type));
    result.append("\n\tpayload.size() \t\t= " + std::to_string(payload.size()));
    result.append("\n\tsystem.name, .hex \t= " + system->name + ", " + std::to_string(system->hex));
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
    header.push_back(system->hex);
    std::vector<uint8_t> bytes_packets_per_frame = utilities::splat_to_nbytes(2, get_packets_per_frame());
    std::vector<uint8_t> bytes_packet_index = utilities::splat_to_nbytes(2, get_this_packet_index());
    std::vector<uint8_t> reserved_pad = {0x00, 0x00};
    header.insert(header.end(), bytes_packets_per_frame.begin(), bytes_packets_per_frame.end());
    header.insert(header.end(), bytes_packet_index.begin(), bytes_packet_index.end());
    header.push_back(static_cast<uint8_t>(type));
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

UplinkBufferElement::UplinkBufferElement(System *new_system, Command *new_command, std::vector<uint8_t> new_varargs): system(new_system), command(new_command) {
    varargs = new_varargs;
}

UplinkBufferElement::UplinkBufferElement(std::vector<uint8_t> raw_data, CommandDeck &deck): system(&deck.get_sys_for_code(raw_data.at(0))), command(&deck.get_command_for_sys_for_code(raw_data.at(0), raw_data.at(1))) {
    // varargs is all the raw data except the first two bytes.
    varargs.resize(raw_data.size() - 2);
    for (int i = 2; i < raw_data.size(); ++i) {
        varargs.push_back(raw_data.at(i));
    }
}

UplinkBufferElement::UplinkBufferElement(const UplinkBufferElement &other):
    system(other.system),
    command(other.command),
    varargs(other.varargs) {
}

UplinkBufferElement::UplinkBufferElement(): 
    system(nullptr), command(nullptr), varargs(0) { }

void UplinkBufferElement::set_varargs(std::vector<uint8_t> new_varargs) {
    varargs = new_varargs;
}

const std::string UplinkBufferElement::to_string() {
    std::string result;
    result.append("UplinkBufferElement::");
    result.append("\n\tsystem.name, .hex \t= " + system->name + ", " + std::to_string(system->hex));
    result.append("\n\tcommand.name, .hex \t= " + command->name + ", " + std::to_string(command->hex));
    result.append("\n\tvarargs.size() \t\t= " + std::to_string(varargs.size()));
    result.append("\n");

    return result;
}

// ========================== PacketFramer ========================= //

PacketFramer::PacketFramer(PacketFramer &other): 
    static_strip_footer_size(other.static_strip_footer_size),
    initial_strip_footer_size(other.initial_strip_footer_size),
    subsequent_strip_footer_size(other.subsequent_strip_footer_size),
    static_strip_header_size(other.static_strip_header_size),
    initial_strip_header_size(other.initial_strip_header_size),
    subsequent_strip_header_size(other.subsequent_strip_header_size),
    packets_per_frame(other.packets_per_frame),
    packet_counter(other.packet_counter),
    frame_size(other.frame_size),
    frame(other.frame),
    frame_done(other.frame_done),
    system(other.system),
    type(other.type)
{
    // todo: do other setup
    // static_strip_footer_size = other.static_strip_footer_size;
    // initial_strip_footer_size = other.initial_strip_footer_size;
    // subsequent_strip_footer_size = other.subsequent_strip_footer_size;

    // static_strip_header_size = other.static_strip_header_size;
    // initial_strip_header_size = other.initial_strip_header_size;
    // subsequent_strip_header_size = other.subsequent_strip_header_size;

    // packets_per_frame = other.packets_per_frame;
    // frame_size = other.frame_size;
    
    // packet_counter = other.packet_counter;
    // frame = other.frame;
    // frame_done = other.frame_done;

    // // set frame_size
    // frame.reserve(frame_size);
}

PacketFramer::PacketFramer(System& new_system, RING_BUFFER_TYPE_OPTIONS new_type): system(new_system) {
    // todo: do other setup
    DataLinkLayer* system_if;
    frame_size = new_system.get_frame_size();
    size_t packet_size = 0;
    type = new_type;

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
    packet_counter = 0;
}

void PacketFramer::push_to_frame(std::vector<uint8_t> new_packet) {
    if (frame_done || frame.size() == frame_size) {
        utilities::error_print("adding packet to full FramePacketizer::frame! refusing.\n");
        return;
    }

    if (frame.size() > 0) { // frame already has data in it
        size_t heads_and_feet_size = static_strip_footer_size 
            + subsequent_strip_footer_size
            + static_strip_header_size
            + subsequent_strip_header_size;

            if (frame.size() + new_packet.size() < heads_and_feet_size) {
                utilities::error_print("PacketFramer::push_to_frame() underflow\n");
            }
        
        if (frame.size() + new_packet.size() - heads_and_feet_size > frame_size) {
            // std::cout << "future exception in PacketFramer::push_to_frame()\n";
            // std::cout << "\tgot packet of size " << std::to_string(new_packet.size()) << " for frame of size " << std::to_string(frame_size) << " with real size " << std::to_string(frame.size()) << "\n";

            utilities::error_print("\tgot packet of size " + std::to_string(new_packet.size()) + " for frame with size " + std::to_string(frame.size()) + " and max size " + std::to_string(frame_size) + ". shrinking. \n");

            new_packet.erase(new_packet.begin(), new_packet.begin() + static_strip_header_size);
            new_packet.erase(new_packet.begin(), new_packet.begin() + subsequent_strip_header_size);

            new_packet.erase(new_packet.end() - static_strip_footer_size, new_packet.end());
            new_packet.erase(new_packet.end() - subsequent_strip_footer_size, new_packet.end());

            new_packet.resize(frame_size - frame.size());
            
            frame_done = true;
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
            // std::cout << "future exception in PacketFramer::push_to_frame()\n";
            utilities::error_print("PacketFramer::push_to_frame() overflow\n");
        }
        if (new_packet.size() < heads_and_feet_size) {
            utilities::error_print("PacketFramer::push_to_frame() underflow\n");
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
        result.at(i) = frame.at(i + start_size - block_size - 1);
        frame.pop_back();
        --packet_counter;
    }

    if (frame.size() == 0) {
        frame_done = true;
    }
    
    return result;
}

uint16_t PacketFramer::get_spw_transaction_id() {
    if (system.hex > 0xf) {
        utilities::error_print("future exception in PacketFramer::get_spw_transaction");
    }
    // todo: since packet_counter is 1-indexed this may not be true!
    if (packet_counter > 0xfff | packets_per_frame > 0xfff) {
        utilities::error_print("future exception in PacketFramer::get_spw_transaction");
    }

    uint16_t msb = (system.hex & 0xf) << 12;
    uint16_t lsb = packet_counter & 0xfff;
    return msb | lsb;
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
    result.append("\n\ttype \t\t\t\t= " + RING_BUFFER_TYPE_OPTIONS_NAMES.at(type));
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
    type = RING_BUFFER_TYPE_OPTIONS::NONE;
}

FramePacketizer::FramePacketizer(System &from_system, System &to_system, RING_BUFFER_TYPE_OPTIONS new_type): system(from_system) {
    if (!to_system.ethernet) {
        // todo: throw
        utilities::error_print("FramePacketizer must use non-null System::ethernet interface\n");
    }
    type = new_type;

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

    // size_t last_packet_size = frame_size % max_packet_size;
    // packets_per_frame = frame_size / max_packet_size + std::min(last_packet_size, (size_t)1);
    // packets_per_frame = std::ceil(packets_per_frame*(1.0 + 8.0/max_packet_size));
    
    header_size = 8;    // todo: migrate to foxsi4-commands/ethernets/*, instantiate from to_system::header_size
    packets_per_frame = std::ceil((float)frame_size / (float)(max_packet_size - header_size));
    packets_remaining_in_frame = packets_per_frame;
    frame.resize(0); 
    frame.reserve(max_packet_size*packets_per_frame);
}

FramePacketizer::FramePacketizer(PacketFramer &pf): system(pf.get_system()) {
    auto biggest_header = std::max(pf.initial_strip_header_size, pf.subsequent_strip_header_size);
    auto biggest_footer = std::max(pf.initial_strip_footer_size, pf.subsequent_strip_footer_size);

    type = pf.type;
    
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

FramePacketizer::FramePacketizer(FramePacketizer &other):
    header_size(other.header_size),
    frame(other.frame),
    max_packet_size(other.max_packet_size),
    packets_remaining_in_frame(other.packets_remaining_in_frame),
    packets_per_frame(other.packets_per_frame),
    system(other.system),
    type(other.type) {}

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
    DownlinkBufferElement dbel(&system, max_packet_size);
    std::vector<uint8_t> header = get_header();
    std::vector<uint8_t> payload = pop_payload();

    double efficiency = 1.0 + 8.0/max_packet_size;
    // size_t downlink_packets_per_frame = std::ceil(efficiency*packets_per_frame);
    // dbel.set_packets_per_frame(downlink_packets_per_frame);
    // dbel.set_this_packet_index(downlink_packets_per_frame - packets_remaining_in_frame);
    dbel.set_packets_per_frame(packets_per_frame);
    dbel.set_this_packet_index(packets_per_frame - packets_remaining_in_frame);

    // utilities::debug_print("\t\tFramePacketizer::packets_remaining_in_frame: " + std::to_string(packets_remaining_in_frame) + "\n");
    dbel.set_payload(payload);
    dbel.set_type(type);
    return dbel;
}

void FramePacketizer::set_frame(std::vector<uint8_t> new_frame) {
    frame.resize(new_frame.size());
    frame = new_frame;
    packets_remaining_in_frame = packets_per_frame;
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
    result.append("\n\ttype \t\t\t\t= " + RING_BUFFER_TYPE_OPTIONS_NAMES.at(type));
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
    header.push_back(static_cast<uint8_t>(type));
    header.push_back(0x00);
    header.push_back(0x00);

    return header;
}

// ========================== SystemManager ========================= //

SystemManager::SystemManager(System& new_system, std::queue<UplinkBufferElement>& new_uplink_buffer): system(new_system), uplink_buffer(new_uplink_buffer) {
    flight_state = FLIGHT_STATE::AWAIT;
    system_state = SYSTEM_STATE::OFF;
}

// SystemManager::SystemManager(SystemManager& other):
//     system(other.system),
//     timing(other.timing),
//     uplink_buffer(other.uplink_buffer),
//     flight_state(other.flight_state),
//     system_state(other.system_state),
//     lookup_frame_packetizer(other.lookup_frame_packetizer),
//     lookup_packet_framer(other.lookup_packet_framer) {
// }

// SystemManager::SystemManager(SystemManager&& other):
//     lookup_frame_packetizer(std::move(other.lookup_frame_packetizer)),
//     lookup_packet_framer(std::move(other.lookup_packet_framer)),
//     uplink_buffer((other.uplink_buffer)),
//     system((other.system)),
//     timing(std::move(other.timing)),
//     flight_state(std::move(other.flight_state)),
//     system_state(std::move(other.system_state)) {
// }

void SystemManager::add_frame_packetizer(RING_BUFFER_TYPE_OPTIONS new_type, FramePacketizer* new_frame_packetizer) {
    lookup_frame_packetizer[new_type] = new_frame_packetizer;
    // auto it = lookup_frame_packetizer.find(new_type);
    // if (it != lookup_frame_packetizer.end()) {
    //     it->second = new_frame_packetizer;
    // } else {
    //     lookup_frame_packetizer.insert(std::make_pair(new_type, new_frame_packetizer));
    // }
}

void SystemManager::add_packet_framer(RING_BUFFER_TYPE_OPTIONS new_type, PacketFramer* new_packet_framer) {
    auto it = lookup_packet_framer.find(new_type);
    if (it != lookup_packet_framer.end()) {
        it->second = new_packet_framer;
    } else {
        lookup_packet_framer.insert(std::make_pair(new_type, new_packet_framer));
    }
}

void SystemManager::add_timing(Timing *new_timing) {
    timing = new_timing;
}

PacketFramer* SystemManager::get_packet_framer(RING_BUFFER_TYPE_OPTIONS type) {
    auto it = lookup_packet_framer.find(type);
    if (it != lookup_packet_framer.end()) {
        return it->second;
    } else {
        utilities::error_print("could not find ring buffer type in PacketFramer map!\n");
        throw std::runtime_error("no key in map");
    }
}

FramePacketizer* SystemManager::get_frame_packetizer(RING_BUFFER_TYPE_OPTIONS type) {
    auto it = lookup_frame_packetizer.find(type);
    if (it != lookup_frame_packetizer.end()) {
        return it->second;
    } else {
        utilities::error_print("could not find ring buffer type in FramePacketizer map!\n");
        throw std::runtime_error("no key in map");
    }
}
