#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[BUF_SIZE];
    socklen_t len = sizeof(servaddr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket timeout for recvfrom
    struct timeval tv;
    tv.tv_sec = 2;  // 2 seconds timeout
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        perror("Error setting socket timeout");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    char *messages[] = {"Hello", "How", "Are", "You", "StopAndWaitARQ"};
    int msg_count = sizeof(messages) / sizeof(messages[0]);
    int seq_num = 0;

    for (int i = 0; i < msg_count; i++) {
        int ack_received = 0;

        while (!ack_received) {
            // Create packet "seq|message"
            snprintf(buffer, BUF_SIZE, "%d|%s", seq_num, messages[i]);
            sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            printf("Sent packet seq %d: %s\n", seq_num, messages[i]);

            // Wait for ACK
            int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
            if (n < 0) {
                perror("Timeout or recvfrom error. Resending...");
                continue;
            }
            buffer[n] = '\0';

            if (strcmp(buffer, "") == 0) {
                printf("No ACK received. Resending...\n");
                continue;
            }

            char expected_ack[10];
            snprintf(expected_ack, sizeof(expected_ack), "ACK%d", seq_num);

            if (strcmp(buffer, expected_ack) == 0) {
                printf("Received %s\n", buffer);
                ack_received = 1;
                seq_num = 1 - seq_num;  // toggle seq num
            } else {
                printf("Wrong ACK %s received. Resending...\n", buffer);
            }
        }
    }

    close(sockfd);
    return 0;
}
