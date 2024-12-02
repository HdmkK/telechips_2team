#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

// Struct definition
struct vent_dto {
    int raining;
    int in_tunnel;
    int air_condition;
};

// Function to generate random data for the struct
struct vent_dto generate_vent_data() {
    struct vent_dto data;
    data.raining = rand() % 255;          // Random 0 or 1
    data.in_tunnel = rand() % 255;       // Random 0 or 1
    data.air_condition = rand() % 255; // Random 0 to 100
    return data;
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    struct vent_dto vent_data;
    const char *server_ip = "127.0.0.1"; // Replace with the server's IP address
    int server_port = 8081;             // Port number

    // Seed the random number generator
    srand(time(NULL));

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", server_ip, server_port);

    // Periodically send data
    while (1) {
        // Generate random data
        vent_data = generate_vent_data();

        // Send the struct data
        if (send(sock, &vent_data, sizeof(vent_data), 0) == -1) {
            perror("Failed to send data");
            break;
        }

        printf("Sent data: raining=%d, in_tunnel=%d, air_condition=%d\n",
               vent_data.raining, vent_data.in_tunnel, vent_data.air_condition);

        // Wait for 1 second before sending the next data
        sleep(1);
    }

    // Close the socket
    close(sock);
    return 0;
}
