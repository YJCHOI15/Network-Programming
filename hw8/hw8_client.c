#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <pthread.h>
	
#define ID_SIZE  10
#define BUF_SIZE 100

char id[ID_SIZE];

void* send_msg(void *csockfd);
void* recv_msg(void *csockfd);
	
// ./client IP PORT ID
int main(int argc, char **argv)
{
	int cli_sockfd;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
    void *thread_return;
	
    if (strlen(argv[3]) != 4) {
        printf("ID have to be 4\n");
        return -1;
    }

    memcpy(id, argv[3], 4);

	cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	  
    if (connect(cli_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect error");
        exit(1);
    }
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&cli_sockfd);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&cli_sockfd);

    pthread_detach(snd_thread);
    pthread_detach(rcv_thread);
    // pthread_join(snd_thread, &thread_return);
	// pthread_join(rcv_thread, &thread_return);

    // main 스레드가 종료되어 프로그램이 종료되는 것을 막기 위함.
    while(1) pause();

    // main 스레드 종료 전에 소켓을 닫아버리면 
    // recv와 send 스레드에서 소켓을 사용할 수 없기 때문에 
    // 두 스레드에서 소켓을 닫아야 한다. 
    // close(cli_sockfd);

	return 0;
}
	
void *send_msg(void *csockfd) {
	int cli_sockfd = *((int*)csockfd);
    char send_buf[BUF_SIZE];

	while (1) {
        char input[BUF_SIZE];
        fgets(input, BUF_SIZE, stdin);

        char *token = strtok(input, " \n");
        if (token == NULL) exit(1);

        int opCount = atoi(token);
        send_buf[0] = (char)opCount;
        if (send_buf[0] <= 0) {
            printf("Overflow Number(%d) - Closed client\n", send_buf[0]);
            close(cli_sockfd);
            exit(1);
        }

        int operands[opCount];
        char operators[opCount];

        for (int i = 0; i < opCount; i++) {
            token = strtok(NULL, " \n");
            operands[i] = atoi(token);
            memcpy(&send_buf[1 + i*4], &operands[i], 4);
        }

        for (int i = 0; i < opCount - 1; i++) {
            token = strtok(NULL, " \n");
            operators[i] = token[0];
            send_buf[1 + 4*opCount + i] = operators[i];
        }

        struct iovec cli_iov[2];
        cli_iov[0].iov_base = id;
        cli_iov[0].iov_len = ID_SIZE;
        cli_iov[1].iov_base = send_buf;
        cli_iov[1].iov_len = BUF_SIZE;

        writev(cli_sockfd, cli_iov, 2);
	}

	return NULL;
}
	
void *recv_msg(void * arg) {
	int cli_sockfd = *((int*)arg);
	char recv_buf[BUF_SIZE];
	int recv_len;

	while (1)
	{
		recv_len = read(cli_sockfd, recv_buf, sizeof(recv_buf));
		if (recv_len < 0) {
			perror("read error");
            close(cli_sockfd);
            exit(1);
		}

		recv_buf[recv_len] = '\0';
		fputs(recv_buf, stdout); 
	}

	return NULL;
}
