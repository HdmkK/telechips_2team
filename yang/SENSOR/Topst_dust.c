#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <time.h>

#define I2C_ADDR2 0x4A    //쓸 address
#define SENSOR_CHANNEL1 0x40 //A0값을 읽기 위해 필요한 cmd값.

#define SAMPLING_TIME 200 //240is too slow
#define DELAY_TIME 40
#define OFF_TIME 9680

#define GPIO_PIN 86  // 사용할 GPIO 핀 번호 (예: GPIO 15)

struct timespec start, end;
double elapsed;

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

int get_data_from_addr(int file,int addr)
{
    char buffer[2];
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
    int val = buffer[0];
    return val;
}

int main() {
    int i2cfile,ret4A;
    char *filename = "/dev/i2c-1";
    
    //i2cfile open
    if ((i2cfile = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return -1;
    }
    exportGPIO(GPIO_PIN);
    setGPIODirection(GPIO_PIN, "out");
    
    //set valueFile
    char path[30];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", GPIO_PIN);
    FILE *valueFile = fopen(path, "w");
    if (valueFile == NULL) {
        perror("Failed to open value file");
        return(1);
    }
    fprintf(valueFile, "1");
    fflush(valueFile);
    usleep(10000);
    for(int i = 0; i<100; i++) {
        ret4A=0;
        fprintf(valueFile, "0");
        fflush(valueFile);
        usleep(SAMPLING_TIME);//200us
        ret4A = get_data_from_addr(i2cfile,I2C_ADDR2);
        fprintf(valueFile, "1");
        fflush(valueFile);
        usleep(OFF_TIME);
        printf("ret4A: %d \n", ret4A);
	usleep(1000000);
    }
    
    fclose(valueFile);
    setGPIOValue(GPIO_PIN,0);
    unexportGPIO(GPIO_PIN);
    close(i2cfile);
    return 0;
}
