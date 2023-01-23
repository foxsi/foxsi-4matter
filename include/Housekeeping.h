#ifndef HOUSEKEEPING_H
#define HOUSEKEEPING_H

#include "AbstractSubsystem.h"

class Housekeeping: AbstractSubsystem::AbstractSubsystem {
    protected:
        std::string name;

    public:
        Housekeeping(std::string subsystem_name);
        virtual ~Housekeeping();

        // ...they're not virtual anymore
        uint8_t* read(uint8_t* addr);
        uint8_t* read_to_memory(uint8_t* addr, SharedMemory* memory);
        uint8_t* write(uint8_t* addr, uint8_t* data);

        std::string get_name();
    
    private:
};

#endif