#include "Buffers.h"
#include "Commanding.h"
#include "Parameters.h"
#include "Utilities.h"
#include "LineInterface.h"
#include <boost/asio.hpp>
#include <iostream>
#include <queue>

int main(int argc, char* argv[]) {
    boost::asio::io_context context;

    LineInterface lif(argc, argv, context);
    System sys = lif.systems[0];

    DownlinkBufferElement a(sys, 100);
    a.set_num_packets_in_frame(20);
    a.set_this_packet_index(1);
    a.set_payload({0x00,0x01,0x02,0x03,0x04});
    std::vector<uint8_t> pre_queue_payload = a.get_payload();
    std::vector<uint8_t> pre_queue_packet = a.get_packet();
    
    std::cout << "payload before enqueueing: \n";
    hex_print(pre_queue_payload);
    std::cout << "packet before enqueueing: \n";
    hex_print(pre_queue_packet);
    std::cout << "\n";

    std::queue<DownlinkBufferElement> q_downlink;

    for(int i = 0; i < 0x64; ++i) {
        q_downlink.push(a);
    }
    std::cout << "pushed " << q_downlink.size() << " elements onto the downlink queue\n";

    DownlinkBufferElement b = q_downlink.front();
    q_downlink.pop();
    std::cout << "popped packet: \n";
    std::vector<uint8_t> queue_out_packet = b.get_packet();
    hex_print(queue_out_packet);
    
    return 0;
}