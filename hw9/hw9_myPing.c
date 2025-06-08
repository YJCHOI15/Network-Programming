#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#define PACKETSIZE 64

// ICMP 패킷을 구성할 구조체 정의 (헤더 + 메시지)
struct packet {
    struct icmphdr hdr;
    char msg[PACKETSIZE - sizeof(struct icmphdr)];
};

int pid = -1;
struct protoent *proto = NULL; // ICMP 프로토콜 정보 저장

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void display(void *buf, int bytes) {
    struct iphdr *ip = buf;
    struct icmphdr *icmp = buf + ip->ihl*4;  // IP 헤더 길이만큼 건너 뜀
    struct in_addr addr;

    printf("----------------\n");

    addr.s_addr = ip->saddr;  // 출발지

    printf("IPv%d: hdr-size=%d pkt-size=%d protocol=%d TTL=%d src=%s ",
        ip->version, ip->ihl*4, ntohs(ip->tot_len), ip->protocol,
        ip->ttl, inet_ntoa(addr));
    
    addr.s_addr = ip->saddr;  // 목적지

    printf("dst=%s\n", inet_ntoa(addr));
    if (icmp->un.echo.id == pid) {
        printf("ICMP: type[%d/%d] checksum[%d] id[%d] seq[%d]\n\n",
            icmp->type, icmp->code, ntohs(icmp->checksum),
            icmp->un.echo.id, icmp->un.echo.sequence);
    }
    
}

void listener(void) {
    const int val = 255;
    struct sockaddr_in addr;
    unsigned char buf[512];

    int sd = socket(AF_INET, SOCK_RAW, proto->p_proto);  // 세 번째 인자에서 Raw 소켓에서 사용할 프로토콜 지정

    if (sd < 0) {
        perror("socket creation error");
        exit(1);
    }

    while(1) {
        int bytes, len = sizeof(addr);

        bzero(buf, sizeof(buf));
        bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
         
        if (bytes > 0) {
            printf("***Got message!***\n");
            display(buf, bytes);
        }
        else {
            perror("recvfrom error");
        }
    }
    exit(1);
}

void ping(struct sockaddr_in *addr) {
    const int val = 255;
    int cnt = 1;
    struct packet pckt;
    struct sockaddr_in r_addr;
    int bytes;

    int sd = socket(AF_INET, SOCK_RAW, proto->p_proto);
    if (sd < 0) {
        perror("socket creation error");
        return;
    }

    // 패킷의 TTL 설정
    if (setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
        perror("set TTL option error");

    // Non-Blocking 소켓 설정
    if (fcntl(sd, F_SETFL, O_NONBLOCK) != 0) 
        perror("request nonblocking I/O");

    while(1) {
        int len = sizeof(r_addr);

        printf("\nMsg #%d\n", cnt);
        bzero(&pckt, sizeof(pckt));
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = pid;

        for (int i = 0; i < sizeof(pckt.msg) - 1; i++) {
            pckt.msg[i] = i + '0';
        }

        pckt.msg[sizeof(pckt.msg) - 1] = 0;
        pckt.hdr.un.echo.sequence = cnt++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

        if (sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0)
            perror("sendto error");
        
        sleep(1);
    }
}

int main(int argc, char **argv) {

    struct hostent *hname;
    struct sockaddr_in addr;

    pid = getpid();
    proto = getprotobyname("ICMP");
    hname = gethostbyname(argv[1]);
    bzero(&addr, sizeof(addr));

    addr.sin_family = hname->h_addrtype;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = *(long*)hname->h_addr;

    if (fork() == 0) 
        listener();
    else    
        ping(&addr);
    
    wait(0);

    return 0;
}
