// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUF_SIZE 1024

int main() {
    printf("M R KRISHNI 24BCE1704");
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[BUF_SIZE];
    socklen_t len = sizeof(servaddr);
    int seq_num = 0;
    char *messages[] = {"Hello", "from", "Stop-and-Wait", "ARQ"};

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    struct timeval tv = {2, 0};  // 2 seconds timeout
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    for (int i = 0; i < 4; ) {
        snprintf(buffer, sizeof(buffer), "%d|%s", seq_num, messages[i]);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&servaddr, len);
        printf("Sent packet seq %d: %s\n", seq_num, messages[i]);

        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
        if (n < 0) {
            perror("Timeout or error, resending...");
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
    close(sockfd);
    return 0;
}
