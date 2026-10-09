#include "punch.h"

int main(){
	srand((unsigned int)time(NULL));
	int num;

	open_device();
	
	for (int i = 0; i<2; i++){
		set_gpio(0);
		sleep(2);
	}	

	reset();

	close_device();

}
