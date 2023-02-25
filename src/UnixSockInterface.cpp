#include "UnixSockInterface.h"

UnixDomainSocketInterface::UnixDomainSocketInterface(std::string endpoint, boost::asio::io_context& context):
    local_socket(context), 
    remote_socket(context) {

    local_socket.connect(endpoint);
}
