#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 30

int main(int argc, char **argv) {

    int recv_sockfd;
    struct sockaddr_in brd_addr;
    char recv_buf[BUF_SIZE];

    recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    memset(&brd_addr, 0, sizeof(brd_addr));
    brd_addr.sin_family = AF_INET;
    brd_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    brd_addr.sin_port = htons(atoi(argv[1]));

    if (bind(recv_sockfd, (struct sockaddr*)&brd_addr, sizeof(brd_addr)) == -1) {
        perror("bind error");
        return -1;
    }

    while(1) {
        ssize_t str_len = recvfrom(recv_sockfd, recv_buf, BUF_SIZE - 1, 0, NULL, 0);
        if (str_len < 0)
            break;
        
        recv_buf[str_len] = 0;
        fputs(recv_buf, stdout);
    }

    close(recv_sockfd);
    return 0;
}
