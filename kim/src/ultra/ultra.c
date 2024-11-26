#include "gpio.h"
#include "ultra.h"
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>


void ultra_gpio_init(){

	exportGPIO(TRIG);
	setGPIODirection(TRIG, "out");

	exportGPIO(ECHO);
	setGPIODirection(ECHO, "in");


	setGPIOValue(TRIG, 0);
	setGPIOValue(ECHO, 1);
}

static void trigger_ultra(){
	setGPIOValue(TRIG, 1);
	usleep(10);
	setGPIOValue(TRIG, 0);

}

static unsigned long get_microseconds() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000000 + time.tv_usec;
}

int get_distance(){
	trigger_ultra();

	while (getGPIOValue(ECHO) != 1);
	unsigned long start_time = get_microseconds();
	while (getGPIOValue(ECHO) != 0);
	unsigned long end_time = get_microseconds();

	return (end_time - start_time)/58;
}



/*int main(){
	int cm;

	ultra_gpio_init();

	while(1){

		cm = read_distance();
		//printf("hello\n");
		printf("%dcm\n", cm);
		//sleep(1);
	}
}*/