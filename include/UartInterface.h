/**
 * @file UartInterface.h
 * @author Kris Cooper
 * @brief Interface for UART using termios.
 * @version v1.0.1
 * @date 2024-03-11
 */
#ifndef UARTINTERFACE_H
#define UARTINTERFACE_H

// C Library headers
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdint.h> 
#include <vector>

// Linux headers
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h> 

// maximum buffer byte size for received message
const int BUFFER_MAX_SIZE = 1000000; 

// function to help print the bytes being sent/received
int print_bl(int, std::vector<uint8_t>);
int print_bl(int, uint8_t*);

// UART class
class UARTPort {
	public:
        // open the default UART0/1 port
		UARTPort(const char* port_file="/dev/ttyAMA1", uint32_t baud_rate=B9600, int vtime=10, int vmin=0, std::string parity="clear", int stop_bits=1, int bits_in_byte=CS8);

        //unsigned int baud_rate, unsigned short parity_bits, unsigned short data_bits, unsigned short stop_bits …
        struct termios tty; // create TERMIOS structure
        int fd;	// the thing you get back when you open() a port
        int serial2tty(); // send serial port attrs to the tty object
        int attrs2tty(); // set the changed attrs to the tty object
        int setup(uint32_t baud_rate, int vtime, int vmin, std::string parity, int stop_bits, int bits_in_byte);

        // methods to set port settings with select options
        int set_parity(std::string parity);
        int set_stop(int stop_bits);
         
        // allow the UART timout (ds) and minimum bytes to read be set whenever
        int vsettings(int vtime, int vmin);
        // allow the Baud rate to be set whenever
        int baudsettings(uint32_t baud_rate);
        
        // define message vector type and function to send to the port
        std::vector<uint8_t> msg; // the message
		int send(std::vector<uint8_t> msg);

        // define buffer type and function to read the port
        std::vector<uint8_t> buff; // the buffer
		int recv(std::vector<uint8_t> buff);

        uint8_t msg_len; // the size of the message
        std::vector<uint8_t> recv_msg; // the message
        uint8_t recv_bytes; // received number of bytes

        // close the port
        int close_fd();
        ~UARTPort();
};
#endif