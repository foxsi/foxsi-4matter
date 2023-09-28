#include <iostream>
#include "Metronome.h"
#include "LineInterface.h"
#include "Commanding.h"

#include <boost/asio.hpp>

int main(int argc, char* argv[]) {
    boost::asio::io_context context;
    LineInterface lif(argc, argv, context);

    return 0;
}