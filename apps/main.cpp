
#include "Metronome.h"
#include "Parameters.h"
#include <iostream>
#include <boost/asio.hpp>

int main(int argc, char** argv){
    std::cout << "maiiiinnnn...\n";
    // std::string name = "CdTe";

    std::cout << "max state: " << static_cast<unsigned short>(STATE_ORDER::STATE_COUNT) << "\n";

    boost::asio::io_context io_context;
    double period = 1.0;
    Metronome metronome(period, io_context);

    io_context.run();


    return 0;
}