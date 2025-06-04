본 프로젝트는 **UDP 기반의 브로드캐스트(Broadcast)** 및 **멀티캐스트(Multicast)** 통신을 구현한 프로그램입니다.  
`news.txt` 파일 내용을 네트워크 상의 여러 수신자에게 전송하는 예제이며, 각각의 전송 방식을 별도의 실행 파일로 구성하였습니다.

---

## 통신 방식 요약

- **Broadcast**: 네트워크 내의 **모든 호스트**에게 UDP 메시지를 전송합니다.
- **Multicast**: **특정 멀티캐스트 그룹에 가입한 호스트**에게만 메시지를 전송합니다.

---

## Broadcast

- **전송자**: `hw7_sender_brd`
- **수신자**: `hw7_receiver_brd`
- **사용 IP 주소 예시**: `255.255.255.255` (Local 브로드캐스트)

![image](https://github.com/user-attachments/assets/8ced4edc-47dc-4a43-88cb-3529c8b1e703)

---

## Multicast

- **전송자**: `hw7_sender_mul`
- **수신자**: `hw7_receiver_mul`
- **사용 IP 주소 예시**: `224.0.0.1` ~ `239.255.255.255` (멀티캐스트 전용 대역)

![image](https://github.com/user-attachments/assets/cecc26ca-9856-4767-9145-f49e07bb449b)

---

## hw7_sender_brd.c

### 동작 과정

1. UDP 소켓 생성
2. `SO_BROADCAST` 옵션 설정으로 브로드캐스트 허용
3. `news.txt` 파일에서 한 줄씩 읽어서 전송
4. 수신자들은 동일 포트로 바인딩된 상태에서 메시지 수신

### 코드 주요 설정

```c
setsockopt(sock, SOL_SOCKET, SO_BROADCAST, ...);
sendto(..., inet_addr("255.255.255.255"), ...);
```

---

## hw7_receiver_brd.c

### 동작 과정

1. UDP 소켓 생성
2. `INADDR_ANY`로 모든 IP 주소에서 수신 대기
3. 해당 포트로 도착한 모든 메시지를 콘솔에 출력

### 코드 주요 설정

```c
bind(sock, INADDR_ANY, port);
recvfrom(...);
```

---

## hw7_sender_mul.c

### 동작 과정

1. UDP 소켓 생성
2. `IP_MULTICAST_TTL` 옵션으로 멀티캐스트 범위 제한 (예: 64)
3. `news.txt` 내용을 읽어 지정된 멀티캐스트 주소로 전송

### 코드 주요 설정

```c
setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, ...);
sendto(..., inet_addr("224.0.0.1"), ...);
```

---

## hw7_receiver_mul.c

### 동작 과정

1. UDP 소켓 생성 및 바인딩
2. `setsockopt`을 사용하여 **멀티캐스트 그룹 가입**
3. 해당 그룹의 데이터 수신 및 출력

### 코드 주요 설정

```c
struct ip_mreq mreq;
mreq.imr_multiaddr = inet_addr(argv[1]);
mreq.imr_interface = INADDR_ANY;
setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, ...);
```

---

## 참고 사항

- `news.txt` 파일이 동일 디렉토리에 있어야 전송이 작동합니다.
- 각 수신자는 해당 포트를 미리 바인딩하고 있어야 수신이 가능합니다.
- 멀티캐스트는 운영체제 및 네트워크 설정에 따라 라우팅 허용 여부가 달라질 수 있습니다.
- 브로드캐스트는 동일 서브넷 내에서만 동작합니다.

---

## 빌드 및 실행결과

### 브로드캐스트

![Image](https://github.com/user-attachments/assets/bf055c5e-7d19-449e-be36-68488e0c3945)

### 멀티캐스트

![Image](https://github.com/user-attachments/assets/672867cb-b50a-4c9e-a2a1-a45fb1638d7d)
