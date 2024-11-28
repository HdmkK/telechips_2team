#include "i2c.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// I2C 데이터 읽기 함수
int get_data_from_addr(int file, int addr) {
    char buffer[2];
    buffer[0] = SENSOR_CHANNEL1;
    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        perror("Failed to connect to the sensor");
        close(file);
        return -1;
    }
    if (write(file, buffer, 1) != 1) {
        perror("Failed to set channel");
        close(file);
        return -1;
    }
    if (read(file, buffer, 2) != 2) {
        perror("Failed to read data from the sensor");
        close(file);
        return -1;
    }
    return buffer[0];
}
