#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/uio.h>

#define MAX_CLI  50
#define ID_SIZE  10
#define BUF_SIZE 100

int cli_cnt = 0;
int csockfd_arr[MAX_CLI];
pthread_mutex_t mutex;

void* handle_cli(void* csockfd);

int main(int argc, char **argv) {

    int serv_sockfd, cli_sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    int cli_addr_size;

    pthread_t t_id;

    pthread_mutex_init(&mutex, NULL);
    
    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
        cli_addr_size = sizeof(cli_addr);
        cli_sockfd = accept(serv_sockfd, (struct sockaddr*)&cli_addr, &cli_addr_size);

        pthread_mutex_lock(&mutex);
        csockfd_arr[cli_cnt++] = cli_sockfd;
        pthread_mutex_unlock(&mutex);

        // 스레드 함수의 인자 타입이 void*로 고정 -> void*로 넘김
        pthread_create(&t_id, NULL, handle_cli, (void*)&cli_sockfd);
        pthread_detach(t_id);
        printf("Connected client port: %d \n", ntohs(cli_addr.sin_port));
    }

    return 0;
}

void* handle_cli(void* csockfd) {
    // 스스로 detach를 호출해 워커 스레드가 종료될때
    // 자동으로 자원 정리가 되도록 함. 
    // 이미 메인 스레드에서 detach를 했으므로 중복해서 할 필요 X
    // pthread_detach(pthread_self());

    int cli_sockfd = *((int*)csockfd);
    char id[ID_SIZE];
    char recv_buf[BUF_SIZE];
    char send_buf[BUF_SIZE];
    
    struct iovec serv_iov[2];
    serv_iov[0].iov_base = id;
    serv_iov[0].iov_len  = ID_SIZE;
    serv_iov[1].iov_base = recv_buf;
    serv_iov[1].iov_len  = BUF_SIZE;

    while(1) {
        memset(id, 0, sizeof(id));
        memset(recv_buf, 0, sizeof(recv_buf));

        int rlen = readv(cli_sockfd, serv_iov, 2);
        if (rlen == 0) {
            printf("Closed client\n");
            break;
        }

        int opCount = (int)recv_buf[0];
        int operand[opCount];
        char operator[opCount];
        int idx = 1;

        for (int i = 0; i < opCount; i++) {
            if (idx + 4 > BUF_SIZE) break;
            memcpy(&operand[i], &recv_buf[idx], 4);
            idx += 4;
        }

        for (int i = 0; i < opCount - 1; i++) {
            if (idx >= BUF_SIZE) break;
            operator[i] = recv_buf[idx++];
        }

        char expr_str[20] = {0};
        char num_str[16];

        snprintf(expr_str, sizeof(expr_str), "%d", operand[0]);
        int opResult = operand[0];

        for (int i = 0; i < opCount - 1; i++) {
            snprintf(num_str, sizeof(num_str), "%c%d", operator[i], operand[i + 1]);
            strncat(expr_str, num_str, sizeof(expr_str) - strlen(expr_str) - 1);

            switch (operator[i]) {
                case '+': opResult += operand[i + 1]; break;
                case '-': opResult -= operand[i + 1]; break;
                case '*': opResult *= operand[i + 1]; break;
                default: break;
            }
        }

        snprintf(send_buf, sizeof(send_buf), "[%s] %s=%d\n", id, expr_str, opResult);
        
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < cli_cnt; i++) {
            write(csockfd_arr[i], send_buf, strlen(send_buf));
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < cli_cnt; i++) {
        if (cli_sockfd == csockfd_arr[i]) {
            while (i < cli_cnt - 1) {
                csockfd_arr[i] = csockfd_arr[i + 1];
                i++;
            }
            break;
        }
    }
    cli_cnt--;
    pthread_mutex_unlock(&mutex);

    close(cli_sockfd);
    return NULL;
}