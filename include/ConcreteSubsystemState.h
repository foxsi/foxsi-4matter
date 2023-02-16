#ifndef CONCRETESUBSYSTEMSTATE_H
#define CONCRETESUBSYSTEMSTATE_H

#include "AbstractState.h"
#include "AbstractSubsystem.h"
#include "Parameters.h"

class Housekeeping: AbstractState::AbstractState, AbstractSubsystem::AbstractSubsystem {
    public:
        Housekeeping();
        ~Housekeeping();

        // ...they're not virtual anymore
        uint8_t* receive(uint8_t* addr) = 0;
        void send(uint8_t* addr, uint8_t* data) = 0;
        uint8_t* read(std::ifstream& file) = 0;
        void write(std::ofstream& file, uint8_t* data) = 0;;

        void enter();
        void exit();
        void update();
    
};

class CdTe: AbstractState::AbstractState, AbstractSubsystem::AbstractSubsystem {
    public:
        CdTe();
        ~CdTe();

        // ...they're not virtual anymore
        uint8_t* read(uint8_t* addr);
        uint8_t* read_to_memory(uint8_t* addr, SharedMemory* memory);
        uint8_t* write(uint8_t* addr, uint8_t* data);

        void enter();
        void exit();
        void update();
    
};

class CMOS: AbstractState::AbstractState, AbstractSubsystem::AbstractSubsystem {
    public:
        CMOS();
        ~CMOS();

        uint8_t* receive(uint8_t* addr) = 0;
        void send(uint8_t* addr, uint8_t* data) = 0;
        uint8_t* read(std::ifstream& file) = 0;
        void write(std::ofstream& file, uint8_t* data) = 0;;

        void enter();
        void exit();
        void update();
    
};

class Timepix: AbstractState::AbstractState, AbstractSubsystem::AbstractSubsystem {
    public:
        Timepix();
        ~Timepix();

        uint8_t* receive(uint8_t* addr) = 0;
        void send(uint8_t* addr, uint8_t* data) = 0;
        uint8_t* read(std::ifstream& file) = 0;
        void write(std::ofstream& file, uint8_t* data) = 0;;

        void enter();
        void exit();
        void update();
    
};

#endif