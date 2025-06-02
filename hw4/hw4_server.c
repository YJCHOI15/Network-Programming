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

    FILE *fp;
    char wbuf[BUF_SIZE];
    char rbuf[BUF_SIZE];
    int read_cnt;

    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int enable = 1;
    setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind error");
        return -1;
    }

    if (listen(serv_sockfd, 5) < 0) {
        perror("listen error");
        return -1;
    }

    if((fp = fopen("test.txt", "r")) == NULL) {
        perror("fopen error");
        return -1;
    }

    cli_len = sizeof(cli_addr);
    if((cli_sockfd = accept(serv_sockfd, (struct sockaddr*)&cli_addr, &cli_len)) < 0) {
        perror("accept error");
        return -1;
    }

    memset(wbuf, 0, sizeof(wbuf));
    while(fgets(wbuf, BUF_SIZE, fp) != NULL) {
        write(cli_sockfd, wbuf, strlen(wbuf));
    }

    shutdown(cli_sockfd, SHUT_WR);

    printf("Message from client:\n");
    memset(rbuf, 0, sizeof(rbuf));
    while((read_cnt = read(cli_sockfd, rbuf, BUF_SIZE)) > 0) {
        fwrite(rbuf, 1, read_cnt, stdout);
    }
    printf("\n");

    fclose(fp);
    close(cli_sockfd);
    close(serv_sockfd);

    return 0;
}