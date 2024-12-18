#include "motor.h"


int blue[8] = {1, 1, 0, 0, 0, 0, 0, 1};
int pink[8] = {0, 1, 1, 1, 0, 0, 0, 0};
int yellow[8] = {0, 0, 0, 1, 1, 1, 0, 0};
int orange[8] = {0, 0, 0, 0, 0, 1, 1, 1};

void setstep(int p1, int p2, int p3, int p4) {

    setGPIOValue(PIN1, p1);
    setGPIOValue(PIN2, p2);
    setGPIOValue(PIN3, p3);
    setGPIOValue(PIN4, p4);
}

void backward(int round, int delay) {
    int i = 0, j = 0;

    for (i = 0; i < ONEROUND * round; i++) {
        for(j = STEPS; j >0; j--){
            setstep(blue[j], pink[j], yellow[j], orange[j]);
            usleep(delay);
        }
    }
    setstep(0, 0, 0, 0);
}

void forward(int round, int delay) {
    int i = 0, j = 0;

    for (i = 0; i < ONEROUND * round; i++) {
        for(j = 0; j < STEPS; j++) {
            setstep(blue[j], pink[j], yellow[j], orange[j]);
            usleep(delay);
        }
    }
    setstep(0, 0, 0, 0);
}

void motor_gpio_init(){
    exportGPIO(PIN1);
    exportGPIO(PIN2);
    exportGPIO(PIN3);
    exportGPIO(PIN4);
    setGPIODirection(PIN1, "out");
    setGPIODirection(PIN2, "out");
    setGPIODirection(PIN3, "out");
    setGPIODirection(PIN4, "out");
    setGPIOValue(PIN1, 0);
    setGPIOValue(PIN2, 0);
    setGPIOValue(PIN3, 0);
    setGPIOValue(PIN4, 0);

}


/*int main(){
    
    motor_gpio_init();
    forward(1,1000);
    usleep(3000*1000);
    backward(1,1000);
}*/

