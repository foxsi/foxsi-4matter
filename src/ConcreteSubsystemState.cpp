#include "ConcreteSubsystemState.h"




Housekeeping::Housekeeping(std::string name, boost::asio::io_context& io_context): local_socket(io_context) {
    name = name;
}

