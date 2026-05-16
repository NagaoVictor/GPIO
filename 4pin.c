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

        req.lines = 4;
        req.lineoffsets[0] = 26; //1
        req.lineoffsets[1] = 19; //2
	req.lineoffsets[2] = 13; //3
	req.lineoffsets[3] = 6; //4

	req.flags = GPIOHANDLE_REQUEST_OUTPUT;
        strcpy(req.consumer_label, "leds");

        if (ioctl(chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0){
                perror("ioctl");
                exit(1);
        }

}
void reset(){
        memset(&data, 0, sizeof(data));
        data.values[0] = 0;
        data.values[1] = 0;
	data.values[2] = 0;
        data.values[3] = 0;
        ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
}

void backward(){
	memset(&data, 0, sizeof(data));
	data.values[0] = 0;
	data.values[1] = 0;
	data.values[2] = 1;
        data.values[3] = 0;
	ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
}

void forward(){
	memset(&data, 0, sizeof(data));
        data.values[0] = 0;
        data.values[1] = 1;
	data.values[2] = 0;
        data.values[3] = 0;
        ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
}

void leftward(){
	memset(&data, 0, sizeof(data));
        data.values[0] = 1;
        data.values[1] = 0;
	data.values[2] = 0;
        data.values[3] = 0;
        ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
}

void rightward(){
	memset(&data, 0, sizeof(data));
        data.values[0] = 0;
        data.values[1] = 0;
	data.values[2] = 0;
        data.values[3] = 1;
        ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);

}


void dclose(){
	close(req.fd);
	close(chip_fd);

}


typedef struct {
  char type;
  long timestamp;
} Event;

// Struct method

void reg(char c){
   Event e;
   e.type = c;
   e.timestamp=time(NULL);
   
   if (c == 'w' || c == 'W'){
      printf("Forward | %ld Seconds since 1970\n", e.timestamp);
    } else if (c == 's' || c == 'S'){
       printf("Backward | %ld Seconds since 1970\n", e.timestamp);
    } else if (c == 'a' || c == 'A'){
      printf("Leftside | %ld Seconds since 1970\n", e.timestamp);
    } else if (c == 'd' || c == 'D'){
      printf("Rightside | %ld Seconds since 1970\n", e.timestamp);
    } else if (c == 'r' || c == 'R'){
      printf("Reset | %ld Seconds since 1970\n", e.timestamp);
    } else {
      printf("Command: %c | %ld Seconds since 1970\n", e.type, e.timestamp);
    }
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
    		if (c == 'q' | c == 'Q') break;
    		reg(c);
    		if (c == 'w'){ forward();}
    		else if (c == 'a'){ leftward();}
		else if (c == 'd'){ rightward();}
		else if (c == 's'){ backward();}
		else if (c == 'r'){ reset();}
		else {reset();}

  	}
	reset();
  	disable_raw_mode();
	dclose();
	return 0;
}

