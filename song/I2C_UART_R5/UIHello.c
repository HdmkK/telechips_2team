#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define UART_DEVICE "/dev/ttyAMA2"
#define BAUDRATE B115200
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



void set_uart(int set_fd) {
    struct termios options;
    // 현재 UART 설정 가져오기
    tcgetattr(set_fd, &options);

    // UART 옵션 설정
    options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; // 보드레이트, 8비트 데이터, 로컬 모드, 수신 가능 설정
    options.c_iflag = IGNPAR;                          // 패리티 무시
    options.c_oflag = 0;                               // 출력 플래그 초기화
    options.c_lflag = 0;                               // 로컬 모드 플래그 초기화

    // 설정 플러시 및 적용
    tcsetattr(set_fd, TCSANOW, &options);             // 설정 즉시 적용
}

int main() {
    int file,ret48;
    char *filename = "/dev/i2c-1";
    
    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return -1;
    }
    
    int uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY); // 읽기 및 쓰기 모드로 UART 장치를 엽니다.
    if (uart_fd == -1) {
        perror("Failed to open UART device");
        return -1;
    }

    set_uart(uart_fd);
    
    while(1){
    	ret48=0;
    	int a = 0;
        ret48 = get_data_from_addr(file,I2C_ADDR);
	usleep(100*1000);
    	// ret48 값을 5개 범위로 나눔
        switch (ret48 / 52) {
            case 0:
                a = 1; // 0~49
                break;
            case 1:
                a = 2; // 50~99
                break;
            case 2:
                a = 3; // 100~149
                break;
            case 3:
                a = 4; // 150~199
                break;
            case 4:
                a = 5; // 200~255
                break;
            default:
                a = 0; // 범위를 벗어난 경우
                break;
                }
        printf("ret48: %d \n", ret48);
        char response_data[2];  // 1바이트 문자 + 널 문자
        snprintf(response_data, sizeof(response_data), "%d", a);
    	write(uart_fd, response_data, strlen(response_data) + 1);
    	tcflush(uart_fd, TCIFLUSH);
    }
    close(uart_fd);
    close(file);
    return 0;
}


