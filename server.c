// server_windows.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUF_SIZE 1024

int main() {
    printf("M R KRISHNI 24BCE1704\n");

    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[BUF_SIZE];
    int expected_seq = 0;
    int cli_len = sizeof(cliaddr);

    srand((unsigned)time(NULL));

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    // Configure server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("Stop-and-Wait Receiver started on port %d...\n", PORT);

    while (1) {
        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr*)&cliaddr, &cli_len);
        if (n == SOCKET_ERROR) {
            printf("recvfrom error: %d\n", WSAGetLastError());
            continue;
        }

        buffer[n] = '\0';

        // Simulate 10% packet loss
        if ((rand() % 10) < 1) {
            printf("Packet lost (simulated).\n");
            continue;
        }

        int seq_num;
        char message[BUF_SIZE];
        sscanf(buffer, "%d|%s", &seq_num, message);

        if (seq_num == expected_seq) {
            printf("Received packet seq %d: %s\n", seq_num, message);

            char ack[20];
            snprintf(ack, sizeof(ack), "ACK%d", seq_num);
            sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr*)&cliaddr, cli_len);

            expected_seq = 1 - expected_seq; // toggle between 0 and 1
        } else {
            // Resend last ACK
            char ack[20];
            snprintf(ack, sizeof(ack), "ACK%d", 1 - expected_seq);
            sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr*)&cliaddr, cli_len);
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
