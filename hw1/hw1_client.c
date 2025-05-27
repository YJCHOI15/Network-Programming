#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    int cli_sockfd;
    char buf[1024];
    struct sockaddr_in serv_addr;

    cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    connect(cli_sockfd, (const struct sockaddr*)&serv_addr, sizeof(serv_addr));

    memset(buf, 0, sizeof(buf));
    scanf("%s", buf);
    write(cli_sockfd, buf, strlen(buf));

    memset(buf, 0, sizeof(buf));
    read(cli_sockfd, buf, sizeof(buf));
    printf("%s\n", buf);

    close(cli_sockfd);
    return 0;
}