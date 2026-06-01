#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <string.h>
#include <stdlib.h>

struct gpiohandle_request req;
struct gpiohandle_data data;

int chip_fd;

int main(){
	// Device file descriptor
	int chip_fd = open("/dev/gpiochip0", O_RDWR);
	if (chip_fd < 0){
		perror("Open");
		exit(1);
	}
	
	// Set gpio request
	memset(&req, 0, sizeof(req));
	req.lines = 1;
	req.lineoffsets[0] = 17;
	req.flags = GPIOHANDLE_REQUEST_INPUT; // INPUT SENSOR
	strcpy(req.consumer_label, "Flying_Fish");

	// Chip file descriptor
	if (ioctl(chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0){
		perror("ioctl chip");
		exit(1);	
	} 
	
	// System Logic
	while(1){
		ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
		printf("%d\n", data.values[0]);
		usleep(100000);
	}

}
