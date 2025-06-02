#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#define BUF_SIZE 1024

void read_childproc(int sig);

int main(int argc, char **argv) {
    int serv_sockfd, cli_sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;

    pid_t pid;
    struct sigaction act;
    int state;

    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);  // 자식 프로세스 종료되면 read_childproc 호출

    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int enable = 1;
    setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    memset(&serv_addr, 0, sizeof(serv_addr));
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
            continue;               // 시그널(SIGCHLD)에 의해 accept()가 강제로 중단(GPT피셜)
        }
        
        printf("new client connected...\n");

        pid = fork();

        if (pid == 0) {
            close(serv_sockfd);      // 자식 프로세스에서 서버 소켓 필요 X
                                     // 클라이언트 소켓만 처리하면 됨

            char opCount;
            int opResult;
            int operand[BUF_SIZE];
            char operator[BUF_SIZE];

            read(cli_sockfd, &opCount, 1);
            if(opCount <= 0) {
                printf("Server close(%d)\n", opCount);
                close(cli_sockfd);
                return 0;
            }
            for(int i=0; i<opCount; i++) {
                read(cli_sockfd, &operand[i], 4);
            }
            for (int i = 0; i < opCount - 1; i++) {
                read(cli_sockfd, &operator[i], 1);
            }
            opResult = operand[0];
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
    
            write(cli_sockfd, &opResult, 4);

            printf("%d: %d", getpid(), operand[0]);
            for (int i = 0; i < opCount - 1; i++) {
                printf("%c%d", operator[i], operand[i+1]);
            }
            printf("=%d\n", opResult);

            close(cli_sockfd);
            return 0;                    // 자식 프로세스 종료
        }
        else {
            close(cli_sockfd);           // 부모 프로세스는 accept만 하기 때문에 
                                         // 클라이언트 소켓 필요 X
        }
    }

    close(serv_sockfd);
    return 0;
}

void read_childproc(int sig) {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);  // 현재 프로세스의 모든 자식 프로세스를 대상으로 함
    printf("removed proc id: %d \n", pid);
  }
