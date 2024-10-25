/**
 * @file Systems.h
 * @author Thanasi Pantazides
 * @brief Container for (ideally) runtime-constant configuration data for onboard systems.
 * @version v1.0.1
 * @date 2024-03-11 
 */

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

/**
 * @brief Store configuration data describing a `System`.
 * 
 * This class is intended to capture runtime-constant configuration data. A unique `name` and `hex` identifier are provided for the system. The object may point to one or more `DataLinkLayer`-derived objects describing physical connections the `System` can use for communication—the interface that should be used for communication is identified by `System::type`.
 * 
 * These objects may (or may not) contain a non-`nullptr` reference to a `DataLinkLayer`-derived object. Alternative designs would be:
 *  1. A generic `DataLinkLayer*` member which would specialize to a derived type. This was not chosen because a `System` may have multiple valid `DataLinkLayer` interfaces.
 *  2. A `std::vector<DataLinkLayer*>` which contains any number of interfaces for the `System`. This was not chosen because a `System` was assumed to have only one of each `DataLinkLayer`-derived interface of a given type. Maybe this should be reconsidered in the future.
 */
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
        System& operator=(const System& other);
        bool operator==(const System& other) const;
        void print();

        /**
         * @brief Get frame size from the interface in `System::type`.
         * @return size of frame.
         */
        size_t get_frame_size();
        /**
         * @brief Get frame size from the `System::ring_params` for the provided `buffer_type`.
         * @param buffer_type type of data to check frame size for.
         * @return size_t frame size, or zero if the requested `buffer_type` is not a key in `ring_params`.
         */
        size_t get_frame_size(RING_BUFFER_TYPE_OPTIONS buffer_type);

        /**
         * @brief A unique (among all `System`s) name for this object.
         */
        std::string name;
        /**
         * @brief A unique (among all `System`s) ID for this object. 
         * @note For FOXSI-4, all `System::hex` values range from `0x00` to `0x0f`, so can be written in 4 bits. This reduction in size is not used for anything critical, but it may be someday.
         */
        uint8_t hex;
        /**
         * @brief The interface used for communication with this `System`.
         */
        COMMAND_TYPE_OPTIONS type;

        /**
         * @brief An optional pointer to a `UART` interface configuration for this object.
         * @warning May be `nullptr`.
         */
        UART* uart;
        /**
         * @brief An optional pointer to a `SpaceWire` interface configuration for this object.
         * @warning May be `nullptr`.
         */
        SpaceWire* spacewire;
        /**
         * @brief An optional pointer to a `Ethernet` interface configuration for this object.
         * @warning May be `nullptr`.
         */
        Ethernet* ethernet;

        /**
         * @brief A map used to look up `RingBufferParameters` by their type.
         * 
         * Used in the object to retrieve ring buffer-specific configuration data for a given data type that the `System` supports.
         */
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