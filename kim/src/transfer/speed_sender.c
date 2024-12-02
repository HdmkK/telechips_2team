#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"  // 서버 IP 주소
#define SERVER_PORT 8080       // 서버 포트 번호
#define INTERVAL 1             // 전송 간격 (초)

int main() {
    int sock;
    struct sockaddr_in server_addr;
    int motor_speed = 0;

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // 서버 IP 주소 변환 및 설정
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);

    // motor_speed 주기적 전송
    while (1) {
        // motor_speed 값을 0~255 범위에서 순환적으로 증가
        motor_speed = (motor_speed + 1) % 256;

        // motor_speed 전송
        if (send(sock, &motor_speed, sizeof(motor_speed), 0) < 0) {
            perror("Send failed");
            break;
        }

        printf("Sent motor_speed: %d\n", motor_speed);

        // 전송 간격
        sleep(INTERVAL);
    }

    // 소켓 닫기
    close(sock);
    printf("Connection closed\n");

    return 0;
}
