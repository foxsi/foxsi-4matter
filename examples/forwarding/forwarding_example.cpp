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
        10000,
        10001,
        context
    );

    // Ground ground(
    //     durations,
    //     flags,
    //     local_ip,
    //     local_ground_port,
    //     ground_ip,
    //     local_ground_port,
    //     context
    // );

    // Housekeeping hk(
    //     HOUSEKEEPING,
    //     CMD_SEND,
    //     ground,
    //     durations,
    //     flags,
    //     local_ip,
    //     local_subsys_port,
    //     ground_ip,
    //     local_subsys_port,
    //     context
    // );

    // std::vector<std::vector<char>> test_buffer(100, std::vector<char>(RECV_BUFF_LEN));
    // auto* downlinq = &test_buffer;
    // hk.recv(test_buffer);

    context.run();

    return 0;
}