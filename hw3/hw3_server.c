#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char **argv) {
    int serv_sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;

    int opCount, opResult;
    int operand[BUF_SIZE];
    char operator[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    char send_buf[BUF_SIZE];

    if ((serv_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind failed");
        return -1;
    }

    while(1) {
        cli_len = sizeof(cli_addr);
        memset(recv_buf, 0, sizeof(recv_buf));
        recvfrom(serv_sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&cli_addr, &cli_len);

        opCount = recv_buf[0];
        if (opCount <= 0) {
            printf("Server close(%d)\n", opCount);
            break;
        }

        for (int i = 0; i < opCount; i++) {
            memcpy(&operand[i], &recv_buf[i*4 + 1], 4);
        }

        for (int i = 0; i < opCount - 1; i++) {
            operator[i] = recv_buf[opCount*4 + 1 + i];
        }

        opResult = operand[0];
        for (int i = 0; i < opCount - 1; i++) {
            
            switch (operator[i]) {
                case '+':
                    opResult += operand[i+1];
                    break;
                case '-':
                    opResult -= operand[i+1];
                    break;
                case '*':
                    opResult *= operand[i+1];
            }
        }
        
        printf("Operand count: %d\n", recv_buf[0]);

        for (int i = 0; i < opCount; i++) {
            printf("Operand %d: %d\n", i, operand[i]);
        }

        for (int i = 0; i < opCount - 1; i++) {
            printf("Operator %d: %c\n", i, operator[i]);
        }

        printf("Operation result: %d\n", opResult);

        memset(send_buf, 0, sizeof(send_buf));
        memcpy(send_buf, &opResult, sizeof(int));
        sendto(serv_sockfd, send_buf, sizeof(send_buf), 0, (const struct sockaddr*)&cli_addr, cli_len);
    }
    
    close(serv_sockfd);
    return 0;
}