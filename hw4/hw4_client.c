#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 1024

void printDomainInfo(const char *domain);
void printFileData(const char *ip, const char *port);

int main(int argc, char **argv) {

    if (argc == 2) {
        printDomainInfo(argv[1]);
    }
    else if (argc == 3) {
        printFileData(argv[1], argv[2]);
    }

    return 0;
}

void printDomainInfo(const char *domain) {
    struct hostent *host;
    struct sockaddr_in addr;

    /* gethostbyname() */
    if(!(host = gethostbyname(domain))) {
        herror("gethostbyname() error");
        exit(1);
    }

    printf("gethostbyname()\n");
    printf("Official name: %s\n", host->h_name);

    for (int i = 0; host->h_aliases[i]; i++) {
        printf("Aliases %d: %s\n", i, host->h_aliases[i]);
    }

    printf("Address type: %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");

    for (int i = 0; host->h_addr_list[i]; i++) {
        printf("IP addr %d: %s\n", i, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
    }

    /* gethostbyaddr() */
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = *(in_addr_t*)host->h_addr_list[0];
    
    if (!(host = gethostbyaddr((char*)&addr.sin_addr, sizeof(struct in_addr), AF_INET))) {
        herror("gethostbyaddr() error");
        exit(1);
    }

    printf("\ngethostbyaddr()\n");
    printf("Official name: %s\n", host->h_name);

    for (int i = 0; host->h_aliases[i]; i++) {
        printf("Aliases %d: %s\n", i, host->h_aliases[i]);
    }

    printf("Address type: %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");

    for (int i = 0; host->h_addr_list[i]; i++) {
        printf("IP addr %d: %s\n", i, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
    }

}

void printFileData(const char *ip, const char *port) {
    int cli_sockfd;
    char recv_buf[BUF_SIZE];
    char send_buf[BUF_SIZE];
    struct sockaddr_in serv_addr;
    int sock_type, read_cnt;
    FILE *fp;

    cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(atoi(port));

    socklen_t optlen = sizeof(sock_type);
    if (getsockopt(cli_sockfd, SOL_SOCKET, SO_TYPE, (void*)&sock_type, &optlen) == -1) {
        perror("getsockopt error");
        exit(1);
    }

    printf("This socket type is : %d(%s)\n", sock_type, (sock_type == SOCK_STREAM) ? "1" : "2");

    if (connect(cli_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect error");
        exit(1);
    }

    if((fp = fopen("copy.txt", "w")) == NULL) {
        perror("fopen error");
        exit(1);
    }

    memset(recv_buf, 0, sizeof(recv_buf));
    while((read_cnt = read(cli_sockfd, recv_buf, BUF_SIZE)) != 0) {
        fwrite((void*)recv_buf, 1, read_cnt, fp);
    }
    fclose(fp);
    printf("Received file data\n");

    if((fp = fopen("copy.txt", "r")) == NULL) {
        perror("fopen error");
        exit(1);
    }

    memset(send_buf, 0, sizeof(send_buf));
    while((read_cnt = fread(send_buf, 1, BUF_SIZE, fp)) != 0) {
        write(cli_sockfd, send_buf, read_cnt);
    }
    fclose(fp);
    close(cli_sockfd);
}