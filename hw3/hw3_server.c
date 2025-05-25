#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t len;
    char buf[MAX];
    int operand[MAX];
    char operator[MAX];

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creating failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind error");
        return -1;
    }

    while(1) {
        len = sizeof(cli_addr);
        memset(buf, 0, sizeof(buf));
        recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&cli_addr, &len);

        char opCount = buf[0];
        if (opCount <= 0) {
            close(sockfd);
            printf("Server close(%d)\n", opCount);
            return -1;
        }

        for (int i = 0; i < opCount; i++) {
            memcpy(&operand[i], &buf[1 + i * 4], 4);
        }

        for (int i = 0; i < opCount - 1; i++) {
            operator[i] = buf[1+opCount*4+i];
        }

        int opResult = operand[0];
        for(int i=0; i<opCount-1; i++) {
            
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
        
        printf("Operand count: %d\n", buf[0]);
        for (int i = 0; i < opCount; i++) {
            printf("Operand %d: %d\n", i, operand[i]);
        }
        for (int i = 0; i < opCount - 1; i++) {
            printf("Operator %d: %c\n", i, operator[i]);
        }
        printf("Operation result: %d\n", opResult);

        memset(buf, 0, sizeof(buf));
        memcpy(buf, &opResult, 4);
        sendto(sockfd, buf, sizeof(buf), 0, (const struct sockaddr*)&cli_addr, len);
    }

    return 0;
}