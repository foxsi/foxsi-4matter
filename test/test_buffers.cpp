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
    System cmos1 = deck.get_sys_for_name("cmos1");
    System gse = deck.get_sys_for_name("gse");

    std::cout << "cdte ring buffers size " << std::to_string(cdte1.ring_params.size()) << "\n";
    std::cout << "cmos ring buffers size " << std::to_string(cmos1.ring_params.size()) << "\n";

    std::queue<DownlinkBufferElement> downlink_buffer_cdte;
    std::queue<DownlinkBufferElement> downlink_buffer_cmos;

    moodycamel::ConcurrentQueue<DownlinkBufferElement> concurrent_downlink_buffer_cdte;
    moodycamel::ConcurrentQueue<DownlinkBufferElement> concurrent_downlink_buffer_cmos;

    PacketFramer pf_cdte(cdte1, RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cdte(cdte1, gse, RING_BUFFER_TYPE_OPTIONS::PC);
    PacketFramer pf_cmos(cmos1, RING_BUFFER_TYPE_OPTIONS::PC);
    FramePacketizer fp_cmos(cmos1, gse, RING_BUFFER_TYPE_OPTIONS::PC);
    
    // print out pf and fp to check
    std::cout << pf_cdte.to_string();
    std::cout << fp_cdte.to_string();
    std::cout << pf_cmos.to_string();
    std::cout << fp_cmos.to_string();

    // build false frame data 
    size_t false_data_length_cdte = cdte1.get_frame_size(RING_BUFFER_TYPE_OPTIONS::PC);
    size_t false_data_length_cmos = cmos1.get_frame_size(RING_BUFFER_TYPE_OPTIONS::PC);
    
    std::vector<uint8_t> false_data_cdte;
    false_data_cdte.push_back(0x01);
    for (int i = 0; i < false_data_length_cdte - 2; ++i) {
        false_data_cdte.push_back(0x00);
    }
    false_data_cdte.push_back(0x01);

    std::vector<uint8_t> false_data_cmos;
    false_data_cmos.push_back(0x01);
    for (int i = 0; i < false_data_length_cmos - 2; ++i) {
        false_data_cmos.push_back(0x00);
    }
    false_data_cmos.push_back(0x01);

    // make packets out of false data, add spw header to each:
    size_t false_spw_header_size = 24;
    size_t false_spw_footer_size = 1;
    size_t packet_size_cdte = cdte1.spacewire->max_payload_size - false_spw_header_size - false_spw_footer_size;    
    size_t packet_size_cmos = cmos1.spacewire->max_payload_size - false_spw_header_size - false_spw_footer_size;    

    std::vector<uint8_t> false_spw_header_cdte(false_spw_header_size);
    for (size_t i = 0; i < false_spw_header_size; ++i) {
        false_spw_header_cdte[i] = i;
    }
    std::vector<uint8_t> false_spw_footer_cdte(false_spw_footer_size);
    for (size_t i = 0; i < false_spw_footer_size; ++i) {
        false_spw_footer_cdte[i] = 0xff;
    }

    std::vector<uint8_t> false_spw_header_cmos(false_spw_header_size);
    for (size_t i = 0; i < false_spw_header_size; ++i) {
        false_spw_header_cmos[i] = i;
    }
    std::vector<uint8_t> false_spw_footer_cmos(false_spw_footer_size);
    for (size_t i = 0; i < false_spw_footer_size; ++i) {
        false_spw_footer_cmos[i] = 0xff;
    }

    // assemble vector of packets of false data:
    std::vector<std::vector<uint8_t>> false_input_packets_cdte;
    for (size_t i = 0; i < false_data_cdte.size(); i += packet_size_cdte) {
        size_t last_element = std::min(false_data_cdte.size(), i + packet_size_cdte);
        std::vector<uint8_t> this_packet;
        std::vector<uint8_t> this_data(false_data_cdte.begin() + i, false_data_cdte.begin() + last_element);
        this_packet.insert(this_packet.begin(), false_spw_header_cdte.begin(), false_spw_header_cdte.end());
        this_packet.insert(this_packet.end(), this_data.begin(), this_data.end());
        this_packet.insert(this_packet.end(), false_spw_footer_cdte.begin(), false_spw_footer_cdte.end());
        
        false_input_packets_cdte.push_back(this_packet);
    }
    std::cout << "\n";

    // assemble vector of packets of false data:
    std::vector<std::vector<uint8_t>> false_input_packets_cmos;
    for (size_t i = 0; i < false_data_cmos.size(); i += packet_size_cmos) {
        size_t last_element = std::min(false_data_cmos.size(), i + packet_size_cmos);
        std::vector<uint8_t> this_packet;
        std::vector<uint8_t> this_data(false_data_cmos.begin() + i, false_data_cmos.begin() + last_element);
        this_packet.insert(this_packet.begin(), false_spw_header_cmos.begin(), false_spw_header_cmos.end());
        this_packet.insert(this_packet.end(), this_data.begin(), this_data.end());
        this_packet.insert(this_packet.end(), false_spw_footer_cmos.begin(), false_spw_footer_cmos.end());
        
        false_input_packets_cmos.push_back(this_packet);
    }
    std::cout << "\n";
    // now, can loop over false_input_packets like they are received from SPMU-001

    // false_data represents one frame.
    for (int i = 0; i < false_input_packets_cdte.size(); ++i) {
        pf_cdte.push_to_frame(false_input_packets_cdte[i]);
    }
    for (int i = 0; i < false_input_packets_cmos.size(); ++i) {
        pf_cmos.push_to_frame(false_input_packets_cmos[i]);
    }

    std::cout << "\n";
    std::cout << "PacketFramer::check_frame_done() (for cdte): " << std::to_string(pf_cdte.check_frame_done()) << "\n";
    if (!pf_cdte.check_frame_done()) {
        std::cout << "PacketFramer::frame.size(): " << std::to_string(pf_cdte.get_frame().size()) << "\n";
    }
    std::cout << "PacketFramer::check_frame_done() (for cmos): " << std::to_string(pf_cmos.check_frame_done()) << "\n";
    if (!pf_cmos.check_frame_done()) {
        std::cout << "PacketFramer::frame.size(): " << std::to_string(pf_cmos.get_frame().size()) << "\n";
    }

    // hand the frame over to FramePacketizer:
    fp_cdte.set_frame(pf_cdte.get_frame());
    fp_cmos.set_frame(pf_cmos.get_frame());

    while(!fp_cdte.frame_emptied()) {
        DownlinkBufferElement this_db(fp_cdte.pop_buffer_element());
        // std::cout << this_db.to_string() << "\n";
        downlink_buffer_cdte.push(this_db);
        concurrent_downlink_buffer_cdte.enqueue(this_db);
        std::vector<uint8_t> this_pack(this_db.get_packet());
        // hex_print(this_pack);
        // std::cout << "payload.size(): " << std::to_string(this_db.get_payload().size()) << "\n";
        // std::cout << "packet.size(): " << std::to_string(this_db.get_packet().size()) << "\n";
        // std::cout << fp.to_string() << "\n";
    }
    while(!fp_cmos.frame_emptied()) {
        DownlinkBufferElement this_db(fp_cmos.pop_buffer_element());
        // std::cout << this_db.to_string() << "\n";
        downlink_buffer_cmos.push(this_db);
        concurrent_downlink_buffer_cmos.enqueue(this_db);
        std::vector<uint8_t> this_pack(this_db.get_packet());
        // hex_print(this_pack);
        // std::cout << "payload.size(): " << std::to_string(this_db.get_payload().size()) << "\n";
        // std::cout << "packet.size(): " << std::to_string(this_db.get_packet().size()) << "\n";
        // std::cout << fp.to_string() << "\n";
    }

    std::cout << "pushed cdte frame to downlink queue. FramePacketizer::frame_emptied(): " << fp_cdte.frame_emptied() << "\n";
    std::cout << "pushed cmos frame to downlink queue. FramePacketizer::frame_emptied(): " << fp_cmos.frame_emptied() << "\n";

    std::cout << fp_cdte.to_string();
    std::cout << fp_cmos.to_string();

    // pop buffer to emulate downlink, and remove header "on ground". Reassemble frame.
    std::vector<uint8_t> false_downlink_cdte;
    DownlinkBufferElement this_concurrent_element_cdte(&cdte1, &gse, RING_BUFFER_TYPE_OPTIONS::PC);
    // this_concurrent_element_cdte.set_type(RING_BUFFER_TYPE_OPTIONS::PC);
    while(downlink_buffer_cdte.size() > 0) {

        // try using the std::queue
        DownlinkBufferElement this_downlink(downlink_buffer_cdte.front());
        downlink_buffer_cdte.pop();

        // try using moodycamel::ConcurrentQueue
        concurrent_downlink_buffer_cdte.try_dequeue(this_concurrent_element_cdte);
        // std::vector<uint8_t> this_packet = this_downlink.get_packet();
        std::vector<uint8_t> this_packet = this_concurrent_element_cdte.get_packet();

        std::vector<uint8_t> this_payload = {this_packet.begin() + 8, this_packet.end()};
        // std::cout << "this_payload.size(): " << std::to_string(this_payload.size()) << "\n";
        // hex_print(this_payload);
        // std::cout << "\n";
        
        false_downlink_cdte.insert(false_downlink_cdte.end(), this_payload.begin(), this_payload.end());
    }
    std::vector<uint8_t> false_downlink_cmos;
    DownlinkBufferElement this_concurrent_element_cmos(&cmos1, &gse, RING_BUFFER_TYPE_OPTIONS::PC);
    while(downlink_buffer_cmos.size() > 0) {

        // try using the std::queue
        DownlinkBufferElement this_downlink(downlink_buffer_cmos.front());
        downlink_buffer_cmos.pop();

        // try using moodycamel::ConcurrentQueue
        concurrent_downlink_buffer_cmos.try_dequeue(this_concurrent_element_cmos);
        // std::vector<uint8_t> this_packet = this_downlink.get_packet();
        std::vector<uint8_t> this_packet = this_concurrent_element_cmos.get_packet();

        std::vector<uint8_t> this_payload = {this_packet.begin() + 8, this_packet.end()};
        // std::cout << "this_payload.size(): " << std::to_string(this_payload.size()) << "\n";
        // hex_print(this_payload);
        // std::cout << "\n";
        
        false_downlink_cmos.insert(false_downlink_cmos.end(), this_payload.begin(), this_payload.end());
    }
    std::cout << "cmos FramePacketizer: " << fp_cmos.to_string() << "\n";

    // check if source data is reconstructed correctly.
    if (false_downlink_cdte == false_data_cdte) {
        std::cout << "\nSUCCESS for cdte transfer.\n";
    } else {
        std::cout << "\ntry again cdte transfer :(\n";
        std::cout << "\treceived " << std::to_string(false_downlink_cdte.size()) << " bytes of data: \n";
        for (size_t i = 0; i < false_downlink_cdte.size(); ++i) {
            std::cout << " " << std::to_string(false_downlink_cdte[i]);
        }
        std::cout << "\n";
    }
    if (false_downlink_cmos == false_data_cmos) {
        std::cout << "\nSUCCESS for cmos transfer.\n";
    } else {
        std::cout << "\ntry again cmos transfer :(\n";
        std::cout << "\treceived " << std::to_string(false_downlink_cmos.size()) << " bytes of data: \n";
        for (size_t i = 0; i < false_downlink_cmos.size(); ++i) {
            std::cout << " " << std::to_string(false_downlink_cmos[i]);
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    // ---------------------------- SpaceWire reply data tests ----------------------- //

    std::cout << "test SpaceWire reply data:\n";
    std::vector<uint8_t> good_reply = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0xfe, 0x01, 0x0d, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x0e, 0x04, 0x22, 0x80, 0x3c, 0x42};
    std::vector<uint8_t> bad_reply = {0x21, 0x12, 0x81, 0x6a, 0x32, 0x3a, 0x0b, 0x70, 0x4b, 0x33, 0x8b, 0x33, 0xab, 0xb3, 0x22, 0xbf, 0x2b, 0x33, 0x32, 0xba, 0xab, 0x23, 0x3a, 0x2a, 0x12, 0xb7, 0x9f, 0x72, 0x91};
    std::cout << "from good packet: \n";
    std::vector<uint8_t> good_data = cdte1.spacewire->get_reply_data(good_reply);
    std::vector<uint8_t> ref_good_data = {0x04, 0x22, 0x80, 0x3c};

    utilities::hex_print(good_data);
    std::cout << "from bad packet: \n";
    std::vector<uint8_t> bad_data = cdte1.spacewire->get_reply_data(bad_reply);
    utilities::hex_print(bad_data);
    std::vector<uint8_t> ref_bad_data = {};

    if (good_data == ref_good_data) {
        std::cout << "\nSUCCESS for good packet parsing.\n";
    } else {
        std::cout << "\ntry again parsing the good packet :(\n";
        std::cout << "\tgot ";
        utilities::hex_print(good_data);
    }

    if (bad_data == ref_bad_data) {
        std::cout << "\nSUCCESS for bad packet parsing.\n";
    } else {
        std::cout << "\ntry again parsing the bad packet :(\n";
        std::cout << "\tgot ";
        utilities::hex_print(bad_data);
    }
    std::cout << "\n";

    std::vector<uint8_t> test = {0,1,2,3,4,5,6,7};
    
    std::vector<uint8_t> other(test.begin() + 8, test.begin() + 8);
    std::cout << "other: ";
    utilities::hex_print(other);

    // ---------------------------- SpaceWire print tests ---------------------------- //

    std::cout << "test SpaceWire printing:\n";
    std::vector<uint8_t> test_cdte_write_msg = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x01, 0x02, 0xfe, 0x01, 0x7d, 0x02, 0x00, 0x00, 0x01, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xb5, 0x3c, 0x3c, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x3c, 0x3c, 0x3c, 0x3c, 0x3e};
    std::vector<uint8_t> test_cdte_read_msg = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x01, 0x02, 0xfe, 0x01, 0x4d, 0x02, 0x00, 0x00, 0x01, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x53, 0xeb, 0xd8, 0x00, 0x07, 0xd0, 0x94};
    std::vector<uint8_t> test_cdte_reply_msg = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0xfe, 0x01, 0x0d, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x0e, 0x00, 0x41, 0x00, 0x18, 0xa9};

    auto test_cmos_write_msg = deck.make_spw_packet_for_sys_for_command(cmos1, deck.get_command_for_sys_for_code(cmos1.hex, 0x10));
    auto test_cmos_read_msg = deck.make_spw_packet_for_sys_for_command(cmos1, deck.get_command_for_sys_for_code(cmos1.hex, 0x92));

    utilities::spw_print(test_cmos_write_msg, cmos1.spacewire);
    utilities::spw_print(test_cmos_read_msg, cmos1.spacewire);
    utilities::spw_print(test_cdte_write_msg, cdte1.spacewire);
    utilities::spw_print(test_cdte_read_msg, cdte1.spacewire);
    utilities::spw_print(test_cdte_reply_msg, nullptr);

    std::cout << "end of main.\n";
    return 0;
}