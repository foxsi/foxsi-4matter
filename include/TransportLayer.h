/**
 * @file TransportLayer.h
 * @author Thanasi Pantazides
 * @brief Manager for onboard communication.
 * @version v1.0.1
 * @date 2024-03-08
 * 
 */
#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include "RingBufferInterface.h"
#include "Fragmenter.h"
#include "Commanding.h"
#include "Systems.h"
#include "Buffers.h"
#include "Parameters.h"
#include <boost/asio.hpp>
#include <vector>
#include <map>
#include <queue>
#include "moodycamel/concurrentqueue.h"
#include "UartInterface.h"

/**
 * @brief Manager for communication among systems connected to the Formatter.
 * 
 * This class manages transport-layer services (UDP, TCP, and UART) for the Formatter software. The class wraps some basic socket input-output functionality (provided by `boost::asio`) and manages internal buffering and forwarding of received data. Currently, assumes network topology in which one remote TCP endpoint sends messages which are filtered and forwarded to another remote UDP endpoint. 
 * @note There are a mix of assumptions made about the network topology in this class, all of which are true for the FOXSI-4 design, but which may make this object difficult to adapt for different system configurations. See the main project README for a diagram of the network structure used here.
 */
class TransportLayerMachine {
    public:
        /**
         * @brief the local machine's UDP socket.
         */
        boost::asio::ip::udp::socket local_udp_sock;
        /**
         * @brief the local machine's general TCP socket.
         */
        boost::asio::ip::tcp::socket local_tcp_sock;
        /**
         * @brief the local machine's TCP socket for the housekeeping system.
         */
        boost::asio::ip::tcp::socket local_tcp_housekeeping_sock;
        /**
         * @brief a remote machine's UDP endpoint.
         */
        boost::asio::ip::udp::endpoint remote_udp_endpoint;
        /**
         * @brief a remote machine's TCP endpoint.
         */
        boost::asio::ip::tcp::endpoint remote_tcp_endpoint;
        /**
         * @brief the remote housekeeping machine's TCP endpoint.
         */
        boost::asio::ip::tcp::endpoint remote_tcp_housekeeping_endpoint;
        /**
         * @brief the local machine's general UART port.
         */
        boost::asio::serial_port local_uart_port;
        /**
         * @brief the local machine's UART port which is reserved for uplink use.
         */
        boost::asio::serial_port uplink_uart_port;
        /**
         * @deprecated replaced by `System`-specific buffers.
         * @brief a rudimentary buffer for data to downlink (send to UDP endpoint).
         */
        std::vector<uint8_t> downlink_buff;
        /**
         * @deprecated replaced by `System`-specific buffers.
         * @brief a rudimentary buffer for uplinked command data (to send to TCP endpoint).
         */
        std::vector<uint8_t> uplink_buff;
        /**
         * @brief a map associating each system with a buffer for its uplink commands, for convenient access.
         */
        std::shared_ptr<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>> uplink_buffer;
        /**
         * @brief a queue of data to be downlinked.
         * 
         * This is implemented using `moodycamel::ConcurrentQueue` which provides thread-safe capability for multiple writers to push data onto the same queue simultaneously. 
         */
        std::shared_ptr<moodycamel::ConcurrentQueue<DownlinkBufferElement>> downlink_buffer;

        /**
         * @deprecated replaced by `System`-specific buffers.         
         * @brief a rudimentary buffer for uplinked command data (to send to TCP endpoint).
         */
        std::vector<uint8_t> command_pipe;
        /**
         * @deprecated unused.
         */
        std::queue<uint8_t> ground_pipe;

        /**
         * @brief instance of `CommandDeck`, storing command and system data used to decode and forward uplinked commands.
         */
        std::shared_ptr<CommandDeck> commands;
        
        /**
         * @deprecated replaced by `PacketFramer`.
         * @brief map from `System::hex` codes for each onboard system to `RingBufferInterface` objects for each system.
         */
        std::unordered_map<uint8_t, RingBufferInterface> ring_buffers;

        /**
         * @deprecated replaced by `FramePacketizer`.
         * @brief instance of `Fragmenter` used to slice downlink data stream into appropriated-sized blocks.
         */
        Fragmenter fragmenter;

        /**
         * @deprecated currently unused. And probably unsafe.
         */
        STATE_ORDER active_state;
        /**
         * @deprecated currently unused. And probably unsafe.
         */
        SUBSYSTEM_ORDER active_subsys;

    public:
        /**
         * @deprecated due to non-initialized UARTs.
         * @brief Default constructor. 
         * Creates an empty `CommandDeck` and assigns `TransportLayerMachine::*socket` and `TransportLayerMachine::*endpoint` to the default values in `Parameters.h`.
         * While constructing, this will try to open sockets and connect to remote interfaces.
         * @param new_uplink_buffer a map of buffers used to store commands sent to each `System`.
         * @param new_downlink_buffer a shared queue for all downlink data.
         * @param context A reference to a `boost::asio::io_context` used to run asynchronous work.
         */
        TransportLayerMachine(
            std::shared_ptr<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>> new_uplink_buffer, 
            std::shared_ptr<moodycamel::ConcurrentQueue<DownlinkBufferElement>> new_downlink_buffer,
            boost::asio::io_context& context
        );

        /**
         * @deprecated due to non-initialized UARTs.
         * @brief Construct a new Transport Layer Machine object from predefined `boost::asio` endpoint objects.
         * While constructing, this will try to open sockets and connect to remote interfaces.
         * @param local_udp_end the local machine's UDP endpoint.
         * @param local_tcp_end the local machine's TCP endpoint for detector communication.
         * @param local_tcp_housekeeping_end the local machine's TCP endpoint for housekeeping communication.
         * @param remote_udp_end a remote machine's UDP endpoint.
         * @param remote_tcp_end a remote machine's TCP endpoint.
         * @param remote_tcp_housekeeping_end a remote machine's TCP endpoint for housekeeping communication.
         * @param new_uplink_buffer a map of buffers used to store commands sent to each `System`.
         * @param new_downlink_buffer a shared queue for all downlink data.
         * @param context a reference to a `boost::asio::io_context` used for running asynchronous work.
         */
        TransportLayerMachine(
            boost::asio::ip::udp::endpoint local_udp_end,
            boost::asio::ip::tcp::endpoint local_tcp_end,
            boost::asio::ip::tcp::endpoint local_tcp_housekeeping_end,
            boost::asio::ip::udp::endpoint remote_udp_end,
            boost::asio::ip::tcp::endpoint remote_tcp_end,
            boost::asio::ip::tcp::endpoint remote_tcp_housekeeping_end,
            std::shared_ptr<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>> new_uplink_buffer, 
            std::shared_ptr<moodycamel::ConcurrentQueue<DownlinkBufferElement>> new_downlink_buffer,
            boost::asio::io_context& context
        );

        /**
         * @brief Construct a new Transport Layer Machine object from predefined `boost::asio` endpoint objects and `UART` interfaces.
         * While constructing, this will try to open sockets, ports, and connect to remote interfaces.
         * @param local_udp_end the local machine's UDP endpoint.
         * @param local_tcp_end the local machine's TCP endpoint for detector communication.
         * @param local_tcp_housekeeping_end the local machine's TCP endpoint for housekeeping communication.
         * @param remote_udp_end a remote machine's UDP endpoint.
         * @param remote_tcp_end a remote machine's TCP endpoint.
         * @param remote_tcp_housekeeping_end a remote machine's TCP endpoint for housekeeping communication.
         * @param new_uplink_buffer a map of buffers used to store commands sent to each `System`.
         * @param new_downlink_buffer a shared queue for all downlink data.
         * @param local_uart a `std::shared_ptr` to the `UART` object storing configuration data for the onboard detector UART.
         * @param uplink_uart a `std::shared_ptr` to the `UART` object storing configuration data for the uplink data UART.
         * @param context a reference to a `boost::asio::io_context` used for running asynchronous work.
         */
        TransportLayerMachine(
            boost::asio::ip::udp::endpoint local_udp_end,
            boost::asio::ip::tcp::endpoint local_tcp_end,
            boost::asio::ip::tcp::endpoint local_tcp_housekeeping_end,
            boost::asio::ip::udp::endpoint remote_udp_end,
            boost::asio::ip::tcp::endpoint remote_tcp_end,
            boost::asio::ip::tcp::endpoint remote_tcp_housekeeping_end,
            std::shared_ptr<std::unordered_map<System, moodycamel::ConcurrentQueue<UplinkBufferElement>>> new_uplink_buffer, 
            std::shared_ptr<moodycamel::ConcurrentQueue<DownlinkBufferElement>> new_downlink_buffer,
            std::shared_ptr<UART> local_uart,
            std::shared_ptr<UART> uplink_uart,
            boost::asio::io_context& context
        );

        /**
         * @deprecated Superseded by functionality in `Circle`.
         * @param new_subsys Next entry in `SUBSYSTEM_ORDER` enum to advance to. Consider replacing with for `Subsystem::hex`.
         * @param new_state Next entry in `STATE_ORDER` enum to advance to.
         */
        void update(SUBSYSTEM_ORDER new_subsys, STATE_ORDER new_state);

        /**
         * @brief replaces `TransportLayerMachine::commands` with the provided `CommandDeck`.
         * @param new_commands new `CommandDeck` to use when parsing uplinked command messages.
         */
        void add_commands(std::shared_ptr<CommandDeck> new_commands);

        /**
         * @deprecated `RingBufferInterface` superseded by `PacketFramer`.
         * @brief replaces `TransportLayerMachine::ring_buffers` with a new interface map.
         * @param new_ring_buffers new mapping from applicable `System::hex` values to system-specific `RingBufferInterface` objects.
         */
        void add_ring_buffer_interface(std::unordered_map<uint8_t, RingBufferInterface> new_ring_buffers);
        /**
         * @deprecated `Fragmenter` superseded by `FramePacketizer`.
         * @brief replaces `TransportLayerMachine::fragmenter` with a new one.
         * @param new_fragmenter new `Fragmenter` object to use to decimate downlink data stream.
         */
        void add_fragmenter(Fragmenter new_fragmenter);
        /**
         * @deprecated `Fragmenter` superseded by `FramePacketizer`.
         * @brief replaces `TransportLayerMachine::fragmenter` with new one constructed in-place from provided values.
         * @param fragment_size See Fragmenter constructor.
         * @param header_size See Fragmenter constructor.
         */
        void add_fragmenter(size_t fragment_size, size_t header_size);

        /**
         * @deprecated unused.
         */
        void handle_recv();
        /**
         * @deprecated unused.
         * @brief Asynchronously forwards any received TCP packets over UDP.
         */
        void recv_tcp_fwd_udp();
        /**
         * @deprecated unused.
         * @brief Asynchronously forwards any received UDP packets over TCP.
         */
        void recv_udp_fwd_tcp();
        /**
         * @deprecated unused.
         * @brief Asynchronously sends data stored in `TransportLayerMachine::uplink_buffer` to  `TransportLayerMachine::remote_tcp_endpoint`.
         */
        void send_tcp();

        /**
         * @deprecated unused.
         * @brief Sending UART message
         */
        void send_uart();
        /**
         * @deprecated unused.
         * @brief Receive UART message
         */
        void recv_uart();

        /**
         * @brief Convenience method to receive and print UDP packets.
         */
        void print_udp_basic();

        /**
         * @brief Receive `buffer.size()` amount of data from `socket`, populating `buffer`, with timeout and retry attempts taken from `sys_man`.
         * 
         * Uses underlying `boost::asio::read()` method, so will return once `buffer.size()` bytes have been received from socket, or the operation times out. Use `TransportLayerMachine::read_some()` if you want to get any available data in the socket.
         * 
         * The read timeout and number of re-read attempts should be specified in the `SystemManager::timing*` object. If data has not been received after the final timeout, `buffer` will be resized to zero.
         * 
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * 
         * @param socket the socket object to read from.
         * @param buffer the buffer to fill with data. Will be resized to the data size received.
         * @param sys_man the `SystemManager` object describing the remote end of the socket. Used to derive timeout/retry information.
         * @return size_t the number of bytes read.
         */
        size_t read(boost::asio::ip::tcp::socket& socket, std::vector<uint8_t>& buffer, SystemManager& sys_man);

        /**
         * @brief Receive any data from `socket` into `buffer`, with timeout and retry attempts taken from `sys_man`.
         * 
         * Uses underlying `boost::asio::ip::tcp::socket::read_some()` method, so will return any available data in the socket, or the operation times out. Use `TransportLayerMachine::read()` if you want to read a specific data size from the socket.
         *
         * The read timeout and number of re-read attempts should be specified in the `SystemManager::timing*` object. If data has not been received after the final timeout, `buffer` will be resized to zero.
         * 
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * 
         * @param socket the socket object to read from.
         * @param buffer the buffer to fill with data. Will be resized.
         * @param sys_man the `SystemManager` object describing the remote end of the socket. Used to derive timeout/retry information.
         * @return size_t the number of bytes read.
         */
        size_t read_some(boost::asio::ip::tcp::socket& socket, std::vector<uint8_t>& buffer, SystemManager& sys_man);

        /**
         * @brief Receive `buffer.size()` amount of data from `socket`, populating `buffer`, with timeout and retry attempts taken from `sys_man`.
         * 
         * Uses underlying `boost::asio::read()` method, so will return once `buffer.size()` bytes have been received from socket, or the operation times out. Use `TransportLayerMachine::read_some()` if you want to get any available data in the socket.
         * 
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * 
         * The read timeout and number of re-read attempts should be specified in the `SystemManager::timing*` object. If data has not been received after the final timeout, `buffer` will be resized to zero.
         * 
         * @param socket the socket object to read from.
         * @param buffer the buffer to fill with data. Will be resized.
         * @param sys_man the `SystemManager` object describing the remote end of the socket. Used to derive timeout/retry information.
         * @return size_t the number of bytes read.
         */
        size_t read_udp(boost::asio::ip::udp::socket& socket, std::vector<uint8_t>& buffer, SystemManager& sys_man);

        /**
         * @brief Receive `buffer.size()` amount of data from `port`, populating `buffer`, with timeout and retry attempts taken from `sys_man`.
         * 
         * Uses underlying `boost::asio::read()` method, so will return once `buffer.size()` bytes have been received from socket, or the operation times out. Use `TransportLayerMachine::read_some()` if you want to get any available data in the socket.
         * 
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * 
         * The read timeout and number of re-read attempts should be specified in the `SystemManager::timing*` object. If data has not been received after the final timeout, `buffer` will be resized to zero.
         * 
         * @param socket the socket object to read from.
         * @param buffer the buffer to fill with data. Will be resized.
         * @param sys_man the `SystemManager` object describing the remote end of the socket. Used to derive timeout/retry information.
         * @return size_t the number of bytes read.
         */
        size_t read(boost::asio::serial_port& port, std::vector<uint8_t>& buffer, SystemManager& sys_man);
        
        /**
         * @brief Receive (blocking) on `local_tcp_sock` until `timeout_ms` expires.
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * @param receive_size the amount of data (bytes) expected.
         * @param timeout_ms the timeout for reading, in milliseconds.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> sync_tcp_read(size_t receive_size, std::chrono::milliseconds timeout_ms);
        /**
         * @brief Receive data `receive_size` amount of data from `socket`, timing out after specified `timeout_ms`.
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * @param socket the socket to read from.
         * @param receive_size the amount of data (bytes) expected.
         * @param timeout_ms the timeout for reading, in milliseconds.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> sync_tcp_read(boost::asio::ip::tcp::socket& socket, size_t receive_size, std::chrono::milliseconds timeout_ms);
        /**
         * @brief Receive any amount of data from `socket`, timing out after specified `timeout_ms`.
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * @param socket the socket to read from.
         * @param timeout_ms the timeout for reading, in milliseconds.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> sync_tcp_read_some(boost::asio::ip::tcp::socket& socket, std::chrono::milliseconds timeout_ms);

        /**
         * @brief Receive data `receive_size` amount of data from `socket`, timing out after specified `timeout_ms`.
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * @param socket the socket to read from.
         * @param receive_size the amount of data (bytes) expected.
         * @param timeout_ms the timeout for reading, in milliseconds.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> sync_udp_read(boost::asio::ip::udp::socket& socket, size_t receive_size, std::chrono::milliseconds timeout_ms);

        /**
         * @brief Receive data `receive_size` amount of data from `port`, timing out after specified `timeout_ms`.
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * @param port the port to read from.
         * @param receive_size the amount of data (bytes) expected.
         * @param timeout_ms the timeout for reading, in milliseconds.
         * @return std::vector<uint8_t> 
         */
        std::vector<uint8_t> sync_uart_read(boost::asio::serial_port& port, size_t receive_size, std::chrono::milliseconds timeout_ms);

        /**
         * @brief Low-level handler for `TransportLayerMachine::sync_tcp_read(...)`-style methods. Swaps received data into `out_` variables.
         * 
         * @param ec error code raised during read operation.
         * @param length length of data read from socket.
         * @param out_ec error code raised during read operation.
         * @param out_length length of data read from socket.
         */
        static void sync_tcp_read_handler(const boost::system::error_code& ec, std::size_t length, boost::system::error_code* out_ec, std::size_t* out_length);
        /**
         * @brief Low-level handler for `TransportLayerMachine::sync_uart_read(...)`-style methods. Swaps received data into `out_` variables.
         * @param ec error code raised during read operation.
         * @param length length of data read from port.
         * @param out_ec error code raised during read operation.
         * @param out_length length of data read from port.
         */
        static void sync_uart_read_handler(const boost::system::error_code& ec, std::size_t length, boost::system::error_code* out_ec, std::size_t* out_length);

        /**
         * @brief Run the underlying `io_context` for `timeout_ms` milliseconds. 
         * If the operation is not completed by the timeout, the socket `::local_udp_sock` will be canceled, and the `io_context` will restart..
         * @param timeout_ms the deadline for the `io_context` to run, in milliseconds.
         * @return bool with value `true` if the deadline passed.
         */
        bool run_udp_context(std::chrono::milliseconds timeout_ms);
        /**
         * @brief Run the underlying `io_context` for `timeout_ms` milliseconds. 
         * If the operation is not completed by the timeout, the socket `::local_tcp_sock` will be canceled, and the `io_context` will restart..
         * @param timeout_ms the deadline for the `io_context` to run, in milliseconds.
         * @return bool with value `true` if the deadline passed.
         */
        bool run_tcp_context(boost::asio::ip::tcp::socket& socket, std::chrono::milliseconds timeout_ms);
        /**
         * @brief Run the underlying `io_context` for `timeout_ms` milliseconds. 
         * If the operation is not completed by the timeout, the ports `::local_uart_port` and `::local_uplink_port` will be canceled, and the `io_context` will restart..
         * @param timeout_ms the deadline for the `io_context` to run, in milliseconds.
         * @return bool with value `true` if the deadline passed.
         */
        bool run_uart_context(std::chrono::milliseconds timeout_ms);

        /**
         * @deprecated Unimplemented, unused, unwanted.
         */
        void run_tcp_context(SystemManager& sys_man);

        /**
         * @brief Execute remote memory transaction over SpaceWire RMAP for `buffer_type` memory region in the remote endpoint defined in `sys_man`.
         * 
         * Synchronously reads ring buffer data frame from `sys_man`, using timeout/retry parameters. Supply a `prior_write_pointer` to check the last position in remote memory that was read. If the write pointer has not advanced, no read will be performed. On a successful read of a complete frame from a remote `System`, this method will packetize the frame and insert each packet into the `::downlink_buffer`.
         * 
         * This is a relatively complicated method, with multiple possible early exit points: 
         *  1. For early returns (likely due to failed read), this will return the same `prior_write_pointer` argument to be reused in the next call.
         *  2. For nominal returns, will return a new value for `prior_write_pointer` which was successfully read. Will also populate data to downlink in `::downlink_buffer`.
         * 
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * 
         * @param sys_man description of the remote system being queried.
         * @param buffer_type the type of ring buffer data to read.
         * @param prior_write_pointer the region of remote memory last written by the remote system.
         * @return size_t the address in remote memory that was read, to be used as the next `prior_write_pointer`.
         */
        size_t sync_remote_buffer_transaction(SystemManager& sys_man, RING_BUFFER_TYPE_OPTIONS buffer_type, size_t prior_write_pointer);

        /**
         * @brief Search the uplink buffer map for commands for `sys_man`, and send them.
         * 
         * Delegates command transmission to `sys_man` to `TransportLayerMachine::sync_send_commmand_to_system()`. If the uplink command gets a reply, the reply will be added to the downlink buffer with data type tag `RING_BUFFER_TYPE_OPTIONS::REPLY`.
         * 
         * @param sys_man the remote system being commanded.
        */
        void sync_send_buffer_commands_to_system(SystemManager& sys_man);

        /**
         * @brief Send the command `cmd` to the system `sys_man`.
         * 
         * This function has relatively complex behavior. 
         *  - If the `sys_man` takes commands over SpaceWire, and `cmd` is a SpaceWire write command with the `reply` bit set, this method will re-try the write command up to 8 times if there is no reply received.
         *  - For `sys_man`s that use different command media, they will try write commands only once (due to lack of native feedback for successful send operations in Ethernet and UART).
         *  - For any `read` operations (whether in response to confirm a SpaceWire `write`, or as the baseline `cmd` that is sent), timeout/retry read methods are used, with timeouts and retry count defined by `sys_man`.
         * 
         * @warning This method starts/stops the underlying `boost::io_context` and cancels asynchronous socket operations in order to handle the socket timeout. Only call in a context that can handle these `io_context` work interruptions.
         * 
         * @param sys_man the system to send the command to.
         * @param cmd the command to send.
         * @returns std::vector<uint8_t> any reply transmitted in response to the command.
        */
        std::vector<uint8_t> sync_send_command_to_system(SystemManager& sys_man, Command cmd);

        /**
         * @deprecated Superseded by `sync_send_command_to_system`.
         */
        std::vector<uint8_t> sync_tcp_housekeeping_transaction(std::vector<uint8_t> data_to_send);
        /**
         * @deprecated Superseded by `sync_send_command_to_system`.
         */
        void sync_tcp_housekeeping_send(std::vector<uint8_t> data_to_send);

        /**
         * @brief Reads up to 8 uplink commands (each of which is 2 bytes long) from `::local_udp_sock`, storing them in the appropriate uplink buffer.
         * @warning If ground support transmits both RS-232 and Ethernet (UDP) uplink, using this function and `::sync_uart_receive_to_uplink_buffer()` will result in duplicate uplink commands in the uplink buffer.
         * @param uplink_sys_man a reference to the `SystemManager` object that defines the uplink interface. 
         */
        void sync_udp_receive_to_uplink_buffer(SystemManager& uplink_sys_man);
        /**
         * @brief Reads up to 8 uplink commands (each of which is 2 bytes long) from `::local_uart_sock`, storing them in the appropriate uplink buffer.
         * Should be used for UART-based uplink.
         * @warning If ground support transmits both RS-232 and Ethernet (UDP) uplink, using this function and `::sync_udp_receive_to_uplink_buffer()` will result in duplicate uplink commands in the uplink buffer.
         * @param uplink_sys_man a reference to the `SystemManager` object that defines the uplink interface. 
         */
        void sync_uart_receive_to_uplink_buffer(SystemManager& uplink_sys_man);
        
        /**
         * @deprecated Due to lack of testing. Use `sync_` version instead.
         */
        void async_udp_receive_to_uplink_buffer();
        /**
         * @deprecated Due to lack of testing. Use `sync_` version instead.
         */
        void async_udp_send_downlink_buffer();


        /**
         * @brief Send all the `DownlinkBufferElement`s currently stored in `::downlink_buffer` out of `local_udp_sock`.
         * @return bool true if there is data left in the downlink buffer (should be never).
         */
        bool sync_udp_send_all_downlink_buffer();

        /**
         * @deprecated Superseded by `sync_send_command_to_system`.
         * @brief Synchronously send command `cmd` to remote `sys_man`.
         * 
         * @param sys_man
         * @param cmd 
         * @return std::vector<uint8_t> any response data to the command.
         */
        std::vector<uint8_t> sync_tcp_send_command_for_sys(SystemManager sys_man, Command cmd);
        
        /**
         * @deprecated Superseded by `sync_send_command_to_system`.
         * @brief Synchronously send command `cmd` to remote housekeeping system `hk_man` over `local_tcp_housekeeping_socket`.
         * 
         * @param sys_man
         * @param cmd 
         * @return std::vector<uint8_t> any response data to the command.
         */
        std::vector<uint8_t> sync_tcp_send_command_for_housekeeping_sys(SystemManager hk_man, Command cmd);

        /**
         * @deprecated Superseded by `sync_send_command_to_system`.
         * @brief Synchronously send command `cmd` to remote `sys_man`.
         * 
         * @param sys_man
         * @param cmd 
         * @return std::vector<uint8_t> any response data to the command.
         */
        std::vector<uint8_t> sync_uart_send_command_for_sys(SystemManager sys_man, Command cmd);

        /**
         * @deprecated Due to lack of testing.
         */
        void async_udp_receive_push_to_uplink_buffer(const boost::system::error_code& err, std::size_t byte_count);

        /**
         * @brief Block until `::local_udp_sock` receives the byte string `0x01` `0x0f`.
         * Used to suspend loop start during ground testing.
         */
        void await_loop_begin();

        /**
         * @brief Extract the data field from a SpaceWire reply sent by `sys`.
         * @todo specialize the method name to SpaceWire e.g. `get_spw_reply_data` or something.
         * @param spw_reply the full, raw reply data (presumed via an SPMU-001).
         * @param sys the `System` sending the reply.
         * @return std::vector<uint8_t> the data field from the SpaceWire packet.
         */
        std::vector<uint8_t> get_reply_data(std::vector<uint8_t> spw_reply, System& sys);

        /**
         * @brief checks if a provided command (lookup in `TransportLayerMachine::commands`) will try to query a remote ring buffer.
         * @todo the frame read commands are just hard-coded defaults currently. Either add an identifying field to `foxsi4-commands` or define these constants in `Parameters.h`.
         * @param sys 1-byte hex code ID for command's remote system.
         * @param cmd 1-byte hex code ID for command.
         * @return bool true if the command will read continuous data from a remote ring buffer.
         * @return bool false if the command is generic.
         */
        bool check_frame_read_cmd(uint8_t sys, uint8_t cmd);
        /**
         * @brief checks if a provided command (lookup in `TransportLayerMachine::commands`) should be intercepted and directly handled by the Formatter instead of being passed on to `sys`.
         * @todo the commands to intercept are just hard-coded defaults currently. Either add an identifying field to `foxsi4-commands` or define these constants in `Parameters.h`.
         * @param sys 1-byte hex code ID for command's remote system.
         * @param cmd 1-byte hex code ID for command.
         * @return bool true if the command should be intercepted and handled by the Formatter.
         * @return bool false if the command is generic.
         */
        bool check_formatter_intercept_cmd(uint8_t sys, uint8_t cmd);
        
        /**
         * @brief implements handling of commands that should be intercepted by Formatter.
         * @return true if the command should be handled by Formatter (regardless of success).
         * @return false if the command should be handled directly by the `sys_man`.
        */
        bool handle_intercept_cmd(SystemManager& sys_man, Command cmd);

    private:
        bool do_uart;
        
        std::vector<uint8_t> uplink_swap;
        std::vector<uint8_t> tcp_local_receive_swap;
        std::vector<uint8_t> udp_local_receive_swap;
        std::vector<uint8_t> uart_local_receive_swap;

        void set_socket_options();
        void set_local_serial_options(std::shared_ptr<UART> port);
        void set_uplink_serial_options(std::shared_ptr<UART> port);

        boost::asio::io_context& io_context;
};

#endif