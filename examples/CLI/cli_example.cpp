#include "LineInterface.h"
#include <boost/asio.hpp>

int main(int argc, char* argv[]) {
    LineInterface lif(argc, argv);
    boost::asio::io_context context;

    context.run();
    return 0;
}