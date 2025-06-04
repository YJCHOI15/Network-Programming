#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/select.h>

#define SELECT_TIMEOUT 5

#define MODE_SIZE 10
#define ID_SIZE   10
#define REQ_SIZE  1024
#define DATA_MAX  20

typedef struct data {
    char id[ID_SIZE];
    char req[REQ_SIZE];
} data;

data dataArray[DATA_MAX];
int dataCount = 0;

int fdsA[2];        // 자식 read, 부모 write
int fdsB[2];        // 부모 read, 자식 write

void store_loop(void);
void multiplex_serv(char *port);

int main(int argc, char **argv) {

    pipe(fdsA);    
    pipe(fdsB);   

    pid_t pid = fork();
    if (pid == 0) {
        // 자식 프로세스:
        // save: 부모 프로세스로부터 받은 데이터 저장
        // load: 해당 id의 데이터를 부모 프로세스에게 전달
        // quit: 자식 프로세스(자신) 종료
        store_loop();
    }
    else {
        // 부모 프로세스:
        // save: 클라이언트로부터 받은 데이터를 처리하고 자식 프로세스에게 전달
        // load: 자식 프로세스로부터 해당 ID의 데이터를 받아와 클라이언트에게 전달
        // quit: 자식 프로세스에게 "quit" 전달하고 종료
        multiplex_serv(argv[1]);
    }

    return 0;
}

void store_loop(void) {
    while(1) {

        char mode[MODE_SIZE] = {0};
        char id[ID_SIZE] = {0};
        char recv_buf[REQ_SIZE] = {0};

        // 블로킹 I/O: 파이프로 부모 프로세스가 보내는 데이터 기다림
        read(fdsA[0], mode, sizeof(mode));

        // save: save id data
        if (strcmp(mode, "save") == 0) {
            read(fdsA[0], id, sizeof(id));
            read(fdsA[0], recv_buf, sizeof(recv_buf));

            // 버퍼 오버플로우 위험이 있을 경우 strncpy() 사용
            strcpy(dataArray[dataCount].id, id);
            strcpy(dataArray[dataCount].req, recv_buf);
            dataCount++;
        }

        // load: load id
        else if (strcmp(mode, "load") == 0) {
            read(fdsA[0], id, sizeof(id));

            char send_buf[REQ_SIZE] = {0};

            for (int i = 0; i < dataCount; i++) {
                if (strcmp(dataArray[i].id, id) == 0) {
                    char temp[ID_SIZE + REQ_SIZE + 3]; // 글자수 "%s: %s\n" 이상

                    snprintf(temp, sizeof(temp), "%s: %s\n", dataArray[i].id, dataArray[i].req);
                    strncat(send_buf, temp, sizeof(send_buf) - strlen(send_buf) - 1);
                }
            }

            if (strlen(send_buf) > 0) {
                write(fdsB[1], send_buf, strlen(send_buf));
            }
            else {
                const char *not_exist_msg = "Not exist";
                write(fdsB[1], not_exist_msg, strlen(not_exist_msg));
            }
        }

        // quit: quit
        else {
            break;
        }
    }
}

void multiplex_serv(char *port) {
    int serv_sockfd, cli_sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_addr_size;

    struct timeval timeout;
    fd_set reads, cpy_reads;
    int fd_max, fd_num;

    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int enable = 1;
    setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(port));

    if (bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind error");
        exit(1);
    }

    if (listen(serv_sockfd, 5) < 0) {
        perror("listen error");
        exit(1);
    }

    FD_ZERO(&reads);                 // 감시할 소켓 초기화
    FD_SET(serv_sockfd, &reads);     // select()로 감시할 소켓 등록
    fd_max = serv_sockfd;            // select()는 0부터 fd_max까지의 파일 디스크립터 감시

    timeout.tv_sec = SELECT_TIMEOUT;
    timeout.tv_usec = 0;

    while(1) {

        // select() fd_set을 0으로 만들기 때문에 복사본 필요 (서버 계속 감시해야해서)
        cpy_reads = reads;

        char mode[MODE_SIZE] = {0};
        char id[ID_SIZE] = {0};
        char recv_buf[REQ_SIZE] = {0};

        struct iovec pproc_iov[3];

        // serv_sockfd를 감시함으로써 새 클라이언트 연결 감지 (CPU 효율적)
        if((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
			break;
		
		if(fd_num == 0)
			continue;   // 타임 아웃, 이벤트 없음 -> 루프 계속
        
        for (int cur_fd = 0; cur_fd < fd_max + 1; cur_fd++) {
            // 읽기 가능한 fd 확인
            if (FD_ISSET(cur_fd, &cpy_reads)) {
                
                // 서버 소켓이면 새 클라이언트 연결
                if (cur_fd == serv_sockfd) {
                    // cli_addr_size만큼 cli_addr에 클라이언트 주소 저장
                    // 초기화 안하면 잘못된 주소가 저장될 수 있다. 
                    cli_addr_size = sizeof(cli_addr);
                    cli_sockfd = accept(serv_sockfd, (struct sockaddr*)&cli_addr, &cli_addr_size);
                    
                    FD_SET(cli_sockfd, &reads);  // 클라이언트 소켓을 감시 목록에 등록

                    if (fd_max < cli_sockfd)
                        fd_max = cli_sockfd;
                    
                    printf("connected client: %d\n", cli_sockfd);
                }

                // 클라이언트 소켓이면 데이터 수신
                else {

                    pproc_iov[0].iov_base = mode;
                    pproc_iov[0].iov_len = sizeof(mode);
                    pproc_iov[1].iov_base = id;
                    pproc_iov[1].iov_len = sizeof(id);
                    pproc_iov[2].iov_base = recv_buf;
                    pproc_iov[2].iov_len = sizeof(recv_buf);

                    ssize_t str_len = readv(cur_fd, pproc_iov, 3);

                    if (str_len == 0) {
                        // 클라이언트가 작업을 끝내고 종료된 경우
                        FD_CLR(cur_fd, &reads);
                        close(cur_fd);
                        printf("closed client: %d\n", cur_fd);
                    }
                    else if (str_len > 0) {

                        if (strncmp(mode, "save", 4) == 0) {
                            char send_buf[REQ_SIZE] = {0};

                            char opCount = recv_buf[0]; 
                            int operand[opCount];
                            char operator[opCount];

                            printf("Operand count: %d\n", opCount);

                            for(int i = 0; i < opCount; i++) {
                                operand[i] = recv_buf[i*4 + 1];
                                printf("Operand %d: %d\n", i, operand[i]);
                            }
                    
                            for (int i = 0; i < opCount - 1; i++) {
                                // recv_buf에서 opCount, operand 정보 다음으로 있는 operator 저장
                                operator[i] = recv_buf[opCount * 4 + i + 1];
                            }

                            snprintf(send_buf, sizeof(send_buf), "%d", operand[0]);
                            for (int i = 0; i < opCount - 1; i++) {
                                char temp[10];
                                snprintf(temp, sizeof(temp), "%c%d", operator[i], operand[i+1]);
                                strncat(send_buf, temp, sizeof(send_buf) - strlen(send_buf) - 1);
                            }

                            int result = operand[0];
                            for (int i = 0; i < opCount - 1; i++) {
                                switch (operator[i]) {
                                    case '+':
                                        result += operand[i + 1];
                                        break;
                                    case '-':
                                        result -= operand[i + 1];
                                        break;
                                    case '*':
                                        result *= operand[i + 1];
                                        break;
                                }
                            }

                            printf("Operation result: %d\n", result);
                            write(cur_fd, &result, sizeof(result));

                            char result_str[20];
                            snprintf(result_str, sizeof(result_str), "=%d", result);
                            strncat(send_buf, result_str, sizeof(send_buf) - strlen(send_buf) - 1);

                            pproc_iov[2].iov_base = send_buf;

                            printf("save to %s\n", id);
                            writev(fdsA[1], pproc_iov, 3);
                        }
                        else if (strncmp(mode, "load", 4) == 0) {
                            writev(fdsA[1], pproc_iov, 2);
                            
                            char recv_result[REQ_SIZE] = {0};
                            read(fdsB[0], recv_result, sizeof(recv_buf));

                            printf("load from %s\n", id);
                            write(cur_fd, recv_result, strlen(recv_result));
                        }
                        else if (strcmp(mode, "quit") == 0) {
                            writev(fdsA[1], pproc_iov, 1);
                            printf("quit\n");

                            FD_CLR(cur_fd, &reads);
                            close(cur_fd);
                            printf("closed client: %d\n", cur_fd);
                            
                            for (int i = 0; i < fd_max + 1; i++) {
                                if (FD_ISSET(i, &reads)) {
                                    close(i);   
                                }
                            }

                            exit(0);
                        }
                    }
                    else {
                        perror("readv from client error");
                        exit(1);
                    }
                }
            }
        }

    }
}
