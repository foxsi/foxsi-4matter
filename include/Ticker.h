#ifndef TICKER_H
#define TICKER_H

#include "Subsystem.h"
#include <queue>

class Ticker {
    // believe that "MyClass*" syntax in std::vector allows any class inheriting MyClass to be used.
    std::vector<TransportLayerMachine*> subsystems;
    double& period;                                 // period of full loop
    std::map<STATE_ORDER, double>& durations;       // max time to spend in each state
        // get the durations std::map from a higher level map that stores fraction of period (or desired data volumes per period) for each state.

    unsigned int state;                             // index to subsystems vector
    unsigned int subsystem;                         // index to durations map
    std::ifstream& log_file;                        // log file pointer
    std::map<std::string, std::string>& flags;      // software error/setting flags (TBR)
    std::vector<std::vector<uint8_t>>& commands;        // commands sent via uplink
    std::queue<std::vector<uint8_t>>& downlink;        // downlink commands

    std::size_t commands_msg_size;                  // size of packet used for uplink commands (<1500B)
    std::size_t downlink_msg_size;                  // size of packet used for downlink messages (<1500B)

    Ticker(
        std::vector<TransportLayerMachine*> systems, 
        double& period,
        std::map<STATE_ORDER, double>& duration,
        std::ifstream& log,
        std::map<std::string, std::string>& flag,
        std::vector<std::vector<uint8_t>>& command,
        std::queue<std::vector<uint8_t>>& down
    );
    
    ~Ticker();
    void tick(double& duration_millis);
        // in here, loop call subsystems[i].recv()/send() on pointer to commands or downlink queue.
};

#endif