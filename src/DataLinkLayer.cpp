#include "DataLinkLayer.h"
#include <algorithm>
#include <iostream>

DataLinkLayer::DataLinkLayer() {
    mean_speed_bps = 0;
    max_payload_size = 1;
    frame_size = 1;
    static_header_size = 0;
    initial_header_size = 0;
    subsequent_header_size = 0;
    static_footer_size = 0;
    initial_footer_size = 0;
    subsequent_footer_size = 0;
}

DataLinkLayer::DataLinkLayer(const DataLinkLayer &other): 
    mean_speed_bps(other.mean_speed_bps),
    max_payload_size(other.max_payload_size),
    frame_size(other.frame_size),
    static_header_size(other.static_header_size),
    initial_header_size(other.initial_header_size),
    subsequent_header_size(other.subsequent_header_size),
    static_footer_size(other.static_footer_size),
    initial_footer_size(other.initial_footer_size),
    subsequent_footer_size(other.subsequent_footer_size) {
}

DataLinkLayer::DataLinkLayer(uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_static_footer_size) {
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = 0;
    subsequent_header_size = 0;
    static_footer_size = new_static_footer_size;
    initial_footer_size = 0;
    subsequent_footer_size = 0;
}

DataLinkLayer::DataLinkLayer(uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_initial_header_size, size_t new_subsequent_header_size, size_t new_static_footer_size, size_t new_initial_footer_size, size_t new_subsequent_footer_size) {
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = new_initial_header_size;
    subsequent_header_size = new_subsequent_header_size;
    static_footer_size = new_static_footer_size;
    initial_footer_size = new_initial_footer_size;
    subsequent_footer_size = new_subsequent_footer_size;
}

DataLinkLayer& DataLinkLayer::operator=(const DataLinkLayer& other) {
    mean_speed_bps = other.mean_speed_bps;
    max_payload_size = other.max_payload_size;
    frame_size = other.frame_size;
    static_header_size = other.static_header_size;
    initial_header_size = other.initial_header_size;
    subsequent_header_size = other.subsequent_header_size;
    static_footer_size = other.static_footer_size;
    initial_footer_size = other.initial_footer_size;
    subsequent_footer_size = other.subsequent_footer_size;

    return *this;
}



/*  Ethernet  */

Ethernet::Ethernet(): DataLinkLayer() {
    protocol = TransportLayerProtocol::NONE;
    address = "127.0.0.1";
    port = 7;
}

Ethernet::Ethernet(const Ethernet &other): 
    DataLinkLayer(other),
    protocol(other.protocol),
    address(other.address),
    port(other.port) {
}

Ethernet::Ethernet(const DataLinkLayer &link_layer): 
    DataLinkLayer(link_layer) {
    protocol = TransportLayerProtocol::NONE;
    address = "127.0.0.1";
    port = 7;
}

Ethernet::Ethernet(std::string new_address, uint16_t new_port, TransportLayerProtocol new_protocol, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_static_footer_size) {
    address = new_address;
    port = new_port;
    protocol = new_protocol;
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = 0;
    subsequent_header_size = 0;
    static_footer_size = new_static_footer_size;
    initial_footer_size = 0;
    subsequent_footer_size = 0;
}

Ethernet::Ethernet(std::string new_address, uint16_t new_port, std::string new_protocol, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_static_footer_size) {
    address = new_address;
    port = new_port;
    
    std::transform(new_protocol.begin(), new_protocol.end(), new_protocol.begin(), ::tolower);
    if (new_protocol == "tcp") {
        protocol = TransportLayerProtocol::TCP;
    } else if (new_protocol == "udp") {
        protocol = TransportLayerProtocol::UDP;
    } else {
        protocol = TransportLayerProtocol::NONE;
        std::cout << "Future error in Ethernet constructor\n";
    }
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = 0;
    subsequent_header_size = 0;
    static_footer_size = new_static_footer_size;
    initial_footer_size = 0;
    subsequent_footer_size = 0;
}

Ethernet::Ethernet(std::string new_address, uint16_t new_port, TransportLayerProtocol new_protocol, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_initial_header_size, size_t new_subsequent_header_size, size_t new_static_footer_size, size_t new_initial_footer_size, size_t new_subsequent_footer_size) {
    address = new_address;
    port = new_port;
    protocol = new_protocol;
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = new_initial_header_size;
    subsequent_header_size = new_subsequent_header_size;
    static_footer_size = new_static_footer_size;
    initial_footer_size = new_initial_footer_size;
    subsequent_footer_size = new_subsequent_footer_size;
}

Ethernet::Ethernet(std::string new_address, uint16_t new_port, std::string new_protocol, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_initial_header_size, size_t new_subsequent_header_size, size_t new_static_footer_size, size_t new_initial_footer_size, size_t new_subsequent_footer_size) {
    address = new_address;
    port = new_port;
    std::transform(new_protocol.begin(), new_protocol.end(), new_protocol.begin(), ::toupper);
    if (new_protocol == "tcp") {
        protocol = TransportLayerProtocol::TCP;
    } else if (new_protocol == "udp") {
        protocol = TransportLayerProtocol::UDP;
    } else {
        protocol = TransportLayerProtocol::NONE;
        std::cout << "Future error in Ethernet constructor\n";
    }
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = new_initial_header_size;
    subsequent_header_size = new_subsequent_header_size;
    static_footer_size = new_static_footer_size;
    initial_footer_size = new_initial_footer_size;
    subsequent_footer_size = new_subsequent_footer_size;
}

bool Ethernet::operator==(Ethernet &other)
{
    if (
        address == other.address 
        && port == other.port
        && protocol == other.protocol
        && mean_speed_bps == other.mean_speed_bps
        && max_payload_size == other.max_payload_size
        && frame_size == other.frame_size
        && static_header_size == other.static_header_size
        && initial_header_size == other.initial_header_size
        && subsequent_header_size == other.subsequent_header_size
        && static_footer_size == other.static_footer_size
        && initial_footer_size == other.initial_footer_size
        && subsequent_footer_size == other.subsequent_footer_size
    ) {
        return true;
    } else {
        return false;
    }
}

bool Ethernet::is_same_endpoint(Ethernet &other) {
    if (address == other.address && port == other.port && protocol == other.protocol) {
        return true;
    } else {
        return false;
    }
}

std::string Ethernet::to_string()
{
    std::string protocol_string;
    if (protocol == TransportLayerProtocol::TCP) {
        protocol_string = "tcp";
    } else if (protocol == TransportLayerProtocol::UDP) {
        protocol_string = "udp";
    } else {
        protocol_string = "";
    }
    return "("+protocol_string+")"+address+":"+std::to_string(port);
}

SpaceWire::SpaceWire(): DataLinkLayer() {
    target_path_address.resize(0);
    reply_path_address.resize(0);
    target_logical_address = 0x00;
    source_logical_address = 0x00;
    key = 0x00;
    crc_version = 'f';
}

SpaceWire::SpaceWire(const SpaceWire &other): 
    DataLinkLayer(other), 
    target_path_address(other.target_path_address),
    reply_path_address(other.reply_path_address),
    target_logical_address(other.target_logical_address),
    source_logical_address(other.source_logical_address),
    key(other.key),
    crc_version(other.crc_version) {
}

SpaceWire::SpaceWire(const DataLinkLayer &link_layer): 
    DataLinkLayer(link_layer) {
    target_path_address.resize(0);
    reply_path_address.resize(0);
    target_logical_address = 0x00;
    source_logical_address = 0x00;
    key = 0x00;
    crc_version = 'f';
}

SpaceWire::SpaceWire(std::vector<uint8_t> new_target_path_address, std::vector<uint8_t> new_reply_path_address, uint8_t new_target_logical_address, uint8_t new_source_logical_address, uint8_t new_key, char new_crc_version, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_static_footer_size) {
    target_path_address = new_target_path_address;
    reply_path_address = new_reply_path_address;
    target_logical_address = new_target_logical_address;
    source_logical_address = new_source_logical_address;
    key = new_key;
    crc_version = new_crc_version;
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = 0;
    subsequent_header_size = 0;
    static_footer_size = new_static_footer_size;
    initial_footer_size = 0;
    subsequent_footer_size = 0;
}

SpaceWire::SpaceWire(std::vector<uint8_t> new_target_path_address, std::vector<uint8_t> new_reply_path_address, uint8_t new_target_logical_address, uint8_t new_source_logical_address, uint8_t new_key, char new_crc_version, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_initial_header_size, size_t new_subsequent_header_size, size_t new_static_footer_size, size_t new_initial_footer_size, size_t new_subsequent_footer_size) {
    target_path_address = new_target_path_address;
    reply_path_address = new_reply_path_address;
    target_logical_address = new_target_logical_address;
    source_logical_address = new_source_logical_address;
    key = new_key;
    crc_version = new_crc_version;
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = new_initial_header_size;
    subsequent_header_size = new_subsequent_header_size;
    static_footer_size = new_static_footer_size;
    initial_footer_size = new_initial_footer_size;
    subsequent_footer_size = new_subsequent_footer_size;
}

bool SpaceWire::operator==(SpaceWire &other)
{
    if (
        target_path_address == other.target_path_address 
        && reply_path_address == other.reply_path_address
        && target_logical_address == other.target_logical_address
        && source_logical_address == other.source_logical_address
        && key == other.key
        && crc_version == other.crc_version
        && mean_speed_bps == other.mean_speed_bps
        && max_payload_size == other.max_payload_size
        && frame_size == other.frame_size
        && static_header_size == other.static_header_size
        && initial_header_size == other.initial_header_size
        && subsequent_header_size == other.subsequent_header_size
        && static_footer_size == other.static_footer_size
        && initial_footer_size == other.initial_footer_size
        && subsequent_footer_size == other.subsequent_footer_size
    ) {
        return true;
    } else {
        return false;
    }
}

uint8_t SpaceWire::crc(std::vector<uint8_t> data)
{
    if (crc_version == 'f') {
        uint8_t result = 0x00;
        size_t length = data.size();
        for (size_t i = 0; i < length; i++) {
            result = RMAPCRCTable[(result ^ data[i]) & 0xff];
        }
        return result;
    } else {
        // todo: throw
    }
}

UART::UART(): DataLinkLayer() {
    baud_rate = 9600;
    parity = 1;
    stop_bits = 1;
    data_bits = 8;
}

UART::UART(const UART &other):
    DataLinkLayer(other),
    baud_rate(other.baud_rate),
    parity(other.parity),
    stop_bits(other.stop_bits),
    data_bits(other.data_bits) { 

}

UART::UART(const DataLinkLayer &link_layer):
    DataLinkLayer(link_layer) {
    baud_rate = 9600;
    parity = 1;
    stop_bits = 1;
    data_bits = 8;
}

UART::UART(uint32_t new_baud_rate, uint8_t new_parity, uint8_t new_stop_bits, uint8_t new_data_bits, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_static_footer_size) {
    baud_rate = new_baud_rate;
    parity = new_parity;
    stop_bits = new_stop_bits;
    data_bits = new_data_bits;
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = 0;
    subsequent_header_size = 0;
    static_footer_size = new_static_footer_size;
    initial_footer_size = 0;
    subsequent_footer_size = 0;
}

UART::UART(uint32_t new_baud_rate, uint8_t new_parity, uint8_t new_stop_bits, uint8_t new_data_bits, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_initial_header_size, size_t new_subsequent_header_size, size_t new_static_footer_size, size_t new_initial_footer_size, size_t new_subsequent_footer_size) {
    baud_rate = new_baud_rate;
    parity = new_parity;
    stop_bits = new_stop_bits;
    data_bits = new_data_bits;
    mean_speed_bps = new_mean_speed_bps;
    max_payload_size = new_max_payload_size;
    frame_size = new_frame_size;
    static_header_size = new_static_header_size;
    initial_header_size = new_initial_header_size;
    subsequent_header_size = new_subsequent_header_size;
    static_footer_size = new_static_footer_size;
    initial_footer_size = new_initial_footer_size;
    subsequent_footer_size = new_subsequent_footer_size;
}

bool UART::operator==(UART &other)
{
    if (
        baud_rate == other.baud_rate 
        && parity == other.parity
        && stop_bits == other.stop_bits
        && data_bits == other.data_bits
        && mean_speed_bps == other.mean_speed_bps
        && max_payload_size == other.max_payload_size
        && frame_size == other.frame_size
        && static_header_size == other.static_header_size
        && initial_header_size == other.initial_header_size
        && subsequent_header_size == other.subsequent_header_size
        && static_footer_size == other.static_footer_size
        && initial_footer_size == other.initial_footer_size
        && subsequent_footer_size == other.subsequent_footer_size
    ) {
        return true;
    } else {
        return false;
    }
}
