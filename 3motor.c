#include "punch.h"

int main(){
	srand((unsigned int)time(NULL));
	int num;

	open_device();
	
	for (int i = 0; i<25; i++){
		//num = rand()%3;
		set_gpio(1);
		sleep(60);
	}	
	memset(&data, 0, sizeof(data));
	ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data );

	close_device();

}
