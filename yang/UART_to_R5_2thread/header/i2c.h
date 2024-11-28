#ifndef I2C_H
#define I2C_H

#define I2C_ADDR 0x48
#define SENSOR_CHANNEL1 0x40 // A0 값을 읽기 위한 cmd 값

// I2C 데이터 읽기 함수
int get_data_from_addr(int file, int addr);

#endif