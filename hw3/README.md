본 프로젝트는 UDP 소켓 프로그래밍을 통해 간단한 서버-클라이언트 통신을 구현한 과제입니다.  
클라이언트가 피연산자와 연산자 정보를 서버에 전송하면, 서버는 이를 계산하여 결과를 클라이언트에게 전송합니다.

![Image](https://github.com/user-attachments/assets/fec6c162-1962-4386-ae6a-c72480478f6a)

---

## 서버 (hw3_server.c)

1. 실행 시 main 함수 매개변수로 포트번호를 입력받아 실행합니다.  

    예: `./server 9190`

2. INADDR_ANY와 입력받은 포트번호를 사용해 소켓을 바인드(bind)합니다.

3. 클라이언트로부터 recvfrom()을 통해 연산 요청 데이터를 수신합니다.

4. 서버는 Iterative UDP Server 형태로 구현되며 종료 조건은 다음과 같습니다:
    - 클라이언트가 전송한 operand count 값이 0보다 작거나 같은 경우
    - 이 경우 `server close(operand count)`를 출력한 뒤 소켓을 닫고 종료합니다.

5. 서버는 수신한 데이터를 기반으로 다음과 같이 연산을 수행합니다:
    - operand count만큼 피연산자를 받고, operand count - 1만큼의 연산자를 받습니다.
    - 피연산자와 연산자는 앞에서부터 순차적으로 계산되며, `*` 연산자에 대한 우선순위는 적용하지 않습니다.  
      예: `[3 | 4, 5, 7 | +, *]` → `((4 + 5) * 7)`

6. 계산된 결과를 클라이언트에게 sendto()로 전송합니다. (connected UDP 사용하지 않음)
    - 전송 전 `Operation result: <결과>` 형태로 표준 출력합니다.
    - 프로그램 종료 조건 외에는 계속해서 클라이언트 요청을 반복 수신/응답합니다.

---

## 클라이언트 (hw3_client.c)

1. 실행 시 main 함수 매개변수로 서버 IP 주소와 포트번호를 입력받아 실행합니다.  

    예: `./client 127.0.0.1 9190`

2. 입력받은 서버 IP와 포트번호를 기반으로 UDP 소켓을 생성하여 서비스 요청을 합니다.

3. 사용자로부터 다음의 표준입력을 받습니다:
    - operand count 입력 시: `Operand count:` 출력
    - operand 입력 시: `Operand 0:`, `Operand 1:` … 형식으로 출력하며 입력
    - operator 입력 시: `Operator 0:`, `Operator 1:` … 형식으로 출력하며 입력
    - 연산자는 `+`, `-`, `*`로 한정됩니다.

4. 입력된 데이터는 다음과 같은 형태로 char 배열에 저장되어 **한 번의 sendto()로 전송**됩니다:
    ```
    [1바이트 operand count] [4바이트 * operand] [1바이트 * operator]
    ```
    예: `(3 | 4, 5, 7 | +, -)` → char 배열로 패킹하여 전송

5. 서버로부터 결과를 수신하면, `Operation result: <결과>` 형식으로 출력하고 소켓을 닫고 종료합니다.

6. operand count가 0 이하인 경우, 해당 값만 전송 후 소켓을 닫고 추가 입력 없이 종료됩니다.

---

## 빌드 및 실행결과

![Image](https://github.com/user-attachments/assets/08549da5-cbfa-4bd4-a4fc-f423f342ebf0)
