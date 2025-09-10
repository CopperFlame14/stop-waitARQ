#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345
#define BUF_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[BUF_SIZE];
    socklen_t len;
    int expected_seq = 0;

    // Seed random for packet loss simulation
    srand(time(NULL));

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Receiver is ready and waiting...\n");

    while (1) {
        len = sizeof(cliaddr);
        int n = recvfrom(sockfd, (char *)buffer, BUF_SIZE, 0, ( struct sockaddr *) &cliaddr, &len);
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

        // Parse received message: format "seq|message"
        int seq_num;
        char msg[BUF_SIZE];
        sscanf(buffer, "%d|%[^\n]", &seq_num, msg);

        if (seq_num == expected_seq) {
            printf("Received packet with seq %d: %s\n", seq_num, msg);
            char ack_msg[10];
            sprintf(ack_msg, "ACK%d", seq_num);
            sendto(sockfd, ack_msg, strlen(ack_msg), 0, (const struct sockaddr *) &cliaddr, len);
            expected_seq = 1 - expected_seq;  // toggle seq number
        } else {
            printf("Duplicate packet with seq %d received.\n", seq_num);
            // Resend ACK for last received correct seq
            char ack_msg[10];
            sprintf(ack_msg, "ACK%d", 1 - expected_seq);
            sendto(sockfd, ack_msg, strlen(ack_msg), 0, (const struct sockaddr *) &cliaddr, len);
        }
    }

    close(sockfd);
    return 0;
}
