#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <string.h>


struct gpiohandle_request req;
struct gpiohandle_data data;

int chip_fd;

void setup(){
	chip_fd = open("/dev/gpiochip0", O_RDWR);
        if (chip_fd < 0){
                perror("open");
                exit(1); 
        }

        memset(&req, 0, sizeof(req));

        req.lines = 1;
        req.lineoffsets[0] = 26;
	req.flags = GPIOHANDLE_REQUEST_OUTPUT;
        strcpy(req.consumer_label, "leds");

        if (ioctl(chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0){
                perror("ioctl");
                exit(1);
        }

}
void tremer(){
        memset(&data, 0, sizeof(data));
        data.values[0] = 1;
        ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &req);

}

void parar(){
	memset(&data, 0 , sizeof(data));
	data.values[0] = 0;
	ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &req);
}


void dclose(){
	close(req.fd);
	close(chip_fd);

}


struct termios newt, oldt;

void enable_raw_mode(){
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VMIN] = 1;
  newt.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSANOW, &newt); 
}

void disable_raw_mode(){
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}


int main(){
  	char c;
  	enable_raw_mode();
  	setup();
	while(1){

    		read(STDIN_FILENO, &c, 1); 
    		if (c == 'q' || c == 'Q') break;
    
    		if (c == 'w'){tremer();}
		else {parar();}
  	}
	parar();
  	disable_raw_mode();
	dclose();
	return 0;
}

