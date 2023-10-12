#include "Systems.h"
#include "DataLinkLayer.h"
#include <iostream>

System::System(): uart(nullptr),spacewire(nullptr),ethernet(nullptr) {
    name = "";
    hex = 0xff;
    type = COMMAND_TYPE_OPTIONS::NONE;
}

System::System(std::string new_name, uint8_t new_hex): uart(nullptr),spacewire(nullptr),ethernet(nullptr) {
    name = new_name;
    hex = new_hex;
    type = COMMAND_TYPE_OPTIONS::NONE;
}

System::System(std::string new_name, uint8_t new_hex, COMMAND_TYPE_OPTIONS new_type, UART* new_uart, SpaceWire* new_spacewire, Ethernet* new_ethernet):
    uart(new_uart),
    spacewire(new_spacewire),
    ethernet(new_ethernet) {
    name = new_name;
    hex = new_hex;
    type = new_type;
}

System::System(std::string new_name, uint8_t new_hex, COMMAND_TYPE_OPTIONS new_type, UART *new_uart, SpaceWire *new_spacewire, Ethernet *new_ethernet, std::vector<RingBufferParameters> new_ring_params):
    uart(new_uart),
    spacewire(new_spacewire),
    ethernet(new_ethernet),
    ring_params(new_ring_params) {
    name = new_name;
    hex = new_hex;
    type = new_type;
}

System::System(std::string new_name, uint8_t new_hex, UART* new_uart):
    uart(new_uart),
    spacewire(nullptr),
    ethernet(nullptr) {
    name = new_name;
    hex = new_hex;
    type = COMMAND_TYPE_OPTIONS::UART;
}

System::System(std::string new_name, uint8_t new_hex, SpaceWire* new_spacewire):
    spacewire(new_spacewire),
    ethernet(nullptr),
    uart(nullptr) {
    name = new_name;
    hex = new_hex;
    type = COMMAND_TYPE_OPTIONS::SPW;
}

System::System(std::string new_name, uint8_t new_hex, Ethernet* new_ethernet):
    ethernet(new_ethernet),
    spacewire(nullptr),
    uart(nullptr) {
    name = new_name;
    hex = new_hex;
    type = COMMAND_TYPE_OPTIONS::ETHERNET;
}

// System::System(const System &other): 
//     name(other.name),
//     hex(other.hex),
//     type(other.type),
//     ethernet(other.ethernet),
//     spacewire(other.spacewire),
//     uart(other.uart),
//     ring_params(other.ring_params) {
// }

// System::System(System &&other):
//     name(std::move(other.name)),
//     hex(std::move(other.hex)),
//     type(std::move(other.type)),
//     uart(std::move(other.uart)),
//     spacewire(std::move(other.spacewire)),
//     ethernet(std::move(other.ethernet)),
//     ring_params(std::move(other.ring_params)) {
// }

System &System::operator=(const System &other) {
    // TODO: insert return statement here
    name = other.name;
    hex = other.hex;
    type = other.type;
    uart = other.uart;
    ethernet = other.ethernet;
    spacewire = other.spacewire;
    ring_params = other.ring_params;

    return *this;
}

bool System::operator==(const System &other) const {
    if (hex == other.hex) {
        return true;
    } else {
        return false;
    }
}

void System::print() {
    std::cout << "system " + name + " has code ";
    utilities::hex_print(hex);
    std::cout << "\n\tcommand type: ";
    switch (type) {
        case COMMAND_TYPE_OPTIONS::ETHERNET:
            std::cout << "Ethernet\n";
            break;
        case COMMAND_TYPE_OPTIONS::SPW:
            std::cout << "SpaceWire\n";
            break;
        case COMMAND_TYPE_OPTIONS::SPI:
            std::cout << "SPI\n";
            break;
        case COMMAND_TYPE_OPTIONS::UART:
            std::cout << "UART\n";
            break;
        case COMMAND_TYPE_OPTIONS::NONE:
            std::cout << "none\n";
            break;
        default:
            std::cout << "found no type!\n";
    }

    // if (ethernet) {
    //     std::cout << "\tEthernet interface: ";
    //     std::cout << ethernet->to_string;
    // }
    // if (spacewire) {
    //     std::cout << "\tSpaceWire interface: ";
    //     std::cout << ""
    // }
    // if (uart) {

    // }
    // if (ring_params) {

    // }
}

size_t System::get_frame_size() {
    switch(type) {
        case COMMAND_TYPE_OPTIONS::ETHERNET:
            if (ethernet) {
                return ethernet->frame_size;
            }
        case COMMAND_TYPE_OPTIONS::SPW:
            if (spacewire) {
                return spacewire->frame_size;
            }
        case COMMAND_TYPE_OPTIONS::UART:
            if (uart) {
                return uart->frame_size;
            }
        case COMMAND_TYPE_OPTIONS::NONE:
            return 0;
        default:
            return 0;
    }
    utilities::error_print("System frame size lookup fell through!\n");
    return 0;
}

size_t System::get_frame_size(RING_BUFFER_TYPE_OPTIONS buffer_type) {
    size_t result = 0;
    if (spacewire) {
        try {
            result = ring_params[static_cast<uint8_t>(buffer_type)].frame_size_bytes;
        } catch (std::exception& e) {
            utilities::error_print("System frame size lookup out of range!\n");
            result = 0;
        }
    } else {
        utilities::error_print("System frame size lookup for ring buffer, but no SpaceWire* interface present!\n");
    }
    return result;
}
