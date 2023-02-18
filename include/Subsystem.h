#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include "AbstractState.h"
#include "UDPInterface.h"
#include "TCPInterface.h"
#include "UARTInterface.h"
#include "Parameters.h"
#include <boost/asio.hpp>



/*  Concept
    Housekeeping, CdTe, CMOS etc run in sequence, triggered by Metronome.
    Ground::send() runs in parallel, interrupted by the ::send()s of the others.
    Ground::send() multicasts to GSE and EVTM.
    <Subsystem>::send()s forward commands and data requests to each subsystem, 
    directly (UART to Timepix) or via TCP/IP to SPMU-001/SpW for the others.

    Decide if Housekeeping (plenum board) will be UDP or TCP
*/

class Ground: public AbstractState::AbstractState, UDPInterface::UDPInterface {
    public:
        std::string name;
        // also inherits STATE_ORDER state from AbstractState

        Ground(std::string name, boost::asio::io_context& io_context);
        ~Ground();

        // inherit the UDPInterface
        // send, recv, etc
        // @todo multicast send

        // realize these here:
        void enter();
        void exit();
        void update();
};


class Housekeeping: public AbstractState::AbstractState, UDPInterface::UDPInterface {
    public:
        std::string name;

        Housekeeping(std::string name, boost::asio::io_context& io_context);
        ~Housekeeping();

        void enter();
        void exit();
        void update();
};

class CdTe: AbstractState::AbstractState, TCPInterface::TCPInterface {
    public:
        std::string name;

        CdTe();
        ~CdTe();

        void enter();
        void exit();
        void update();
    
};

class CMOS: AbstractState::AbstractState, TCPInterface::TCPInterface {
    public:
        std::string name;

        CMOS();
        ~CMOS();

        void enter();
        void exit();
        void update();
    
};

class Timepix: AbstractState::AbstractState, UARTInterface::UARTInterface {
    public:
        std::string name;
        
        Timepix();
        ~Timepix();
        
        void enter();
        void exit();
        void update();
    
};

#endif