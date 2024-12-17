#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define UART_DEVICE "/dev/ttyAMA2"
#define BAUDRATE B115200
#define I2C_ADDR 0x48     // 0x48 module i2c
#define SENSOR_CHANNEL1 0x40 // A0값을 읽기 위해 필요한 cmd값.
#define SERVER_IP "192.168.137.3"
#define SERVER_PORT 8080
#define SLEEP_INTERVAL 1



int file, uart_fd;

int motor_speed;

// I2C에서 데이터 가져오는 함수
int get_data_from_I2C(int file, int addr) {
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

// UART 설정 함수
void set_uart(int set_fd) {
    struct termios options;
    tcgetattr(set_fd, &options);

    options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; // 보드레이트, 8비트 데이터, 로컬 모드, 수신 가능 설정
    options.c_iflag = IGNPAR;                          // 패리티 무시
    options.c_oflag = 0;                               // 출력 플래그 초기화
    options.c_lflag = 0;                               // 로컬 모드 플래그 초기화

    tcsetattr(set_fd, TCSANOW, &options);              // 설정 즉시 적용
}

// 압력센서 데이터 송신 (A72 -> R5)
void* thread_Send() {
    int file,sendD;
    char *filename = "/dev/i2c-1";
    
    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return NULL;
    }
    
    int uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY); // 읽기 및 쓰기 모드로 UART 장치를 엽니다.
    if (uart_fd == -1) {
        perror("Failed to open UART device");
        return NULL;
    }

    set_uart(uart_fd);
    
    while (1) {
    	int a = 0;
    	char send_data[4];  // 1바이트 문자 + 널 문자
        sendD = get_data_from_I2C(file,I2C_ADDR);
	usleep(200*1000);
        printf("T1.send: %d \n", sendD);
        snprintf(send_data, sizeof(send_data), "%d", sendD);
    	write(uart_fd, send_data, strlen(send_data) + 1);
    	tcflush(uart_fd, TCIFLUSH);     
    }
}

// 속도 수신 후 속도 업데이트 (R5->A72)
void* thread_receive_send() {
    char receive_data[10];
    int total_bytes_read;
    int bytes_read;
    int uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY); // 읽기 및 쓰기 모드로 UART 장치를 엽니다.
    if (uart_fd == -1) {
        perror("Failed to open UART device");
        return NULL;
    }
    set_uart(uart_fd);
    while (1) {
        total_bytes_read = 0;

        while (total_bytes_read < sizeof(receive_data) - 1) {
            bytes_read = read(uart_fd, &receive_data[total_bytes_read], sizeof(receive_data) - total_bytes_read - 1);
            if (bytes_read > 0) {
                total_bytes_read += bytes_read;
                if (receive_data[total_bytes_read - 1] == '\n' || receive_data[total_bytes_read - 1] == '\0') {
                    break;
                }
            } 
        }

        // 수신된 데이터를 널 문자로 종료
        receive_data[total_bytes_read] = '\0';
        if (total_bytes_read > 0) {
            printf("T2.receive: %s\n", receive_data); // 이거를 소켓 통신으로 qt에 보내면 된다.
        }

        //motor speed 전역변수 업데이트
        int m_value = atoi(receive_data);
        motor_speed = m_value;


	usleep(200*1000);
    }

    tcflush(uart_fd, TCIFLUSH);
    close(uart_fd);
    return 0;
}

//속도 송신(A72 -> A72(타보드))
void* thread_TCPIP(void* arg){

    int sock;
    struct sockaddr_in server_addr;
    ssize_t result;

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);

    // 데이터 전송 루프
    while (1) {
        // 서버로 motor_speed 값(int 형식) 전송
        result = send(sock, &motor_speed, sizeof(motor_speed), 0);
        if (result < 0) {
            perror("Failed to send data");
            break;
        }
        printf("Sent motor_speed: %d\n", motor_speed);

        // SLEEP_INTERVAL만큼 대기
        sleep(SLEEP_INTERVAL);
    }

    // 소켓 닫기
    close(sock);
    printf("Connection closed.\n");
}


int main() {
    char* filename = "/dev/i2c-1";

    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return -1;
    }

    uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY);
    if (uart_fd == -1) {
        perror("Failed to open UART device");
        return -1;
    }

    set_uart(uart_fd);

    // 스레드 생성
    pthread_t thread1, thread2, thread3;

    if (pthread_create(&thread1, NULL, thread_Send, NULL) != 0) {
        perror("Failed to create Task 1 thread");
        return -1;
    }

    if (pthread_create(&thread2, NULL, thread_receive_send, NULL) != 0) {
        perror("Failed to create Task 2 thread");
        return -1;
    }

    if (pthread_create(&thread3, NULL, thread_TCPIP, NULL) != 0) {
        perror("Failed to create Task 3 thread");
        return -1;
    }

    // 스레드 종료 대기
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    close(uart_fd);
    close(file);
    return 0;
}

