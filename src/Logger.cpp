#include "Logger.h"

Logger::Logger(
    std::string file_prefix, 
    log_modes new_log_mode, 
    std::vector<std::shared_ptr<SystemManager>> new_system_managers, 
    boost::asio::ip::udp::endpoint new_local_endpoint, 
    boost::asio::ip::udp::endpoint new_remote_endpoint, 
    boost::asio::io_context &context
): 
    local_socket(context),
    system_managers(new_system_managers), 
    local_endpoint(new_local_endpoint),
    remote_endpoint(new_remote_endpoint),
    log_mode(new_log_mode) 
{
    // make file names (with file prefix)
    // file name: prefix_systemname_timestamp.log
    size_t system_size = system_managers.size();
    std::string now_time = utilities::get_now_string();
    for (auto sys_man: system_managers) {
        for (auto ring_params: sys_man->system.ring_params) {
            // std::string this_filename = file_prefix + "_" + sys_man->system.name + "_" + RING_BUFFER_TYPE_OPTIONS_NAMES[ring_params.type] + "_" + now_time + ".log";

            // file_names.emplace(std::make_pair(sys_man, std::make_pair(ring_params.type, this_filename)));
        }
    }
    // try to open files
    // store file names and open files in maps
    
    // query system_managers to get expected data frame sizes
    // resize packet_buffers to expected data frame size
    
    // check if local_endpoint is multicast or not
    // open local socket appropriately
}
