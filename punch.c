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

	reset();
	close_device();

}
