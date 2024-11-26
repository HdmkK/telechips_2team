#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define I2C_ADDR 0x48     //0x48 module i2c
#define SENSOR_CHANNEL1 0x40 //A0값을 읽기 위해 필요한 cmd값.

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
    int file,ret48;
    char *filename = "/dev/i2c-1";

    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return -1;
    }

    for(int i = 0; i<60; i++) {
	ret48=0;
        ret48 = get_data_from_addr(file,I2C_ADDR);
	usleep(100*1000);
        
        printf("ret48: %d \n", ret48);
        
        usleep(100*1000);
    }

    close(file);
    return 0;
}
