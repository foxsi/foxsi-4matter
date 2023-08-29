#pragma once
#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <map>
#include <boost/interprocess/shared_memory_object.hpp>

/**
 * @deprecated unused, ignore.
 * 
 */
class SharedMemory{
    // memory map: takes string keys and reads/writes the memory for the key
    std::map<std::string, boost::interprocess::shared_memory_object> map;
    
    public:
        // just supply the std::map defining the memory map
        SharedMemory(std::map<std::string, boost::interprocess::shared_memory_object> memory_map);
        ~SharedMemory();

        // writes could also draw from shared memory, i.e. command buffer.
        bool write(std::string key, uint8_t* data);
        // reads should copy from shared memory---not returning Boost shared memory object, and not returning pointer
        uint8_t* read(std::string key);
};

#endif