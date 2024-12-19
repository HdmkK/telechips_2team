#ifndef __I2C_H__
#define __I2C_H__


#include <stdio.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "gpio.h"

#define I2C_ADDR 0x48     //0x48 module i2c
#define I2C_ADDR2 0x49    //0x4A module i2c
#define I2C_ADDR3 0x4A    //0x49 module i2c
#define I2C_ADDR4 0x4B    //0x4B module i2c
#define SENSOR_CHANNEL1 0x40 //AN0값을 읽기 위해 필요한 cmd값.

//dust sensor timing rate
#define SAMPLING_TIME 200
#define DELAY_TIME 40
#define OFF_TIME 9680

//사용할 dust GPIO 핀 번호 (예: GPIO 15)
#define GPIO_PIN 84 //gpioc 23번 : 물리 11번  

extern int i2cfile,ret48,ret49,ret4A,ret4B;
extern FILE *valueFile;

int get_data_from_addr(int file,int addr);
int init_i2c();
int destory_i2c();


#endif