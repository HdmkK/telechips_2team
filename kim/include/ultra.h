#ifndef __ULTRA_H__
#define __ULTRA_H__

#define TRIG 112//물리 29
#define ECHO 113//물리 31

void ultra_gpio_init();
int get_distance();

#endif