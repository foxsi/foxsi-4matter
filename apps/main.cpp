
#include "AbstractSubsystem.h"
#include "Metronome.h"
#include <iostream>
#include <boost/asio.hpp>

int main(int argc, char** argv){
    std::cout << "maiiiinnnn...\n";
    // std::string name = "CdTe";
    // AbstractSubsystem sys = AbstractSubsystem(name);

    // std::cout << "AbstractSubsystem name:\t " << sys.get_name() << std::endl;

    std::cout << "max state: " << STATE_COUNT << "\n";

    boost::asio::io_context io_context;
    Metronome metronome(io_context);

    io_context.run();


    return 0;
}