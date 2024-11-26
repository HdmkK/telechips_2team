#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "dto.h"

#define SERVER_IP "127.0.0.1"
#define PORT 8080

void *send_data(void *arg) {
    int sock = *(int *)arg;
    struct sensor_dto data;

    while (1) {
        // 데이터를 생성 (임의값)
        data.motor_speed = rand() % 100;
        data.precipitation = rand() % 100;
        data.tunnel_distance = rand() % 500;
        data.air_condition = rand() % 10;

        // 데이터 전송
        ssize_t bytes_sent = send(sock, &data, sizeof(data), 0);
        if (bytes_sent <= 0) {
            printf("Connection closed or error occurred\n");
            break;
        }

        printf("Sent - Motor Speed: %d, Precipitation: %d, Tunnel Distance: %d, Air Condition: %d\n",
               data.motor_speed, data.precipitation, data.tunnel_distance, data.air_condition);

        sleep(1); // 1초 대기
    }

    return NULL;
}



int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // 서버 주소 설정
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // IP 주소 변환 및 설정
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server\n");

    // 데이터 전송을 위한 스레드 생성
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, send_data, &sock) != 0) {
        perror("Failed to create thread");
        close(sock);
        exit(EXIT_FAILURE);
    }

    pthread_join(thread_id, NULL); // 스레드가 종료될 때까지 대기

    close(sock);
    return 0;
}
