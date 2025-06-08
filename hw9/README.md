# Raw Socket 기반 Ping 프로그램 (ICMP Echo)

본 프로젝트는 **Raw 소켓**과 **ICMP 프로토콜**을 사용하여 `ping` 명령어와 유사한 기능을 수행하는 C 프로그램입니다.  
ICMP Echo Request 패킷을 전송하고, 응답(Echo Reply)을 수신하여 네트워크 연결 상태를 진단합니다.

---

## 주요 기능

- ICMP Echo Request (type 8) 전송
- ICMP Echo Reply (type 0) 수신 및 출력
- 송신 및 수신을 `fork()`로 분리하여 병렬 처리
- 수신한 ICMP 패킷에서 IP 헤더, TTL, 출발지 주소 등 분석

---

## 프로그램 구성

### `struct packet`

- 실제로 전송할 ICMP 패킷 구조체 정의
- `struct icmphdr` + `msg`(사용자 메시지 영역)

```c
struct packet {
    struct icmphdr hdr;
    char msg[PACKETSIZE - sizeof(struct icmphdr)];
};
```

---

## 주요 함수 설명

### `main(argc, argv)`

- `argv[1]`: 대상 도메인 또는 IP (예: `8.8.8.8`)
- `gethostbyname()`로 대상 IP 해석
- `fork()`를 통해 두 개의 프로세스로 분기:
  - 자식 → `listener()` : 응답 수신 담당
  - 부모 → `ping()` : Echo Request 송신 담당

---

### `ping(struct sockaddr_in *addr)`

- Raw 소켓 생성 후 `ICMP_ECHO` 타입의 패킷을 전송
- `setsockopt()`로 TTL 설정 (`255`)
- 1초 간격으로 시퀀스 번호를 증가시키며 ICMP 패킷 전송

**패킷 구성 요소:**

| 필드 | 설명 |
|------|------|
| `hdr.type` | 8 (Echo Request) |
| `hdr.un.echo.id` | 프로세스 ID |
| `hdr.un.echo.sequence` | 전송 순번 증가 |
| `msg[]` | 임의의 텍스트 데이터 (`0` ~ `'9'`) |

- `checksum()` 함수를 통해 전체 패킷에 대한 체크섬 계산
- `sendto()`로 대상 IP에 전송

---

### `listener(void)`

- 수신 전용 Raw 소켓을 생성하여 `recvfrom()`으로 패킷 대기
- 수신한 응답 패킷을 `display()` 함수로 전달
- 무한 루프로 모든 수신 패킷을 처리

---

### `display(void *buf, int bytes)`

- 수신된 ICMP 응답을 파싱하여 다음 정보 출력:

```
IPv4: hdr-size=20 pkt-size=84 protocol=1 TTL=64 src=8.8.8.8 dst=192.168.0.2
ICMP: type[0/0] checksum[xxxx] id[xxxx] seq[xx]
```

- IP 헤더와 ICMP 헤더를 직접 파싱 (`iphdr`, `icmphdr`)
- `inet_ntoa()`로 IP 주소 문자열 출력

---

### `checksum(void *b, int len)`

- ICMP 프로토콜에서 요구하는 **1의 보수 합** 체크섬 계산
- 16비트 단위로 덧셈 + 상위 비트 누적 + 보수 처리

---

## 실행 권한 주의

- Raw 소켓(`SOCK_RAW`) 사용을 위해 **루트 권한**이 필요합니다.
- `ping()`과 `listener()` 모두 `socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)` 방식 사용

---

## 통신 흐름 요약

1. `ping()`에서 ICMP Echo Request 전송
2. `listener()`가 `recvfrom()`으로 Echo Reply 수신
3. 수신한 ICMP 패킷을 `display()`로 분석 후 출력
4. 1초마다 송신/수신 반복

---

## 사용된 주요 시스템 함수 및 구조체

- `socket()`, `sendto()`, `recvfrom()`, `setsockopt()`, `fcntl()`
- `fork()`, `wait()`
- `struct icmphdr`, `struct iphdr`, `struct sockaddr_in`, `struct in_addr`
- `inet_ntoa()`, `gethostbyname()`, `getprotobyname()`

---

## 참고

- 이 프로그램은 IPv4만 지원하며, ICMPv6는 지원하지 않습니다.
- `gethostbyname()`은 `getaddrinfo()`로 대체 가능하며, 이 코드는 전통적인 BSD 소켓 스타일입니다.
- `inet_ntop()`을 사용하면 IPv6 주소도 문자열로 출력할 수 있습니다.

## 빌드 및 실행결과

![Image](https://github.com/user-attachments/assets/dbc4d29c-f98a-4400-8066-e6c3332a7cce)
