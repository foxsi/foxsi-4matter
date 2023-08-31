// C Library headers
#include <stdio.h>
#include <string.h>
#include <stdint.h> 

// Linux headers
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h> 

int print_bl(int num_bytes, uint8_t* read_buf) {
    int i;
    printf("{");
    for (i=0; i<num_bytes; i++) {
	printf("%02x ", read_buf[i]);
    }
    printf("}\n");
    return 0;
}

int main() {
    int serial_port = open("/dev/ttyS0", O_RDWR);
    
    struct termios tty;
    
    if (tcgetattr(serial_port, &tty) != 0 ) {
	printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	return 1;
    }
    
    tty.c_cflag &= ~PARENB; //tty.c_cflag = tty.c_cflag & ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
	
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;
    
    uint8_t frame[] = {0x7B, 0x03, 0x02, 0x05, 0x07, 0x7D};
    int frame_size = sizeof(frame);
	
    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = frame_size;
    
    cfsetispeed(&tty, B460800);
    cfsetospeed(&tty, B460800);
    
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
	printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	return 1;
    }
    
    write(serial_port, frame, frame_size);
    printf("Sent frame:\n");
    print_bl(frame_size, frame);
    
    uint8_t read_buf[256];
    
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    
    if (num_bytes < 0) {
	printf("Error reading: %s\n",strerror(errno));
	return 1;
    }
    
    printf("Read %i bytes. Received message:\n", num_bytes);
    print_bl(num_bytes, read_buf);
    
    close(serial_port);
    return 0;
}


