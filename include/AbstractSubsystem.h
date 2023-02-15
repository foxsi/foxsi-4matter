#pragma once
#ifndef ABSTRACTSUBSYSTEM_H
#define ABSTRACTSUBSYSTEM_H

#include <string>
#include "SharedMemory.h"
// #include <boost/interprocess/shared_memory_object.hpp>

class AbstractSubsystem{
    protected:
        std::string name;

    public:
        AbstractSubsystem(std::string subsystem_name);
        virtual ~AbstractSubsystem();

        virtual uint8_t* read(uint8_t* addr);
        virtual uint8_t* read_to_memory(uint8_t* addr, SharedMemory* memory);
        virtual uint8_t* write(uint8_t* addr, uint8_t* data);

        virtual std::string get_name();
    
    private:

};

#endif