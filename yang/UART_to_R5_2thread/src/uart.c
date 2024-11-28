#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

// UART 설정 함수
void set_uart(int set_fd) {
    struct termios options;
    tcgetattr(set_fd, &options);
    options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcsetattr(set_fd, TCSANOW, &options);
}

// UART 데이터를 보내는 함수 (범위 변환 포함)
void uart_send_data(int uart_fd, int data) {
    // 데이터를 5개 범위로 나누기
    int divide_5;
    switch (data / 52) {
        case 0: divide_5 = 1; break;
        case 1: divide_5 = 2; break;
        case 2: divide_5 = 3; break;
        case 3: divide_5 = 4; break;
        case 4: divide_5 = 5; break;
        default: divide_5 = 0; break;
    }
    printf("ret48: %d, mapped: %d\n", data, divide_5);

    char response_data[2];
    snprintf(response_data, sizeof(response_data), "%d", divide_5);
    write(uart_fd, response_data, strlen(response_data) + 1);
    tcflush(uart_fd, TCIFLUSH);
}
