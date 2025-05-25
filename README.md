# 📡 네트워크 프로그래밍 과제 모음
이 레포지토리는 [네트워크 프로그래밍(Network Programming)] 수업에서 수행한 과제들을 정리한 공간입니다. TCP/IP 기반 통신, 소켓 프로그래밍, 멀티스레딩, 클라이언트-서버 모델 등의 개념을 실습하며 배운 내용을 코드로 구현하였습니다.

# 🛠 기술 스택
C

Python (선택적으로 사용한 경우)

POSIX 소켓 (Linux 환경)

TCP / UDP 프로토콜

멀티스레딩 (pthread or select, epoll 등)

# 📁 과제 목록
과제명	설명	주요 개념
01 TCP Client–Server 통신 프로그램 
02_udp_chat_client	UDP 기반 채팅 클라이언트/서버 구현	UDP, datagram
03_multi_client_server	다중 클라이언트 처리 서버 구현	select() 또는 멀티스레드
04_file_transfer	클라이언트-서버 간 파일 전송	파일 I/O, TCP stream
05_http_server	간단한 HTTP 서버 구현	HTTP 1.0, socket, MIME
06_ping_simulator	ping 명령어 유사 기능 구현	ICMP 구조 이해, 시간 측정

📂 각 과제 폴더에는 과제 설명 및 실행 방법이 포함되어 있습니다.


# 📚 학습 목표
소켓 생성 및 연결 과정 이해

TCP vs UDP 차이점 실습을 통한 체득

다중 클라이언트 처리 방법 학습 (select, thread)

네트워크 지연 및 패킷 손실 상황 대응

간단한 프로토콜 설계 및 구현
