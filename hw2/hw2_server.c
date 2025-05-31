#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char **argv) {
    int serv_sockfd, cli_sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;

    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int enable = 1;
    setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind error");
        return -1;
    }

    if (listen(serv_sockfd, 5) < 0) {
        perror("listen error");
        return -1;
    }

    while(1) {
        cli_len = sizeof(cli_addr);
        cli_sockfd = accept(serv_sockfd, (struct sockaddr *)&cli_addr, &cli_len);

        if (cli_sockfd < 0) {
            perror("accept error");
            return -1;
        }

        int opCount, opResult;
        int operand[BUF_SIZE];
        char operator[BUF_SIZE];

        read(cli_sockfd, &opCount, sizeof(char));
        opCount = (char)opCount;

        if (opCount <= 0) {
            printf("Server close(%d)\n", opCount);
            close(cli_sockfd);
            close(serv_sockfd);

            return -1;
        }
        else
            printf("Operand Count: %d\n", opCount);

        read(cli_sockfd, operand, sizeof(int) * opCount);
        read(cli_sockfd, operator, sizeof(char) * (opCount - 1));

        opResult = operand[0];
        printf("Operand 0: %d\n", opResult);

        for (int i = 1; i < opCount; i++) {

            printf("Operand %d: %d\n", i, operand[i]);

            switch(operator[i-1]) {
                case '+':
                    opResult += operand[i];
                    break;
                case '-':
                    opResult -= operand[i];
                    break;
                case '*':
                    opResult *= operand[i];
                    break;
            }
        }

        printf("Operation result: %d\n", opResult);
        write(cli_sockfd, &opResult, sizeof(int));

        close(cli_sockfd);
    }

    close(serv_sockfd);
    return 0;
}