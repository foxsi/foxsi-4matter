#pragma once
#ifndef FRAGMENTER_H
#define FRAGMENTER_H

#include <vector>
#include <stdint.h>

class Fragmenter {

    public:
        Fragmenter();
        Fragmenter(size_t new_fragment_size, size_t new_header_size, std::vector<uint8_t> new_prefix);
        Fragmenter(size_t new_fragment_size, size_t new_header_size);

        Fragmenter(const Fragmenter& other);
        Fragmenter& operator=(const Fragmenter& other);

        const size_t get_fragment_size() const {return fragment_size;}
        const size_t get_header_size() const {return header_size;}
        const std::vector<uint8_t> get_fragment_prefix() const {return fragment_prefix;}
       
        /**
         * @brief Provided a long byte list, `fragment` it into segments no longer than `Fragmenter::max_packet_size`. Each fragment will receive a prefix: `<packet length><fragment size><original length>` to facilitate reassembly by the receiver.
         * 
         * @return std::vector<std::vector<uint8_t>> 
         */
        std::vector<std::vector<uint8_t>> fragment(std::vector<uint8_t> message_to_fragment);

    private:
        /**
         * @brief stores the maximum allowable size of a fragment in bytes.
         * 
         */
        size_t fragment_size;

        /**
         * @brief stores the maximum allowable header length for a fragment in bytes.
         * 
         * The header has three pieces: `<data length>` `<packet count>` `<counter>` 
         * fields. The `<data length>` field holds the total number of bytes in 
         * this packet. The `<packet count>` field holds the total number of packets
         * needed to transmit this framer. The `<counter>` field indexes each fragment so they can be reassembled on the ground.
         * 
         */
        size_t header_size;

        /**
         * @brief inferred from `header_size`.
         * 
         */
        size_t max_message_size;

        /**
         * @brief constant prefix (i.e. delimiter) for all fragments. Can be empty.
         * 
         */
        std::vector<uint8_t> fragment_prefix;

        /**
         * @brief define `Fragmenter::max_message_size` from `header_size`.
         * 
         */
        void make_max_message_size();

   };

#endif