#ifndef UARTINTERFACE_H
#define UARTINTERFACE_H

#include "AbstractSerial.h"

class UARTInterface: AbstractSerial {
    public:
        unsigned long baud;
        unsigned short parity;
        
        // add share memory interface attribute

        UARTInterface();

        ~UARTInterface();

        int recv(uint8_t* addr, char* buffer);
        int async_recv(uint8_t* addr, char* buffer);

        int send(uint8_t* addr, char* buffer);
        int async_send(uint8_t* addr, char* buffer);
};

#endif