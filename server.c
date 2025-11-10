// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUF_SIZE 1024

int main() {
    printf("M R KRISHNI 24BCE1704");
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[BUF_SIZE];
    socklen_t len = sizeof(cliaddr);
    int expected_seq = 0;

    srand(time(NULL));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(1);
    }
    printf("Stop-and-Wait Receiver started...\n");
    while (1) {
        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("recvfrom error");
            continue;
        }
        buffer[n] = '\0';

        // Simulate 10% packet loss
        if ((rand() % 10) < 1) {
            printf("Packet lost (simulated).\n");
            continue;
        }
        int seq_num;
        sscanf(buffer, "%d|%s", &seq_num, buffer);

        if (seq_num == expected_seq) {
            printf("Received packet seq %d: %s\n", seq_num, buffer);
            char ack[20];
            snprintf(ack, sizeof(ack), "ACK%d", seq_num);
            sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&cliaddr, len);
            expected_seq = 1 - expected_seq; // toggle between 0 and 1
        } else {
            // Resend last ACK
            char ack[20];
            snprintf(ack, sizeof(ack), "ACK%d", 1 - expected_seq);
            sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&cliaddr, len);
        }
    }
    close(sockfd);
    return 0;
}
