#ifndef UARTINTERFACE_H
#define UARTINTERFACE_H

class UARTInterface {
    public:
        unsigned long baud;
        unsigned short parity;
        
        // add share memory interface attribute

        UARTInterface();

        ~UARTInterface();

        int recv(uint8_t* addr, uint8_t* buffer);
        int async_recv(uint8_t* addr, uint8_t* buffer);

        int send(uint8_t* addr, uint8_t* buffer);
        int async_send(uint8_t* addr, uint8_t* buffer);
};

#endif