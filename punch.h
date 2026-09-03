#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <string.h>
#include <errno.h>

#define PORT1 26
#define PORT2 19
#define PORT3 13
#define PORT4 6
#define PORT5 5
#define PORT6 12
#define PORT7 16
#define PORT8 20
#define PORT9 21

struct gpiohandle_request req;
struct gpiohandle_data data;
int fd;

void set_gpio(int value){
	memset(&data, 0, sizeof(data));
	data.values[value] = 1;
	ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
}

int open_device(){
	fd = open("/dev/gpiochip0", O_RDWR, 0666);
	if (fd < 0){
		perror("OPEN DEV");
		return EXIT_FAILURE;
	}


	memset(&req, 0, sizeof(req));
	req.lines = 9;
	req.lineoffsets[0] = PORT1;
	req.lineoffsets[1] = PORT2;
	req.lineoffsets[2] = PORT3;
	req.lineoffsets[3] = PORT4;
	req.lineoffsets[4] = PORT5;
	req.lineoffsets[5] = PORT6;
	req.lineoffsets[6] = PORT7;
	req.lineoffsets[7] = PORT8;
	req.lineoffsets[8] = PORT9;
	req.flags = GPIOHANDLE_REQUEST_OUTPUT;

	if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req)<0){
		fprintf(stderr, "chip_fd ioctl failure %s, cod(%d)", strerror(errno), errno);
		close(fd);
		return EXIT_FAILURE;
	}
}

void close_device(){
	close(fd);
}

