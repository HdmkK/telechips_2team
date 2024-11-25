#ifndef __MOTOR_H__
#define __MOTOR_H__


//#define PIN1 84 //gpioc 23 : 물리 11번 06 blue -> yellow
#define PIN1 86 //gpioc 25 : 물리 15번 06 blue -> yellow
#define PIN2 85 //gpioc 24 : 물리 13번 13 pink -> green
#define PIN3 90 //gpioc 29 : 물리 16번 19 yellow -> blue
#define PIN4 65 //gpioc 04 : 물리 18번 26 orange -> purple

#define STEPS 8
#define ONEROUND 512


void motor_gpio_init();
void forward(int round, int delay);
void backward(int round, int delay);

#endif