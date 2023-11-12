#ifndef SYSTEMS_H
#define SYSTEMS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "DataLinkLayer.h"
#include "DirectMemoryAccess.h"
#include "Utilities.h"
#include "Parameters.h"

class System {
    public:
        System();
        System(std::string new_name, uint8_t new_hex);
        System(
            std::string new_name, 
            uint8_t new_hex, 
            COMMAND_TYPE_OPTIONS new_type,
            UART* new_uart,
            SpaceWire* new_spacewire,
            Ethernet* new_ethernet
        );
        System(
            std::string new_name, 
            uint8_t new_hex, 
            COMMAND_TYPE_OPTIONS new_type,
            UART* new_uart,
            SpaceWire* new_spacewire,
            Ethernet* new_ethernet,
            std::unordered_map<RING_BUFFER_TYPE_OPTIONS, RingBufferParameters> new_ring_params
        );
        System(
            std::string new_name, 
            uint8_t new_hex, 
            UART* new_uart
        );
        System(
            std::string new_name, 
            uint8_t new_hex, 
            SpaceWire* new_spacewire
        );
        System(
            std::string new_name, 
            uint8_t new_hex, 
            Ethernet* new_ethernet
        );
        // System(const System& other);
        // System(System&& other);
        System& operator=(const System& other);
        bool operator==(const System& other) const;
        void print();
        size_t get_frame_size();
        size_t get_frame_size(RING_BUFFER_TYPE_OPTIONS buffer_type);

        std::string name;
        uint8_t hex;
        COMMAND_TYPE_OPTIONS type;

        UART* uart;
        SpaceWire* spacewire;
        Ethernet* ethernet;
        // todo: add `Timing* timing;` as a field?

        std::unordered_map<RING_BUFFER_TYPE_OPTIONS, RingBufferParameters> ring_params;
    
    private:
};

template<>
struct std::hash<System> {
    std::size_t operator()(const System& sys) const {
        // http://stackoverflow.com/a/1646913/126995
        // std::size_t res = 17;
        // res = 31*res + std::hash<uint8_t>()(sys.hex);

        // don't actually need to hash. `System::hex` should always be unique.
        std::size_t res = sys.hex;
        return res;
    }
};

#endif