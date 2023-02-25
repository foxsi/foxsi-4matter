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

class Ground: public AbstractState::AbstractState, public UDPInterface::UDPInterface {
    public:
        SUBSYSTEM_ORDER name;
        // also inherits STATE_ORDER state from AbstractState

        Ground(SUBSYSTEM_ORDER name, STATE_ORDER state, boost::asio::io_context& io_context);
        ~Ground();

        // inherit the UDPInterface
        // send, recv, etc
        // @todo multicast send

        // realize these here:
        void enter();
        void exit();
        void update();
};


class Housekeeping: public AbstractState::AbstractState, public UDPInterface::UDPInterface {
    public:
        SUBSYSTEM_ORDER name;

        Housekeeping(SUBSYSTEM_ORDER name, STATE_ORDER state, std::string local_ip, unsigned short local_port, std::string remote_ip, unsigned short remote_port,boost::asio::io_context& io_context);

        void enter();
        void exit();
        void update();
};

class CdTe: AbstractState::AbstractState, public TCPInterface::TCPInterface {
    public:
        SUBSYSTEM_ORDER name;
        boost::asio::ip::tcp::endpoint ground_endpoint;

        CdTe(
            SUBSYSTEM_ORDER sys_name, 
            STATE_ORDER initial_state, 
            std::string local_ip, 
            unsigned short local_port, 
            std::string remote_ip, 
            unsigned short remote_port, 
            boost::asio::ip::tcp::endpoint& ground, 
            boost::asio::io_context& io_context);
        ~CdTe();

        void forward_to_ground();

        void enter();
        void exit();
        void update();
    
};

class CMOS: AbstractState::AbstractState, public TCPInterface::TCPInterface {
    public:
        SUBSYSTEM_ORDER name;

        CMOS();
        ~CMOS();

        void enter();
        void exit();
        void update();
    
};

class Timepix: AbstractState::AbstractState, public UARTInterface::UARTInterface {
    public:
        SUBSYSTEM_ORDER name;
        
        Timepix();
        ~Timepix();
        
        void enter();
        void exit();
        void update();
    
};

#endif