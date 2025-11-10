// client_windows.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUF_SIZE 1024

int main() {
    printf("M R KRISHNI 24BCE1704\n");

    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in servaddr;
    char buffer[BUF_SIZE];
    int seq_num = 0;
    int serv_len = sizeof(servaddr);
    char *messages[] = {"Hello", "from", "Stop-and-Wait", "ARQ"};

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

    // Set 2 second timeout
    int timeout = 2000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    // Server setup
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);

    for (int i = 0; i < 4;) {
        snprintf(buffer, sizeof(buffer), "%d|%s", seq_num, messages[i]);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&servaddr, serv_len);
        printf("Sent packet seq %d: %s\n", seq_num, messages[i]);

        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
        if (n == SOCKET_ERROR) {
            printf("Timeout or error, resending...\n");
            continue;
        }
        buffer[n] = '\0';
        printf("Received %s\n", buffer);

        int ack_num;
        sscanf(buffer, "ACK%d", &ack_num);
        if (ack_num == seq_num) {
            seq_num = 1 - seq_num;
            i++;
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
