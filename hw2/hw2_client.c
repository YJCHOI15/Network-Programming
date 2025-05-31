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

    int opCount, opResult;
    char buf[BUF_SIZE];

    cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(cli_sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect error");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    printf("Operand count: ");
    scanf("%d", &opCount);
    buf[0] = (char)opCount;

    if (buf[0] <= 0) {
        write(cli_sockfd, &buf[0], sizeof(char));
        close(cli_sockfd);
        return -1;
    }

    for (int i = 0; i < opCount; i++) {
        printf("Operand %d: ", i);
        scanf("%d", (int*)&buf[(i * 4) + 1]);
    }

    for (int i = 0; i < opCount - 1; i++) {
        printf("Operator %d: ", i);
        scanf(" %c", &buf[opCount * 4 + 1 + i]);
    }

    write(cli_sockfd, buf, 1 + 4 * opCount + (opCount - 1));

    read(cli_sockfd, &opResult, sizeof(int));
    printf("Operation result: %d\n", opResult);

    close(cli_sockfd);
    return 0;
}