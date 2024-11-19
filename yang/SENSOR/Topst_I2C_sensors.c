#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define I2C_ADDR 0x48     //0x48 module i2c
#define I2C_ADDR2 0x49    //0x4A module i2c
#define I2C_ADDR3 0x4A    //0x49 module i2c
#define SENSOR_CHANNEL1 0x40 //AN0값을 읽기 위해 필요한 cmd값.

//dust sensor timing rate
#define SAMPLING_TIME 200
#define DELAY_TIME 40
#define OFF_TIME 9680

//사용할 dust GPIO 핀 번호 (예: GPIO 15)
#define GPIO_PIN 84 //gpioc 23번 : 물리 11번  

//gpio setting tools
void exportGPIO(int pin) {
    FILE *exportFile = fopen("/sys/class/gpio/export", "w");
    if (exportFile == NULL) {
        perror("Failed to open export file");
        exit(1);
    }
    fprintf(exportFile, "%d", pin);
    fclose(exportFile);
}

void unexportGPIO(int pin) {
    FILE *unexportFile = fopen("/sys/class/gpio/unexport", "w");
    if (unexportFile == NULL) {
        perror("Failed to open unexport file");
        exit(1);
    }
    fprintf(unexportFile, "%d", pin);
    fclose(unexportFile);
}

void setGPIODirection(int pin, const char *direction) {
    char path[35];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    FILE *directionFile = fopen(path, "w");
    if (directionFile == NULL) {
        perror("Failed to open direction file");
        exit(1);
    }
    fputs(direction, directionFile);
    fclose(directionFile);
}

void setGPIOValue(int pin, int value) {
    char path[30];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    FILE *valueFile = fopen(path, "w");
    if (valueFile == NULL) {
        perror("Failed to open value file");
        exit(1);
    }
    fprintf(valueFile, "%d", value);
    fclose(valueFile);
}

//i2c data read
int get_data_from_addr(int file,int addr)
{
    char buffer[2];
    //buffer give 0x40 -> AN0 output
    buffer[0] = SENSOR_CHANNEL1;
    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        perror("Failed to connect to the sensor");
        close(file);
        return 1;
    }
    if (write(file, buffer, 1) != 1) {
       perror("Failed to set channel");
       close(file);
       return 1;
    }
    if (read(file, buffer, 2) != 2) {
        perror("Failed to read data from the sensor");
        close(file);
        return 1;
    }
    // buffer[0] = 8bit output from ADC
    return buffer[0];
}

int main() {
    int i2cfile,ret48,ret49,ret4A;
    char *i2cfilename = "/dev/i2c-1";
    
    //i2cfile open
    if ((i2cfile = open(i2cfilename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return -1;
    }

    exportGPIO(GPIO_PIN);
    setGPIODirection(GPIO_PIN, "out");
    
    //set gpio valueFile
    char path[30];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", GPIO_PIN);
    FILE *valueFile = fopen(path, "w");
    if (valueFile == NULL) {
        perror("Failed to open value file");
        return -1;
    }
    fprintf(valueFile, "1");
    fflush(valueFile);
    
    //sleep 10ms
    usleep(10*1000);
    for(int i = 0; i<100; i++) {
        ret48=ret49=ret4A=0;
        fprintf(valueFile, "0");
        fflush(valueFile);
        //200us
        usleep(SAMPLING_TIME);
        //80us
        ret4A = get_data_from_addr(i2cfile,I2C_ADDR3);
        fprintf(valueFile, "1");
        fflush(valueFile);
        usleep(OFF_TIME);
        usleep(100*1000);
        ret48 = get_data_from_addr(i2cfile,I2C_ADDR);
        usleep(100*1000);
        ret49 = get_data_from_addr(i2cfile,I2C_ADDR2);
        //printf for debug
        printf("ret48-air: %d, ret49-rain: %d, ret4A-dust: %d \n",ret48, ret49, ret4A);
    }
    
    fclose(valueFile);
    setGPIOValue(GPIO_PIN,0);
    unexportGPIO(GPIO_PIN);
    close(i2cfile);
    return 0;
}


