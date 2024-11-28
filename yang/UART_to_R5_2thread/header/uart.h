#ifndef UART_H
#define UART_H

#define UART_DEVICE "/dev/ttyAMA2"
#define BAUDRATE B115200

// UART 설정 함수
void set_uart(int set_fd);

// UART 데이터 전송 함수
void uart_send_data(int uart_fd, int data);

#endif