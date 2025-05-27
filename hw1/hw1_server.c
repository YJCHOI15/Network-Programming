#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    int serv_sockfd, cli_sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    char buf[1024];
    socklen_t cli_len;

    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int enable = 1;
    setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    listen(serv_sockfd, 5);

    cli_sockfd = accept(serv_sockfd, (struct sockaddr *)&cli_addr, &cli_len);

    memset(buf, 0, sizeof(buf));
    read(cli_sockfd, buf, sizeof(buf));
    printf("%s\n", buf);

    strcat(buf, "_최용진");
    write(cli_sockfd, buf, strlen(buf));

    close(cli_sockfd);
    close(serv_sockfd);

    return 0;
}