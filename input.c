#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/gpio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

// GLOBAL SCOPE!

struct gpiohandle_request req;
struct gpiohandle_data data;

int chip_fd;

int main(){
	int fd = open("/dev/gpiochip0", O_RDWR);
	if (fd< -1){
		perror("open");
		exit(1);
	}

	//Settings
	memset(&req,0,sizeof(req));
	req.lines = 1;
	req.lineoffsets[0] = 26;	
	req.flags = GPIOHANDLE_REQUEST_INPUT;
	strcpy(req.consumer_label, "Sensor name");

	//Chip file descripor
	if (ioctl(chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0){
		perror("chip_fd");	
		exit(1);
	}

	// ioctl sample: ioctl(req.fd, GPIO_GET_LINE_VALUES_IOCTL, &data);
	while(1){
	
	}

}
