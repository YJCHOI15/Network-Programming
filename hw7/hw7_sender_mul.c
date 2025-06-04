#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define TTL      64
#define BUF_SIZE 30

int main(int argc, char **argv) {

    int send_sockfd;
    struct sockaddr_in mul_addr;
    FILE *fp;
    char send_buf[BUF_SIZE];

    // 멀티캐스트의 생존시간(Time To Live) 설정을 위한 변수
    int time_live = TTL; 

    send_sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP 소켓
    
    memset(&mul_addr, 0, sizeof(mul_addr));
    mul_addr.sin_family = AF_INET;
    mul_addr.sin_addr.s_addr = inet_addr(argv[1]);
    mul_addr.sin_port = htons(atoi(argv[2]));

    // 멀티캐스트 TTL 설정
    setsockopt(send_sockfd, IPPROTO_IP, IP_MULTICAST_TTL, 
                (void*)&time_live, sizeof(time_live));

    if ((fp = fopen("news.txt", "r")) == NULL) {
        perror("fopen error");
        return -1;
    }

    while(!feof(fp)) {
        fgets(send_buf, BUF_SIZE, fp);
        sendto(send_sockfd, send_buf, strlen(send_buf),
            0, (struct sockaddr*)&mul_addr, sizeof(mul_addr));
        sleep(2);
    }

    fclose(fp);
    close(send_sockfd);
    return 0;
}