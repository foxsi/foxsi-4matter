
#include "Metronome.h"
#include "Parameters.h"
#include <iostream>
#include <boost/asio.hpp>

int main(int argc, char** argv){

    std::cout << "max state: " << static_cast<unsigned short>(STATE_ORDER::STATE_COUNT) << "\n";

    std::unordered_map<SUBSYSTEM_ORDER, std::unordered_map<STATE_ORDER, double>> lookup_periods;

    lookup_periods[SUBSYSTEM_ORDER::CDTE_1][STATE_ORDER::CMD_SEND] = 1*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_1][STATE_ORDER::DATA_REQ] = 2*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_1][STATE_ORDER::DATA_RECV] = 3*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_1][STATE_ORDER::DATA_CHECK] = 4*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_1][STATE_ORDER::DATA_STORE] = 5*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_1][STATE_ORDER::IDLE] = 6*1000;

    lookup_periods[SUBSYSTEM_ORDER::CDTE_2][STATE_ORDER::CMD_SEND] = 1*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_2][STATE_ORDER::DATA_REQ] = 2*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_2][STATE_ORDER::DATA_RECV] = 3*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_2][STATE_ORDER::DATA_CHECK] = 4*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_2][STATE_ORDER::DATA_STORE] = 5*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_2][STATE_ORDER::IDLE] = 6*1000;

    lookup_periods[SUBSYSTEM_ORDER::CDTE_3][STATE_ORDER::CMD_SEND] = 1*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_3][STATE_ORDER::DATA_REQ] = 2*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_3][STATE_ORDER::DATA_RECV] = 3*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_3][STATE_ORDER::DATA_CHECK] = 4*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_3][STATE_ORDER::DATA_STORE] = 5*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_3][STATE_ORDER::IDLE] = 6*1000;

    lookup_periods[SUBSYSTEM_ORDER::CDTE_4][STATE_ORDER::CMD_SEND] = 1*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_4][STATE_ORDER::DATA_REQ] = 2*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_4][STATE_ORDER::DATA_RECV] = 3*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_4][STATE_ORDER::DATA_CHECK] = 4*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_4][STATE_ORDER::DATA_STORE] = 5*1000;
    lookup_periods[SUBSYSTEM_ORDER::CDTE_4][STATE_ORDER::IDLE] = 6*1000;

    lookup_periods[SUBSYSTEM_ORDER::CMOS_1][STATE_ORDER::CMD_SEND] = 1*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_1][STATE_ORDER::DATA_REQ] = 2*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_1][STATE_ORDER::DATA_RECV] = 3*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_1][STATE_ORDER::DATA_CHECK] = 4*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_1][STATE_ORDER::DATA_STORE] = 5*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_1][STATE_ORDER::IDLE] = 6*1000;

    lookup_periods[SUBSYSTEM_ORDER::CMOS_2][STATE_ORDER::CMD_SEND] = 1*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_2][STATE_ORDER::DATA_REQ] = 2*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_2][STATE_ORDER::DATA_RECV] = 3*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_2][STATE_ORDER::DATA_CHECK] = 4*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_2][STATE_ORDER::DATA_STORE] = 5*1000;
    lookup_periods[SUBSYSTEM_ORDER::CMOS_2][STATE_ORDER::IDLE] = 6*1000;

    lookup_periods[SUBSYSTEM_ORDER::TIMEPIX][STATE_ORDER::CMD_SEND] = 1*1000;
    lookup_periods[SUBSYSTEM_ORDER::TIMEPIX][STATE_ORDER::DATA_REQ] = 2*1000;
    lookup_periods[SUBSYSTEM_ORDER::TIMEPIX][STATE_ORDER::DATA_RECV] = 3*1000;
    lookup_periods[SUBSYSTEM_ORDER::TIMEPIX][STATE_ORDER::DATA_CHECK] = 4*1000;
    lookup_periods[SUBSYSTEM_ORDER::TIMEPIX][STATE_ORDER::DATA_STORE] = 5*1000;
    lookup_periods[SUBSYSTEM_ORDER::TIMEPIX][STATE_ORDER::IDLE] = 6*1000;

    lookup_periods[SUBSYSTEM_ORDER::HOUSEKEEPING][STATE_ORDER::CMD_SEND] = 1*1000;
    lookup_periods[SUBSYSTEM_ORDER::HOUSEKEEPING][STATE_ORDER::DATA_REQ] = 2*1000;
    lookup_periods[SUBSYSTEM_ORDER::HOUSEKEEPING][STATE_ORDER::DATA_RECV] = 3*1000;
    lookup_periods[SUBSYSTEM_ORDER::HOUSEKEEPING][STATE_ORDER::DATA_CHECK] = 4*1000;
    lookup_periods[SUBSYSTEM_ORDER::HOUSEKEEPING][STATE_ORDER::DATA_STORE] = 5*1000;
    lookup_periods[SUBSYSTEM_ORDER::HOUSEKEEPING][STATE_ORDER::IDLE] = 6*1000;
    

    boost::asio::io_context io_context;
    double period = 1.0;
    Metronome metronome(period, lookup_periods, io_context);

    io_context.run();


    return 0;
}