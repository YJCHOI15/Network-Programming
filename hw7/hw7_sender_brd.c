#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 30

int main(int argc, char **argv) {

    int send_sockfd;
    struct sockaddr_in brd_addr;
    FILE *fp;
    char send_buf[BUF_SIZE];

    send_sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    
    memset(&brd_addr, 0, sizeof(brd_addr));
    brd_addr.sin_family = AF_INET;
    // Local 브로드캐스트:
    // 255.255.255.255로 데이터를 전송하면, 
    // 전송한 호스트가 속한 네트워크로 데이터가 전송된다. 
    brd_addr.sin_addr.s_addr = inet_addr(argv[1]);
    brd_addr.sin_port = htons(atoi(argv[2]));

    int so_brd = 1;
    setsockopt(send_sockfd, SOL_SOCKET, SO_BROADCAST, 
                (void*)&so_brd, sizeof(so_brd));

    if ((fp = fopen("news.txt", "r")) == NULL) {
        perror("fopen error");
        return -1;
    }

    while(!feof(fp)) {
        fgets(send_buf, BUF_SIZE, fp);
        sendto(send_sockfd, send_buf, strlen(send_buf),
            0, (struct sockaddr*)&brd_addr, sizeof(brd_addr));
        sleep(2);
    }

    fclose(fp);
    close(send_sockfd);
    return 0;
}