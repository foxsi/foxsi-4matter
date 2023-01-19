#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include <string>

class AbstractSubsystem {
    public:
        std::string name;

    public:
        AbstractSubsystem(std::string subsystem_name);
        // virtual uint8_t* read(uint8_t* addr);
        // virtual uint8_t* write(uint8_t* addr, uint8_t* data);
};

#endif