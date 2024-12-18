#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>


void exportGPIO(int pin);
void unexportGPIO(int pin);
void setGPIODirection(int pin, const char *direction);
void setGPIOValue(int pin, int value);
int getGPIOValue(int pin);


#endif
