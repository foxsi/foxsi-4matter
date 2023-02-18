#pragma once
#ifndef ABSTRACTSERIAL_H
#define ABSTRACTSERIAL_H

#include "SharedMemory.h"
#include "Parameters.h"
#include <string>
#include <iostream>
#include <fstream>

// #include <boost/interprocess/shared_memory_object.hpp>

// The syntax virtual <return type> <function name>(args) = 0 is a pure virtual function
// i.e. the concrete class that extends must implement the function
class AbstractSerial {
    protected:
        std::string name;

    public:
        AbstractSerial();
        virtual ~AbstractSerial() = 0;

        virtual int recv(uint8_t* addr, uint8_t* buffer) = 0;
        virtual int async_recv(uint8_t* addr, uint8_t* buffer) = 0;

        // virtual uint8_t* recv_to_memory(uint8_t* addr, SharedMemory* memory) = 0;
        virtual int send(uint8_t* addr, uint8_t* data) = 0;
        virtual int async_send(uint8_t* addr, uint8_t* data) = 0;

        // virtual uint8_t* read(std::ifstream& file) = 0;
        // virtual void write(std::ofstream& file, uint8_t* data) = 0;
    
    private:

};

#endif