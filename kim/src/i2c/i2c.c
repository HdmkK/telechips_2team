#include "i2c.h"
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "gpio.h"


int i2cfile,ret48,ret49,ret4A,ret4B;
FILE *valueFile;


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


int init_i2c(){

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
    valueFile = fopen(path, "w");
    if (valueFile == NULL) {
        perror("Failed to open value file");
        return -1;
    }
    fprintf(valueFile, "1");
    fflush(valueFile);
}


int destory_i2c(){
    fclose(valueFile);
    setGPIOValue(GPIO_PIN,0);
    unexportGPIO(GPIO_PIN);
    close(i2cfile);
}