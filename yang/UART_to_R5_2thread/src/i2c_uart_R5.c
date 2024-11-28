#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include "uart.h"
#include "i2c.h"

pthread_mutex_t lock; // 데이터 동기화를 위한 mutex
int shared_data = 0;  // I2C 데이터를 저장할 공유 변수

// I2C 데이터를 읽는 쓰레드
void* i2c_thread(void* arg) {
    int file = *(int*)arg;
    while (1) {
        int data = get_data_from_addr(file, I2C_ADDR);
        pthread_mutex_lock(&lock); // 데이터 접근 보호
        shared_data = data;
        pthread_mutex_unlock(&lock);
        usleep(100 * 1000); // 100ms 대기
    }
    return NULL;
}

// UART 데이터를 보내는 쓰레드
void* uart_thread(void* arg) {
    int uart_fd = *(int*)arg;
    while (1) {
        int data;
        pthread_mutex_lock(&lock); // 데이터 접근 보호
        data = shared_data;
        pthread_mutex_unlock(&lock);
        uart_send_data(uart_fd, data);
        usleep(100 * 1000); // 100ms 대기
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    char* filename = "/dev/i2c-1";
    int file;
    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return -1;
    }

    int uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY);
    if (uart_fd == -1) {
        perror("Failed to open UART device");
        return -1;
    }
    set_uart(uart_fd);

    pthread_mutex_init(&lock, NULL); // Mutex 초기화

    // I2C와 UART 쓰레드 생성
    pthread_create(&thread1, NULL, i2c_thread, &file);
    pthread_create(&thread2, NULL, uart_thread, &uart_fd);

    // 쓰레드가 종료될 때까지 대기
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&lock); // Mutex 해제
    close(uart_fd);
    close(file);
    return 0;
}
