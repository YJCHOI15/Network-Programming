#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>

#define MODE_SIZE 10
#define ID_SIZE   10
#define REQ_SIZE  1024

int main(int argc, char *argv[]) {
    int cli_sockfd;
	struct sockaddr_in serv_addr;

    char mode[MODE_SIZE] = {0};
    char id[ID_SIZE] = {0};
    char send_buf[REQ_SIZE] = {0};

	cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);   

    memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

    printf("Mode: ");
    scanf("%s", mode);
    if ( !( (strcmp(mode, "save") == 0) || (strcmp(mode, "load") == 0) || (strcmp(mode, "quit") == 0) )) {
        printf("supported mode: save load quit\n");
        return -1;
    }

    if(connect(cli_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
		perror("connect() error!");
        return -1;
    }
	else
		puts("Connected....");

    struct iovec iov[3];
    iov[0].iov_base = mode;
    iov[0].iov_len  = sizeof(mode);
    iov[1].iov_base = id;
    iov[1].iov_len  = sizeof(id);
    iov[2].iov_base = send_buf;
    iov[2].iov_len  = sizeof(send_buf);

    if (strncmp(mode, "save", 4) == 0) {
        int opCount;

        printf("ID: ");
        scanf("%s", id);
        if (strlen(id) != 4) {
            printf("Error: ID length must be 4\n");
            return -1;
        }

        printf("Operand count: ");
        scanf("%d", &opCount);
        send_buf[0] = (char)opCount;
        
        if(send_buf[0] <= 0) {
            printf("Overflow will happen(%d)\n", send_buf[0]);
            close(cli_sockfd);
            return 0;
        }
        
        for(int i=0; i<opCount; i++) {
            printf("Operand %d: ", i);
            scanf("%d", (int*)&send_buf[(i * 4) + 1]);
        }
    
        for (int i = 0; i < opCount - 1; i++) {
            printf("Operator %d: ", i);
            scanf(" %c", &send_buf[(opCount * 4) + i + 1]);
        }

        writev(cli_sockfd, iov, 3);

        int opResult;
        read(cli_sockfd, &opResult, sizeof(opResult));
        printf("Operation result: %d\n", opResult);

    }
    else if (strncmp(mode, "load", 4) == 0) {
        printf("ID: ");
        scanf("%s", id);
        if (strlen(id) != 4) {
            printf("Error: ID length must be 4\n");
            exit(0);
        }

        writev(cli_sockfd, iov, 2);

        // 서버로부터 받은 정보를 받고 출력
        char recv_buf[REQ_SIZE] = {0};
        read(cli_sockfd, recv_buf, sizeof(recv_buf));
        recv_buf[strlen(recv_buf)] = '\0';
        printf("%s\n", recv_buf);
    }
    else if (strncmp(mode, "quit", 4) == 0) {
        printf("quit\n");
        writev(cli_sockfd, iov, 1);
    }

    close(cli_sockfd);
    return 0;
}