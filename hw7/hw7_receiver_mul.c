#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 30

int main(int argc, char **argv) {

    // argv[1]: 멀티캐스트 그룹 IP 주소
    // argv[2]: 포트 번호호

    int recv_sockfd;
    struct sockaddr_in mul_addr;
    struct ip_mreq join_addr;
    char recv_buf[BUF_SIZE];

    recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    memset(&mul_addr, 0, sizeof(mul_addr));
    mul_addr.sin_family = AF_INET;
    // 로컬의 모든 IP 인터페이스에서 멀티캐스트 수신 허용용
    mul_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    mul_addr.sin_port = htons(atoi(argv[2]));

    if (bind(recv_sockfd, (struct sockaddr*)&mul_addr, sizeof(mul_addr)) == -1) {
        perror("bind error");
        return -1;
    }

    join_addr.imr_multiaddr.s_addr = inet_addr(argv[1]); // argv[1]: 가입할 멀티캐스트 주소
    join_addr.imr_interface.s_addr = htonl(INADDR_ANY);  // 멀티캐스트 수신 인터페이스 등록

    // 특정 멀티캐스트 주소(argv[1])에 대해,
    // 로컬 인터페이스에서 수신을 허용하도록 요청
    setsockopt(recv_sockfd, IPPROTO_IP, 
            IP_ADD_MEMBERSHIP, (void*)&join_addr, sizeof(join_addr));

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
