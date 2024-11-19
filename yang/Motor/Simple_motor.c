#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/cdev.h>

#define PIN1 84 //gpioc 23 : 물리 11번
#define PIN1 85 //gpioc 24 : 물리 13번
#define PIN1 90 //gpioc 29 : 물리 16번
#define PIN1 65 //gpioc 04 : 물리 18번

#define STEPS 8
#define ONEROUND 512

int blue[8] = {1, 1, 0, 0, 0, 0, 0, 1};
int pink[8] = {0, 1, 1, 1, 0, 0, 0, 0};
int yellow[8] = {0, 0, 0, 1, 1, 1, 0, 0};
int orange[8] = {0, 0, 0, 0, 0, 1, 1, 1};

void setstep(int p1, int p2, int p3, int p4) {
    gpio_set_value(PIN1, p1);
    gpio_set_value(PIN2, p2);
    gpio_set_value(PIN3, p3);
    gpio_set_value(PIN4, p4);
}

void backward(int round, int delay) {
    int i = 0, j = 0;

    for (i = 0; i < ONEROUND * round; i++) {
        for(j = STEPS; j >0; j++;){
            setstep(blue[j], pink[j], yellow[j], orange[j]);
            usleep(delay);
        }
    }
    setstep(0, 0, 0, 0);
}

void forward(int round, int delay) {
    int i = 0, j = 0;

    for (i = 0; i < ONEROUND * round; i++) {
        for(j = STEPS; j >0; j++;)
            setstep(blue[j], pink[j], yellow[j], orange[j]);
            usleep(delay);
    }
    setstep(0, 0, 0, 0);
}

static int __init simple_motor_init(void) {
    gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
    gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p2");
    gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p3");
    gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p4");

    forward(1,3000);
    mdelay(3000);
    backward(1,1500);
}

static void __exit simple_motor_exit(void) {
    gpio_free(PIN1);
    gpio_free(PIN2);
    gpio_free(PIN3);
    gpio_free(PIN4);
}

module_init(simple_motor_init);
module_exit(simple_motor_exit);
