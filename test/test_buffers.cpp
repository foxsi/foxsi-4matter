#include "Buffers.h"
#include "Commanding.h"
#include "Parameters.h"
#include "Utilities.h"
#include "LineInterface.h"
#include <boost/asio.hpp>
#include <iostream>
#include <algorithm>
#include <queue>
#include "moodycamel/concurrentqueue.h"

int main(int argc, char* argv[]) {
    boost::asio::io_context context;

    LineInterface lif(argc, argv, context);
    System sys = lif.systems[0];

    CommandDeck deck = lif.get_command_deck();

    // setup test of buffer pipe
    System cdte1 = deck.get_sys_for_name("cdte1");
    System gse = deck.get_sys_for_name("gse");

    std::queue<DownlinkBufferElement> downlink_buffer;

    moodycamel::ConcurrentQueue<DownlinkBufferElement> concurrent_downlink_buffer;

    PacketFramer pf(cdte1, RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp(cdte1, gse, RING_BUFFER_TYPE_OPTIONS::PC);
    
    // print out pf and fp to check
    std::cout << pf.to_string();
    std::cout << fp.to_string();

    // build false frame data 
    size_t false_data_length = cdte1.get_frame_size(RING_BUFFER_TYPE_OPTIONS::PC);
    std::vector<uint8_t> false_data;
    false_data.push_back(0x01);
    for (int i = 0; i < false_data_length - 2; ++i) {
        false_data.push_back(0x00);
    }
    false_data.push_back(0x01);

    // make packets out of false data, add spw header to each:
    size_t false_spw_header_size = 24;
    size_t packet_size = cdte1.spacewire->max_payload_size - false_spw_header_size;
    
    std::vector<uint8_t> false_spw_header(false_spw_header_size);
    for (size_t i = 0; i < false_spw_header_size; ++i) {
        false_spw_header[i] = i;
    }

    // assemble vector of packets of false data:
    std::vector<std::vector<uint8_t>> false_input_packets;
    for (size_t i = 0; i < false_data.size(); i += packet_size) {
        size_t last_element = std::min(false_data.size(), i + packet_size);
        std::vector<uint8_t> this_packet;
        std::vector<uint8_t> this_data(false_data.begin() + i, false_data.begin() + last_element);
        this_packet.insert(this_packet.begin(), false_spw_header.begin(), false_spw_header.end());
        this_packet.insert(this_packet.end(), this_data.begin(), this_data.end());
        false_input_packets.push_back(this_packet);

        // std::cout << "\nfalse packet (size " << this_packet.size() << "): ";
        // for (size_t j = 0; j < this_packet.size(); ++j) {
        //     std::cout << " " << std::to_string(this_packet[j]);
        // }
    }
    std::cout << "\n";
    // now, can loop over false_input_packets like they are received from SPMU-001

    // false_data represents one frame.
    for (int i = 0; i < false_input_packets.size(); ++i) {
        // std::cout << "loop " << std::to_string(i) << "/" << std::to_string(false_input_packets.size()) << "\n";
        pf.push_to_frame(false_input_packets[i]);
    }

    std::cout << "\n";
    std::cout << "PacketFramer::check_frame_done(): " << std::to_string(pf.check_frame_done()) << "\n";

    // hand the frame over to FramePacketizer:
    fp.set_frame(pf.get_frame());

    while(!fp.frame_emptied()) {
        DownlinkBufferElement this_db(fp.pop_buffer_element());
        // std::cout << this_db.to_string() << "\n";
        downlink_buffer.push(this_db);
        concurrent_downlink_buffer.enqueue(this_db);
        std::vector<uint8_t> this_pack(this_db.get_packet());
        // hex_print(this_pack);
        // std::cout << "payload.size(): " << std::to_string(this_db.get_payload().size()) << "\n";
        // std::cout << "packet.size(): " << std::to_string(this_db.get_packet().size()) << "\n";
        // std::cout << fp.to_string() << "\n";
    }

    std::cout << "pushed frame to downlink queue. FramePacketizer::frame_emptied(): " << fp.frame_emptied() << "\n";

    // pop buffer to emulate downlink, and remove header "on ground". Reassemble frame.
    std::vector<uint8_t> false_downlink;
    DownlinkBufferElement this_concurrent_element;
    while(downlink_buffer.size() > 0) {

        // try using the std::queue
        DownlinkBufferElement this_downlink = downlink_buffer.front();
        downlink_buffer.pop();

        // try using moodycamel::ConcurrentQueue
        concurrent_downlink_buffer.try_dequeue(this_concurrent_element);
        // std::vector<uint8_t> this_packet = this_downlink.get_packet();
        std::vector<uint8_t> this_packet = this_concurrent_element.get_packet();

        std::vector<uint8_t> this_payload = {this_packet.begin() + 8, this_packet.end()};
        // std::cout << "this_payload.size(): " << std::to_string(this_payload.size()) << "\n";
        // hex_print(this_payload);
        // std::cout << "\n";
        
        false_downlink.insert(false_downlink.end(), this_payload.begin(), this_payload.end());
    }

    // check if source data is reconstructed correctly.
    if (false_downlink == false_data) {
        std::cout << "\nSUCCESS.\n";
    } else {
        std::cout << "\ntry again :(\n";
        std::cout << "\t received " << std::to_string(false_downlink.size()) << " bytes of data: \n";
        for (size_t i = 0; i < false_downlink.size(); ++i) {
            std::cout << " " << std::to_string(false_downlink[i]);
        }
        std::cout << "\n";
    }


    std::cout << "end of main.\n";
    return 0;
}