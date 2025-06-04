본 프로젝트는 **TCP 소켓 프로그래밍**을 기반으로 한 클라이언트-서버 구조의 프로그램입니다.  
멀티프로세스를 활용하여 계산 수행, 클라이언트 요청 처리, 결과 저장 및 조회 기능을 제공합니다.

클라이언트는 세 가지 모드 중 하나를 선택하여 서버에 요청을 보냅니다:

- **save**: 사용자가 입력한 수식을 서버에서 계산하고 해당 결과를 ID와 함께 저장합니다.

  ![Image](https://github.com/user-attachments/assets/e9b18042-3d22-428f-8d9f-8af2ea867826)
  
- **load**: 지정한 ID로 저장된 모든 수식 결과를 요청하여 클라이언트로 가져옵니다.

  ![Image](https://github.com/user-attachments/assets/5fdc5634-b74d-4e97-9480-ccc2fc4635b9)
  
- **quit**: 서버에게 종료 신호를 전송하며, 서버와 클라이언트 모두 종료됩니다.

  ![Image](https://github.com/user-attachments/assets/10debd6d-afb3-4078-97f7-d42370e2f97a)

---

## hw6_client.c

### 동작 과정

1. **Mode 입력 요청**
   - 실행 시 `Mode:`를 출력하고 사용자로부터 모드를 입력받습니다.
   - 지원하는 모드는 `save`, `load`, `quit`입니다.
   - 그 외 입력 시 다음과 같이 출력 후 종료됩니다:

     ```
     Mode: aaaa
     supported mode: save load quit
     ```

2. **ID 입력**
   - `save` 또는 `load` 모드일 경우 `ID:`를 출력하고 입력을 받습니다.
   - 입력된 ID 길이가 4가 아니면 다음과 같이 출력 후 종료됩니다:

     ```
     ID: aaaaa
     error: ID length must be 4
     ```

3. **Save 모드**
   
   - `iovec` 배열 구성:
     - `iov[0]`: mode
     - `iov[1]`: ID
     - `iov[2]`: 계산 정보 (기존 과제 형식과 동일)
   - 연산자 수가 0 미만이면 다음과 같이 출력 후 종료됩니다:

     ```
     Overflow will happen(<operand_count>)
     ```

   - 서버로부터 수신한 결과는 다음 형식으로 출력됩니다:

     ```
     Operation result: <계산결과>
     ```

5. **Load 모드**
   
   - `iovec` 배열 구성:
     - `iov[0]`: mode
     - `iov[1]`: ID
     - `iov[2]`: 사용하지 않음
   - 서버로부터 ID에 해당하는 수식 결과 문자열 또는 `Not exist`를 수신하여 출력 후 종료됩니다.

7. **Quit 모드**
   
   - `iovec` 배열 구성:
     - `iov[0]`: mode
     - `iov[1]`, `iov[2]`: 사용하지 않음
   - 서버에게 종료 신호를 전송하고 클라이언트 종료

---

## 🖥️ hw6_server.c

### 실행 방법

`hw6_server <PORT>` 형식으로 실행합니다.

### 동작 과정

1. **서버 초기화**
   - 지정 포트로 TCP 서버 생성
   - `pipe()` 2개 생성 후 `fork()` 호출
     - **부모 프로세스**: 클라이언트 처리
     - **자식 프로세스**: 수식 저장/조회 처리

2. **Multiplexing 처리**
   - 부모 프로세스는 `select()`를 사용하여 클라이언트 I/O 처리 (timeout 5초)
   - 클라이언트 연결 시:

     ```
     connected client : <file descriptor>
     ```

   - 클라이언트 종료 시:

     ```
     closed client : <file descriptor>
     ```

3. **클라이언트 요청 처리**
   - `readv()`로 `mode`, `ID`, 계산 데이터 수신

   #### save 모드
   - 출력:
     ```
     save to <ID>
     ```
   - 계산 후 수식=`결과` 문자열을 생성하고 파이프로 자식에게 전달

   #### load 모드
   - 출력:
     ```
     load from <ID>
     ```
   - 자식에게 요청 전달 → ID에 해당하는 수식들 또는 `Not exist` 문자열을 받아 클라이언트로 전송

   #### quit 모드
   - 출력:
     ```
     quit
     ```
   - 자식에게 종료 명령 전송 → 자식 종료
   - 부모도 이후 클라이언트 요청을 종료함

---

## 통신 흐름 요약

1. 클라이언트 실행 → 서버 연결 요청
2. 클라이언트가 `mode`, `ID`, (필요시) 계산 데이터 전송
3. 서버는 모드에 따라 처리:
   - 계산 후 결과 저장 (save)
   - 저장된 결과 조회 (load)
   - 종료 처리 (quit)
4. 클라이언트로 결과 전송 후 연결 종료

---

## 빌드 및 실행결과

### 실행결과1

![Image](https://github.com/user-attachments/assets/430ef9a5-3147-4c7d-bad7-9618f40320d9)

### 실행결과2

![Image](https://github.com/user-attachments/assets/8d64fd29-9425-4019-a923-65a46ddcb2fd)

### 처리사항1: mode가 의도한 문자 외의 값을 받았을 경우

![Image](https://github.com/user-attachments/assets/4379ca4a-3f63-4020-9265-45931ee50808)

### 처리사항2: ID의 길이가 4bytes가 아닐 시 아래와 같이 처리

![Image](https://github.com/user-attachments/assets/19c7e708-2039-4f0a-bb24-d707d41bb243)

### 처리사항3: Load 했을 때 해당 ID로 기록된 데이터가 없다면 Not exist를 전송

![Image](https://github.com/user-attachments/assets/49ce576e-3396-4f04-9776-6efd45f9ca11)

### 처리사항4: Operand count가 overflow가 일어나는 값 (0 이하)을 받았다면, 클라이언트는 close()하고 종료

![Image](https://github.com/user-attachments/assets/3bcaba48-b2a2-4b43-8d60-417760f5e2f0)

## 참고 사항

- 사용된 주요 시스템 콜: `iovec`, `pipe`, `fork`, `select`, `readv`, `writev`
- 자식 프로세스는 파일이 아닌 **메모리 내 문자열 배열**로 수식 기록을 관리
- 서버 종료 시 저장 데이터는 유지되지 않음
