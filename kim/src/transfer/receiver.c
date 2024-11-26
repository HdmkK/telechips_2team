#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "dto.h"

#define PORT 8080
#define BUFFER_SIZE sizeof(struct sensor_dto)


void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    struct sensor_dto data;

    printf("Client connected, thread ID: %lu\n", pthread_self());

    // 데이터 지속 수신
    while (1) {
        ssize_t bytes_read = read(client_socket, &data, BUFFER_SIZE);
        if (bytes_read <= 0) {
            printf("Client disconnected or error occurred, thread ID: %lu\n", pthread_self());
            break;
        }

        printf("[Thread %lu] Received - Motor Speed: %d, Precipitation: %d, Tunnel Distance: %d, Air Condition: %d\n",
               pthread_self(), data.motor_speed, data.precipitation, data.tunnel_distance, data.air_condition);
    }

    close(client_socket);
    return NULL;
}



int main() {

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 주소 설정
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 소켓 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 연결 대기
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d\n", PORT);

    // 클라이언트 연결 처리
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_socket) != 0) {
            perror("Failed to create thread");
            free(client_socket);
            close(new_socket);
        }

        pthread_detach(thread_id); // 스레드가 종료되면 리소스 자동 정리
    }

    close(server_fd);
    return 0;
}
