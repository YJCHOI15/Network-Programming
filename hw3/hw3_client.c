#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char **argv) {
    int cli_sockfd;
    struct sockaddr_in serv_addr;
    socklen_t serv_len;

    int opCount;
    char recv_buf[BUF_SIZE];
    char send_buf[BUF_SIZE];

    cli_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(cli_sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect error");
        return -1;
    }

    memset(send_buf, 0, sizeof(send_buf));

    printf("Operand count: ");
    scanf("%d", &opCount);
    send_buf[0] = (char)opCount;

    if (send_buf[0] <= 0) {
        write(cli_sockfd, &send_buf[0], sizeof(char));
        close(cli_sockfd);
        return -1;
    }

    for (int i = 0; i < opCount; i++) {
        printf("Operand %d: ", i);
        scanf("%d", (int*)&send_buf[(i * 4) + 1]);
    }

    for (int i = 0; i < opCount - 1; i++) {
        printf("Operator %d: ", i);
        scanf(" %c", &send_buf[opCount * 4 + 1 + i]);
    }

    serv_len = sizeof(serv_addr);
    sendto(cli_sockfd, send_buf, sizeof(send_buf), 0, (const struct sockaddr*)&serv_addr, serv_len);

    memset(recv_buf, 0, sizeof(recv_buf));
    recvfrom(cli_sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&serv_addr, &serv_len);
    printf("Operation result: %d\n", recv_buf[0]);

    close(cli_sockfd);
    return 0;
}