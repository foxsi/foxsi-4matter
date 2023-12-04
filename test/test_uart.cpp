#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "UartInterface.h"

// some example messages
const std::vector<uint8_t> FRAME0 = {0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D};
const std::vector<uint8_t> FRAME1 = {0x03, 0x02, 0x05, 0x07};
const std::vector<uint8_t> FRAME2 = {0x08, 0x1e, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D, 0x7B, 0x03, 0x02, 0x05, 0x07, 0x6f};

int test0() {
    // send-receive 

    // create UART port object and message to send
    UARTPort uart;

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
    
    // change Baud rate after set-up
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

// for multithreading tests
UARTPort send_msg(){
    // create UART port object and message to send
    UARTPort uart("/dev/ttyAMA1");

    // send the frame information
    uart.send(FRAME0);
    return uart;
};
// for multithreading tests
int receive_msg(UARTPort uart_port, std::vector<uint8_t> buffer){
    printf("Pausing thread for 5 seconds.\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(5000)); //make the thread wait for 5 seconds
    printf("Continuing thread.\n");
    // send buffer to the reading port method
    uart_port.recv(buffer);

    // make sure to close the port
    uart_port.close_fd();
    return 0;
};
// for multithreading tests
int loop_loop_loop(){
    for (int i = 0; i <= 12; i++) {
        printf("Main thread process.\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); //make the thread wait for 0.5 seconds
    };
    return 0;
}; 
int test10() {
    // send-receive 

    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    // send the message
    UARTPort uart = send_msg();
    // spawn a thread to wait for the message to come in 
    std::thread thread_obj(&receive_msg, uart, buffer);
    thread_obj.join();
    print_bl(BUFFER_MAX_SIZE, buffer);

    // print new line
    printf("Test10. Send a mesage and then spawn a thread to wait for the message and populate a buffer.\n");
    printf("\n");

    return 0;
};

int test11() {
    // send-receive 
    // define a vector to be used as a buffer 
    std::vector<uint8_t> buffer(BUFFER_MAX_SIZE);

    // send the message
    UARTPort uart = send_msg();
    // spawn a thread to wait for the message to come in 
    std::thread thread_obj(&receive_msg, uart, buffer);
    loop_loop_loop(); // make sure this executes while the thread is paused 
    print_bl(BUFFER_MAX_SIZE, buffer);

    // print new line
    printf("Test11. Send a mesage and then spawn a thread to wait for the message and populate a buffer. Make sure non-blocking.\n");
    printf("\n");

    return 0;
};

int test_multithreading(){
    // test a thread can be opened that is waiting to receive a message
    test10();
    // test a thread can be opened that is waiting to receive a message but making sure it isn't blocking the main thread
    test11();

    return 0;
}


int main() {
    // initial, simple tests to check default message sending
    test_collection_initial();
    // port tests
    test_collection_port();
    // multithreading tests
    test_multithreading();

    return 0;
};