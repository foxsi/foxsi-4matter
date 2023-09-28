#include "Ticker.h"
#include "Utilities.h"

Ticker::Ticker(
    std::vector<TransportLayerMachine*> systems, 
    double& loop_period,
    std::map<STATE_ORDER, double>& duration,
    std::ifstream& log,
    std::map<std::string, std::string>& flag,
    std::vector<std::vector<uint8_t>>& command,
    std::queue<std::vector<uint8_t>>& down
):  
    subsystems(systems), 
    period(loop_period),  
    durations(duration),
    log_file(log),
    flags(flag),
    commands(command),
    downlink(down)

{
    commands_msg_size = commands.front().size();
    downlink_msg_size = commands.front().size();

    state = 0;
    subsystem = 0;
}

void Ticker::tick(double& duration_millis) {
    switch(state) {
        case 0:     // send command
            // if using this calling scheme, would need to sort through commands inside subsystems' senders
            subsystems[subsystem]->send(commands);
            break;
        case 1:     // send data request
            break;
        case 2:     // receive data/resend to ground
            subsystems[subsystem]->recv(downlink);
                // internally, subsystems' implementation of recv (or async_recv) should forward to ground.
            break;
        case 3:      // idle
            
            break;

        default:
            std::cerr << "state switch/case fell through\n";
    }
    subsystems[subsystem];


    // advance state
    state = inc_mod(state, durations.size());
    if (state == 0) {
        inc_mod(subsystem, subsystems.size());
    }
}
