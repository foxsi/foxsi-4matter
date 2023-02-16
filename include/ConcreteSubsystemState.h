#ifndef CONCRETESUBSYSTEMSTATE_H
#define CONCRETESUBSYSTEMSTATE_H

#include "AbstractState.h"
#include "AbstractSubsystem.h"
#include "Parameters.h"
#include <boost/asio.hpp>


/* Concept
    Housekeeping, CdTe, CMOS etc run in sequence, triggered by Metronome.
    Ground::send() runs in parallel, interrupted by the ::send()s of the others.
    Ground::send() multicasts to GSE and EVTM.
    <Subsystem>::send()s forward commands and data requests to each subsystem, 
    directly (UART to Timepix) or via TCP/IP to SPMU-001/SpW for the others.
*/

class Ground: public AbstractState::AbstractState, AbstractSubsystem::AbstractSubsystem {
    public:
        Ground(std::string name, boost::asio::io_context& io_context);
        ~Ground();

        // ...they're not virtual anymore
        uint8_t* receive(uint8_t* addr);
        void send(uint8_t* addr, uint8_t* data);
        uint8_t* read(std::ifstream& file);
        void write(std::ofstream& file, uint8_t* data);

        void enter();
        void exit();
        void update();
    
        std::string name;
        boost::asio::ip::udp::endpoint local_endpoint;
        boost::asio::ip::udp::socket local_socket;
        boost::asio::ip::udp::endpoint remote_endpoint;     // there will be multiples: GSE + EVTM
};


class Housekeeping: public AbstractState::AbstractState, AbstractSubsystem::AbstractSubsystem {
    public:
        Housekeeping(std::string name, boost::asio::io_context& io_context);
        ~Housekeeping();

        uint8_t* receive(uint8_t* addr);
        void send(uint8_t* addr, uint8_t* data);
        uint8_t* read(std::ifstream& file);
        void write(std::ofstream& file, uint8_t* data);

        void enter();
        void exit();
        void update();
    
        std::string name;
        boost::asio::ip::udp::endpoint local_endpoint;
        boost::asio::ip::udp::socket local_socket;
        boost::asio::ip::udp::endpoint remote_endpoint;
};

class CdTe: AbstractState::AbstractState, AbstractSubsystem::AbstractSubsystem {
    public:
        CdTe();
        ~CdTe();

        uint8_t* receive(uint8_t* addr);
        void send(uint8_t* addr, uint8_t* data);
        uint8_t* read(std::ifstream& file);
        void write(std::ofstream& file, uint8_t* data);

        void enter();
        void exit();
        void update();
    
};

class CMOS: AbstractState::AbstractState, AbstractSubsystem::AbstractSubsystem {
    public:
        CMOS();
        ~CMOS();

        uint8_t* receive(uint8_t* addr);
        void send(uint8_t* addr, uint8_t* data);
        uint8_t* read(std::ifstream& file);
        void write(std::ofstream& file, uint8_t* data);

        void enter();
        void exit();
        void update();
    
};

class Timepix: AbstractState::AbstractState, AbstractSubsystem::AbstractSubsystem {
    public:
        Timepix();
        ~Timepix();

        uint8_t* receive(uint8_t* addr);
        void send(uint8_t* addr, uint8_t* data);
        uint8_t* read(std::ifstream& file);
        void write(std::ofstream& file, uint8_t* data);
        
        void enter();
        void exit();
        void update();
    
};

#endif