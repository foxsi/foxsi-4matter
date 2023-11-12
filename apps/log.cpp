#include "LineInterface.h"
#include "Logger.h"
#include <boost/asio.hpp>

int main(int argc, char* argv[]) {
    boost::asio::io_context context;
    
    LineInterface lif(argc, argv, context);
    
    
    return 0;
}