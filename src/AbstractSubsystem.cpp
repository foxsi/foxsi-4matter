#include "AbstractSubsystem.h"

// need to define all this stuff for the linker, even though it's all virtual :/

AbstractSubsystem::AbstractSubsystem(std::string subsystem_name) {
    name = subsystem_name;
}

AbstractSubsystem::~AbstractSubsystem() {}

uint8_t* AbstractSubsystem::read(uint8_t* addr) {}
uint8_t* AbstractSubsystem::read_to_memory(uint8_t* addr, SharedMemory* memory) {}
uint8_t* AbstractSubsystem::write(uint8_t* addr, uint8_t* data) {}

std::string AbstractSubsystem::get_name() {}