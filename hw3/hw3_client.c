#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in serv_addr;
    socklen_t len;
    char buf[MAX];

    if(argc < 3) {
        printf("usage: ./client IP PORT");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    int opCount;
    printf("Operand count: ");
    scanf("%d", &opCount);

    buf[0] = (char)opCount;
    if (buf[0] <= 0) {
        sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        close(sockfd);
        return -1;
    }

    for (int i = 0; i < opCount; i++) {
        printf("Operand %d: ", i);
        scanf("%d", (int*)&buf[(i * 4) + 1]);
    }

    for (int i = 0; i < opCount - 1; i++) {
        printf("Operator %d: ", i);
        scanf(" %c", &buf[(opCount*4)+i+1]);
    }

    sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    memset(buf, 0, sizeof(buf));
    recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&serv_addr, &len);
    printf("Operation result: %d\n", buf[0]);

    close(sockfd);
    return 0;
}   