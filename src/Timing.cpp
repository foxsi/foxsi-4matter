#include "Timing.h"


Timing::Timing() {
    period_millis = 0;
    command_millis = 0;
    request_millis = 0;
    reply_millis = 0;
    idle_millis = 0;
    retry_max_count = 1;
}

void Timing::add_times_seconds(double total_allocation_seconds, double command_time_seconds, double request_time_seconds, double reply_time_seconds, double idle_time_seconds) {
    period_millis = (uint32_t)(total_allocation_seconds*1000);
    command_millis = (uint32_t)(command_time_seconds*1000);
    request_millis = (uint32_t)(request_time_seconds*1000);
    reply_millis = (uint32_t)(reply_time_seconds*1000);
    idle_millis = (uint32_t)(idle_time_seconds*1000);

    Timing::resolve_times();
}

void Timing::resolve_times() {
    double sum = command_millis + request_millis + reply_millis + idle_millis;
    command_millis = command_millis*period_millis/sum;
    request_millis = request_millis*period_millis/sum;
    reply_millis = reply_millis*period_millis/sum;
    idle_millis = idle_millis*period_millis/sum;
}

const std::string Timing::to_string() {

    std::string result;
    result.append("Timing::");
    result.append("\n\tperiod \t\t\t= " + std::to_string(period_millis) + " ms");
    result.append("\n\tcommand \t\t= " + std::to_string(command_millis) + " ms");
    result.append("\n\trequest \t\t= " + std::to_string(request_millis) + " ms");
    result.append("\n\treply \t\t\t= " + std::to_string(reply_millis) + " ms");
    result.append("\n\tidle \t\t\t= " + std::to_string(idle_millis) + " ms");
    result.append("\n\tretry_max_count \t= " + std::to_string(retry_max_count));
    result.append("\n\ttimeout \t\t= " + std::to_string(timeout_millis) + " ms");
    result.append("\n\tintercommand_space \t= " + std::to_string(intercommand_space_millis) + " ms");
    result.append("\n");
    return result;
}
