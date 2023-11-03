#include "UartInterface.h"

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

#include <cstdlib>
#include <unistd.h>



// maximum buffer byte size for received message
const int BUFFER_MAX_SIZE = 1000000; 



// custom print functions
int print_bl(int num_bytes, std::vector<uint8_t> read_buf) {
    // overloaded function to print out a vector
    int i;
    printf("{");
    for (i=0; i<num_bytes; i++) {
	    printf("%02x ", read_buf[i]);
    }
    printf("}\n");
    return 0;
}
int print_bl(int num_bytes, uint8_t* read_buf) {
    // overloaded function to print out a array
    int i;
    printf("{");
    for (i=0; i<num_bytes; i++) {
	    printf("%02x ", read_buf[i]);
    }
    printf("}\n");
    return 0;
}



UARTPort::UARTPort(const char* port_file) {
    // open the UART port
    // UARTPort::fd = open("/dev/ttyS0", O_RDWR);
    // UARTPort::fd = open("/dev/ttyAMA1", O_RDWR);
    UARTPort::fd = open(port_file, O_RDWR);
};

int UARTPort::serial2tty(){
    // get parameters associated with serial_port and store them in tty
    if (tcgetattr(UARTPort::fd, &tty) != 0 ) {
	    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	    return 1;
    };

    return 0;
};

int UARTPort::attrs2tty(){
    // set parameters associated with serial_port in tty
    if (tcsetattr(UARTPort::fd, TCSANOW, &tty) != 0) {
	    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	    return 1;
    };
    
    return 0;
};

int UARTPort::set_parity(std::string parity){
    // handle code to set parity of UART port
    if (parity=="clear"){
        UARTPort::tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity
    }else if (parity=="set"){
        UARTPort::tty.c_cflag |= PARENB; // Set parity bit, enabling parity
    }else{
        std::cout<<"Parity not cleared or set with: "<<parity<<", Choose \'clear\' or\'set\'.\n";
        return 1;
    };
    
    return 0;
};

int UARTPort::set_stop(int stop_bits){
    // handle code to set stop bits of UART port
    if (stop_bits==1){
        UARTPort::tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    }else if (stop_bits==2){
        UARTPort::tty.c_cflag |= CSTOPB; // Set stop field, two stop bits used in communication
    }else{
        std::cout<<"Stop bits (CSTOPB) not cleared or set with: "<<stop_bits<<", Choose \'1\' or\'2\'.\n";
        return 1;
    }
    
    return 0;
};

int UARTPort::setup(uint32_t baud_rate, int vtime, int vmin, std::string parity, int stop_bits, int bits_in_byte) {
    // baud_rate : Baud Rate for comminications
    //          B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400,
    //          B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000,
    //          B576000, B921600, B1000000, B1152000, B1500000, B2000000, but others may 
    //          be available. The B0 option is used to terminate the connection.
    //          DEFAULT: B9600
    //
    // vtime : Timeout time for receiving a message
    //          Integer value in ds (deci-seconds) for when to read the receiving buffer 
    //          DEFAULT: 10
    //
    // vmin : Minimum number of bytes
    //          Integer bytes to wait for before reading receiving buffer
    //          DEFAULT: 0
    //
    // parity : Whether parity is used for each byte
    //          Either "clear" or "set"
    //          DEFAULT: "clear"
    //
    // stop_bits : Number of stop bits to use
    //          Either integer 1 or 2.
    //          DEFAULT: 1
    //
    // bits_in_byte : number of bits in one message byte
    //          CS5, CS6, CS7, or CS8 (5, 6, 7, or 8 bits in a byte)
    //          DEFAULT: CS8
    //

    // get parameters associated with serial_port and store them in tty
    UARTPort::serial2tty();

    // add in settings for the communications
    // tty.c_cflag = tty.c_cflag & ~PARENB is the same as tty.c_cflag &= ~PARENB
    // &= ~PARENB is to clear and |= PARENB is to set
    // a lot from: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
    UARTPort::set_parity(parity);
    UARTPort::set_stop(stop_bits);

    UARTPort::tty.c_cflag &= ~CSIZE; // Clear all the size bits, then set number below
    UARTPort::tty.c_cflag |= bits_in_byte; // a byte is 8 bits, after CSIZE
    UARTPort::tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control 
    UARTPort::tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
	
    UARTPort::tty.c_lflag &= ~ICANON; //disable canonical mode (terminal stuff)
    UARTPort::tty.c_lflag &= ~ECHO; // Disable echoing send message
    UARTPort::tty.c_lflag &= ~ECHOE; // Disable erasure (?)
    UARTPort::tty.c_lflag &= ~ECHONL; // Disable new-line echo
    UARTPort::tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT, SUSP, and DSUSP
    UARTPort::tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl (might help to clear buffer when new messages arrive?)
    UARTPort::tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes
    
    UARTPort::tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    UARTPort::tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    // set parameters associated with serial_port in tty
    UARTPort::attrs2tty();

    // set Baud Rate
    UARTPort::baudsettings(baud_rate);

    // default is int vtime = 10, int vmin = 0
    UARTPort::vsettings(vtime, vmin);

    return 0;
};

int UARTPort::baudsettings(uint32_t baud_rate){
    // baud_rate : Baud Rate for comminications
    //          B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400,
    //          B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000,
    //          B576000, B921600, B1000000, B1152000, B1500000, B2000000, but others may 
    //          be available. The B0 option is used to terminate the connection.
    //          DEFAULT: B9600

    // get parameters associated with serial_port and store them in tty
    UARTPort::serial2tty();
	
    cfsetispeed(&tty, baud_rate);
    cfsetospeed(&tty, baud_rate);

    // set parameters associated with serial_port in tty
    UARTPort::attrs2tty();

    return 0;
};

int UARTPort::vsettings(int vtime, int vmin){
    // vtime : Timeout time for receiving a message
    //          Integer value in ds (deci-seconds) for when to read the receiving buffer 
    //          DEFAULT: 10
    //
    // vmin : Minimum number of bytes
    //          Integer bytes to wait for before reading receiving buffer
    //          DEFAULT: 0

    // default is VTIME = 10 deci-seconds, tenths of a second to timeout and read
    // 0 is the minimum number of bytes to read

    // get parameters associated with serial_port and store them in tty
    UARTPort::serial2tty();
	
    UARTPort::tty.c_cc[VTIME] = vtime; // deci-seconds, tenths of a second to timeout and read
    UARTPort::tty.c_cc[VMIN] = vmin; // minimum number of bytes to read

    // set parameters associated with serial_port in tty
    UARTPort::attrs2tty();

    return 0;
};

int UARTPort::send(std::vector<uint8_t> msg){

    tcflush(UARTPort::fd,TCIOFLUSH);

    // set fields for the object
    UARTPort::msg_len = msg.size();
    UARTPort::msg = msg;

    // confirm frame is set-up and what is expected
    printf("Sending frame:\n");
    print_bl(UARTPort::msg_len, UARTPort::msg);
    printf("Sent bytes:%i\n", UARTPort::msg_len);

    // write to the port
    write(UARTPort::fd, &msg[0], UARTPort::msg_len);
    
    // confirm frame is sent
    printf("Sent frame.\n");
    
    return 0;
};

int UARTPort::recv(std::vector<uint8_t> buff){

    // confirm length of buffer
    printf("Buffer is %i bytes long.\n",buff.size());
    
    // pass pointer to the first data location of `buff`
    int num_bytes = read(UARTPort::fd, &buff[0], BUFFER_MAX_SIZE);//buff.size()

    // check we've read bytes
    if (num_bytes < 0) {
	    printf("Error reading: %s\n",strerror(errno));
	    return 1;
    }

    // save fields
    UARTPort::recv_msg = buff;
    UARTPort::recv_bytes = num_bytes;

    // confirm the number of read bytes and what's been read
    printf("Read %i bytes. Received message:\n", num_bytes);
    print_bl(UARTPort::recv_bytes, UARTPort::recv_msg);

    tcflush(UARTPort::fd,TCIOFLUSH);
    
    return num_bytes;
};

int UARTPort::close_fd() {
    // close the UART port
    close(UARTPort::fd);
    return 0;
};

UARTPort::~UARTPort() {
    // close the UART port
    UARTPort::close_fd();
};



const std::vector<uint8_t> FRAME0 = {0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D};
const std::vector<uint8_t> FRAME1 = {0x03, 0x02, 0x05, 0x07};
const std::vector<uint8_t> FRAME2 = {0x08, 0x1e, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x6f};

int test0() {
    // send-receive 

    // create UART port object and message to send
    UARTPort uart;
    
    // set up the UART port
    uart.setup();

    // send the frame information
    uart.send(FRAME0);

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    // send buffer to the reading port method
    uart.recv(buffer);

    // make sure to close the port
    uart.close_fd();

    // print new line
    printf("Test0.\n");
    printf("\n");

    return 0;
};

int test1() {
    // send-receive wait time of 1,000 ms

    // create UART port object and message to send
    UARTPort uart;
    
    // set up the UART port
    uart.setup();

    // send the frame information
    uart.send(FRAME0);

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    //delay
    usleep(1000); //microseconds

    // send buffer to the reading port method
    uart.recv(buffer);

    // make sure to close the port
    uart.close_fd();

    // print new line
    printf("Test1. 1,000 us delay between write and read.\n");
    printf("\n");

    return 0;
};

int test2() {
    // send-receive wait time of 10,000 ms

    // create UART port object and message to send
    UARTPort uart;
    
    // set up the UART port
    uart.setup();

    // send the frame information
    uart.send(FRAME0);

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    //delay
    usleep(10000); //microseconds

    // send buffer to the reading port method
    uart.recv(buffer);

    // make sure to close the port
    uart.close_fd();

    // print new line
    printf("Test2. 10,000 us delay between write and read.\n");
    printf("\n");

    return 0;
};

int test3() {
    // send-receive wait time of 100,000 ms

    // create UART port object and message to send
    UARTPort uart;
    
    // set up the UART port
    uart.setup();

    // send the frame information
    uart.send(FRAME0);

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    //delay
    usleep(100000); //microseconds

    // send buffer to the reading port method
    uart.recv(buffer);

    // make sure to close the port
    uart.close_fd();

    // print new line
    printf("Test3. 100,000 us delay between write and read.\n");
    printf("\n");

    return 0;
};

int test4() {
    // change vtime and vmin while sending messages

    // create UART port object and message to send
    UARTPort uart;
    
    // set up the UART port
    uart.setup();

    // send the frame information
    uart.send(FRAME0);

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    // send buffer to the reading port method
    uart.recv(buffer);

    // try to change the settings to wait and catch full message
    uart.vsettings(5, FRAME0.size());
    uart.send(FRAME0);
    uart.recv(buffer);

    // make sure to close the port
    uart.close_fd();

    // print new message but the same number of bytes as the first one
    print_bl(uart.recv_bytes+5, uart.recv_msg);

    // print new line
    printf("Test4. Same as test0 then change vtime and vmin for the same message again (see what is left in the buffer).\n");
    printf("\n");

    return 0;
};

int test5() {
    // different sized frames

    // create UART port object and message to send
    UARTPort uart;
    
    // set up the UART port
    uart.setup();

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    // try to change the settings to wait and catch full message
    uart.vsettings(5, FRAME0.size());
    uart.send(FRAME0);
    uart.recv(buffer);
    int num_bytes = uart.recv_bytes;

    // send a smaller frame after a longer one
    uart.send(FRAME1);
    uart.recv(buffer);

    // print new message but the same number of bytes as the first one
    printf("Read %i bytes (1st message value). Received message:\n", num_bytes);
    print_bl(num_bytes, uart.recv_msg);

    // make sure to close the port
    uart.close_fd();

    // print new line
    printf("Test5. First frame is long, second is short.\n");
    printf("\n");

    return 0;
};

int test6() {
    // different sized frames

    // create UART port object and message to send
    UARTPort uart;
    
    // set up the UART port
    uart.setup();

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    // try to change the settings to wait and catch full message
    uart.vsettings(5, FRAME0.size());
    uart.send(FRAME0);
    uart.recv(buffer);
    int num_bytes = uart.recv_bytes;

    // send a smaller frame after a longer one
    uart.vsettings(5, FRAME1.size());
    uart.send(FRAME1);
    uart.recv(buffer);

    // print new message but the same number of bytes as the first one
    printf("Read %i bytes (1st message value). Received message:\n", num_bytes);
    print_bl(num_bytes, uart.recv_msg);

    // send a larger frame after a shorter one
    uart.vsettings(5, FRAME2.size());
    uart.send(FRAME2);
    uart.recv(buffer);

    // make sure to close the port
    uart.close_fd();

    // print new line
    printf("Test6. First frame is long, second is short, third is longer again and scale vmin to match.\n");
    printf("\n");

    return 0;
};

int test7() {
    // send-receive 

    // create UART port object and message to send
    UARTPort uart;
    
    // set up the UART port
    uart.setup();
    uart.baudsettings(B57600);

    // send the frame information
    uart.send(FRAME0);

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    // send buffer to the reading port method
    uart.recv(buffer);

    // make sure to close the port
    uart.close_fd();

    // print new line
    printf("Test7. Same as Test0 but with 6xBaud rate.\n");
    printf("\n");

    return 0;
};

int test_collection_initial(){
    // simple use case
    test0();
    // test with 1,000 us between writing and reading 
    test1();
    // test with 10,000 us between writing and reading 
    test2();
    // test with 100,000 us between writing and reading 
    test3();
    // change the vtime and vmin after set-up
    test4();
    // change size of message
    test5();
    // change size of message and adjust vmin
    test6();
    // 6x Baud rate of Test0
    test7();

    return 0;
}

int test8() {
    // send-receive 

    // create UART port object and message to send
    UARTPort uart("/dev/ttyAMA1");
    
    // set up the UART port
    uart.setup();

    // send the frame information
    uart.send(FRAME0);

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    // send buffer to the reading port method
    uart.recv(buffer);

    // make sure to close the port
    uart.close_fd();

    // print new line
    printf("Test8. Change from default UART0/1 port (/dev/ttyS0) to UART3 (/dev/ttyAMA1).\n");
    printf("\n");

    return 0;
};

int test9() {
    // send-receive 

    // create UART port object and message to send
    UARTPort uart01;
    UARTPort uart3("/dev/ttyAMA1");
    
    // set up the UART port
    uart01.setup();
    // uart01.vsettings(5, FRAME0.size());
    uart3.setup();
    // uart3.vsettings(5, FRAME0.size());

    // send the frame information
    uart01.send(FRAME0);
    uart3.send(FRAME0);

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer01(BUFFER_MAX_SIZE);
    std::vector<uint8_t> buffer3(BUFFER_MAX_SIZE);

    // send buffer to the reading port method
    uart01.recv(buffer01);
    uart3.recv(buffer3);

    // make sure to close the port
    uart01.close_fd();
    uart3.close_fd();

    // print new line
    printf("Test9. Test default UART0/1 port (/dev/ttyS0) and UART3 (/dev/ttyAMA1).\n");
    printf("\n");

    return 0;
};

int test_collection_port(){
    // test another port enabled 
    test8();
    // test two ports
    test9();

    return 0;
}


int main() {
    // initial, simple tests to check default message sending
    test_collection_initial();
    // port tests
    test_collection_port();
};