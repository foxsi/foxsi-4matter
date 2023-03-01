#include "Subsystem.h"
#include "Ticker.h"
#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <string>

int main() {
    
    // for gse-based testing; flip these if running on formatter processor
    std::string local_ip = "192.168.1.100";
    std::string ground_ip = "192.168.1.8";

    unsigned short local_ground_port = 9999;
    unsigned short local_subsys_port = 10000;

    boost::asio::io_context context;

    std::map<STATE_ORDER, double> durations{
        {CMD_SEND, 1.0},
        {DATA_REQ, 1.0},
        {DATA_RECV, 1.0},
        {IDLE, 1.0}
    };

    std::map<std::string, std::string> flags{
        {"flag0", "no"}
    };

    PepperMill frmtr(
        "192.168.1.100",
        "192.168.1.8",
        "192.168.1.8",
        9999,
        9999,
        10001,
        context
    );

    frmtr.recv_tcp_fwd_udp();
    context.run();

    return 0;
}