#include "DataLinkLayer.h"
#include "Utilities.h"
#include <algorithm>
#include <iomanip>
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
        // utilities::error_print("got bad SpaceWire CRC version!: " + std::to_string(crc_version) + "\n");
        utilities::error_log("Spacewire::crc()\tunsupported CRC version. Should be f.");
        return 0x00;
    }
}

std::vector<uint8_t> SpaceWire::get_reply_data(std::vector<uint8_t> spw_reply) {
    size_t ether_prefix_length = 12; // using SPMU-001, this is always true
    size_t target_path_address_length = 0; // path address is removed by the time we receive reply.

    // now extract data based on data length field
    size_t data_length_start_offset = 9;
    size_t data_length_length = 3;

    if (ether_prefix_length + target_path_address_length + data_length_start_offset + data_length_length + 1 > spw_reply.size()) {
        // utilities::error_print("message too short to parse!\n");
        utilities::error_log("SpaceWire::get_reply_data()\ttoo short to parse");
        return {};
    }

    std::vector<uint8_t> data_length_vec(
        spw_reply.begin() 
            + ether_prefix_length 
            + target_path_address_length 
            + data_length_start_offset
            - 1, 
        spw_reply.begin() 
            + ether_prefix_length
            + target_path_address_length
            + data_length_start_offset
            + data_length_length
            - 1
    );

    // pre-pad `data_length_vec` with zero to use with `unsplat_from_4bytes()`
    std::vector<uint8_t> zero_prefix(1); 
    zero_prefix[0] = 0x00;
    data_length_vec.insert(data_length_vec.begin(), zero_prefix.begin(), zero_prefix.end());

    uint32_t data_length = utilities::unsplat_from_4bytes(data_length_vec);

    if (spw_reply.size() < ether_prefix_length + target_path_address_length + data_length_start_offset + data_length_length + data_length) {
        // utilities::error_print("can't read past end of reply!\n");
        utilities::error_log("SpaceWire::get_reply_data()\ttoo short to parse");
        return {};
    }

    // now read rest of spw_reply based on data_length
    std::vector<uint8_t> reply_data(
        spw_reply.begin()
            + ether_prefix_length
            + target_path_address_length
            + data_length_start_offset
            + data_length_length,
        spw_reply.begin()
            + ether_prefix_length
            + target_path_address_length
            + data_length_start_offset
            + data_length_length
            + data_length
    );

    return reply_data;
}

namespace utilities {
    void spw_print(std::vector<uint8_t> data, SpaceWire* spw) {
        // assumes a 12-B Ethernet header (SPMU-001) is prepended
        if (data.at(0) != 0x00) {
            // error_print("got malformed SpaceWire Ethernet header!\n");
            error_log("utilities::spw_print()\texpect zero at beginning of SpaceWire message.");
            hex_print(data);
            return;
        }
        
        size_t payload_size;
        size_t target_path_size = 0;
        size_t reply_path_size = 0;
        size_t less_target_path_size = data.size() - target_path_size;
        if (less_target_path_size < 26) {
            // error_print("SpaceWire message is impossibly short!\n");
            error_log("utilities::spw_print()\tSpaceWire message is too short.");
            hex_print(data);
            return;
        }

        std::vector<size_t> block_sizes;
        std::vector<char> block_cols;
        
        bool reply =  false;
        if (!spw) {
            reply = true;
            payload_size = data.size() - 12 - 12 - 1;
            block_sizes = {12, 1,1,1,1, 1,2,1,3,1, payload_size, 1};
            block_cols = {'n', 'r','y','n','n', 'r','n','n','n','n', 'b', 'n'};
        } else {
            target_path_size = spw->target_path_address.size();
            reply_path_size = spw->reply_path_address.size();
            less_target_path_size = data.size() - target_path_size;

            size_t instr_index = 11 + target_path_size + 3;
            if ((data.at(instr_index) & 0x20) != 0) {
                // write
                payload_size = data.size() - 12 - target_path_size - 4 - reply_path_size - 12 - 1;
                block_sizes = {12, target_path_size, 1,1,1,1, reply_path_size, 1,2,1,4,3,1, payload_size, 1};
                block_cols = {'n', 'b', 'r','y','n','n', 'b', 'r','n','n','n','n','n', 'b', 'n'};
            } else {
                // read
                payload_size = 0;
                block_sizes = {12, target_path_size, 1,1,1,1, reply_path_size, 1,2,1,4,3,1};
                block_cols = {'n', 'b', 'r','y','n','n', 'b', 'r','n','n','n','n','n'};
            }
        }

        size_t k = 0;
        for (size_t i = 0; i < block_sizes.size(); ++i) {
            for (size_t j = 0; j < block_sizes[i]; ++j) {

                switch (block_cols[i]) {
                    case 'n':
                        std::cout << "\033[0m";
                        break;
                    case 'y':
                        std::cout << "\033[37;43m";
                        break;
                    case 'r':
                        std::cout << "\033[37;41m";
                        break;
                    case 'b':
                        std::cout << "\033[37;44m";
                        break;
                    default:
                        error_print("spw_print() got unknown color code!\n");
                        return;
                }
                std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)(data[k] & 0xff) << "\033[0m";
                ++k;
            }
            std::cout << " ";
        }
        std::cout << "\033[0m\n";
    }
}

UART::UART(): DataLinkLayer() {
    tty_path = "";
    baud_rate = 9600;
    parity = 1;
    stop_bits = 1;
    data_bits = 8;
}

UART::UART(const UART &other):
    DataLinkLayer(other),
    tty_path(other.tty_path),
    baud_rate(other.baud_rate),
    parity(other.parity),
    stop_bits(other.stop_bits),
    data_bits(other.data_bits) { 

}

UART::UART(const DataLinkLayer &link_layer):
    DataLinkLayer(link_layer) {
    tty_path = "";
    baud_rate = 9600;
    parity = 1;
    stop_bits = 1;
    data_bits = 8;
}

UART::UART(std::string new_tty_path, uint32_t new_baud_rate, uint8_t new_parity, uint8_t new_stop_bits, uint8_t new_data_bits, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_static_footer_size) {
    tty_path = new_tty_path;
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

UART::UART(std::string new_tty_path, uint32_t new_baud_rate, uint8_t new_parity, uint8_t new_stop_bits, uint8_t new_data_bits, uint32_t new_mean_speed_bps, size_t new_max_payload_size, size_t new_frame_size, size_t new_static_header_size, size_t new_initial_header_size, size_t new_subsequent_header_size, size_t new_static_footer_size, size_t new_initial_footer_size, size_t new_subsequent_footer_size) {
    tty_path = new_tty_path;
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
        tty_path == other.tty_path
        && baud_rate == other.baud_rate 
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

std::string UART::to_string() {
    std::string result;
    result.append("UART::");
    result.append("\n\ttty_path \t= " + tty_path);
    result.append("\n\tbaud_rate \t= " + std::to_string(baud_rate));
    result.append("\n\tparity \t\t= " + std::to_string(parity));
    result.append("\n\tstop_bits \t= " + std::to_string(stop_bits));
    result.append("\n\tdata_bits \t= " + std::to_string(data_bits));
    result.append("\n");

    return result;
}