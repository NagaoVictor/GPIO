#include "punch.h"

int  main(int argc, char*argv[]){
	// ./punch <amount> <time> 
	
	float timer = atof(argv[2]);
	int amount = atoi(argv[1]);

	srand((unsigned int)time(NULL));
	int num;

	open_device();	

	//Logic
	
	for (int i = 0; i < amount; i++){
		num = rand()%9;
		set_gpio(num);
		sleep(timer);
	}
	
	memset(&data, 0, sizeof(data));
	ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data );
	
	close_device();

}
