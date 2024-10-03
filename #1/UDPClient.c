#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // uint16_t 사용
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for inet_addr() and htons()
#include <sys/socket.h>  // for socket functions
#include <netinet/in.h>  // for sockaddr_in

#define BUFSIZE 512

// 오류 처리 함수
void err_quit(const char *msg) {
    perror(msg);   // 시스템 오류 메시지 출력
    exit(-1);      // 프로그램 종료
}

int main() {
    int retval;

    // IP 주소와 포트 번호 입력 받기
    char server_ip[INET_ADDRSTRLEN];  // IPv4 주소의 최대 길이로 설정
    int server_port = 0;

    printf("Enter server IP address: ");
    fgets(server_ip, sizeof(server_ip), stdin);
    server_ip[strcspn(server_ip, "\n")] = 0;  // 개행 문자 제거

    printf("Enter server port number: ");
    scanf("%d", &server_port); // server default setting: 9000

    // Server port 입력 후 입력 버퍼 클리어
    while (getchar() != '\n'); // 남아있는 모든 문자를 읽어버리면서 버퍼를 클리어


    if (server_port <= 0 || server_port > 65535) {
        printf("Invalid port number.\n");
        return -1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        err_quit("socket()");
    }

    // 서버 주소 세팅
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(server_port);
    serveraddr.sin_addr.s_addr = inet_addr(server_ip); 

    // 변수
    struct sockaddr_in peeraddr;
    socklen_t addrlen;
    char commandBuf[6];
    char buf[BUFSIZE + 1];
    char sendBuf[BUFSIZE + 1 + sizeof(uint16_t)];
    int len;
    uint16_t command;

    while(1){
        // 커맨드 입력
        printf("\n[enter command] ");
        if (fgets(commandBuf, BUFSIZE + 1, stdin) == NULL)
            break;

        // 커맨드 가공
        len = strlen(commandBuf);
        if (commandBuf[len - 1] == '\n')
            commandBuf[len - 1] = '\0';
        if (strlen(commandBuf) == 0)
            break;

        // 커맨드 입력에 따른 전송 데이터 세팅
        if (strcmp(commandBuf, "echo") == 0) {
            command = htons(0x01);
        } else if (strcmp(commandBuf, "chat") == 0) {
            command = htons(0x02);
        } else if (strcmp(commandBuf, "stat") == 0) {
            command = htons(0x03);
        } else if (strcmp(commandBuf, "quit") == 0) {
            command = htons(0x04);
        } else {
            printf("Unknown command\n");
            continue;  // 유효하지 않은 커맨드일 때 루프 다시 시작
        }

        // quit 커맨드는 메시지를 수신하지 않고 바로 서버에 quit 커맨드 전송과 함께 소켓 종료
        if (strcmp(commandBuf, "quit") == 0){
            memcpy(sendBuf, &command, sizeof(command));
            retval = sendto(sock, sendBuf, sizeof(command) + strlen(buf), 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));
            close(sock);
            return 0;
        }
        
        // 메시지 입력
        printf("\n[enter message] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;

        // 메시지 가공
        len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        // 새로운 버퍼에 커맨드와 메시지 결합 => 기본적으로 커맨드와 메시지는 합쳐서 한 번에 전송
        memcpy(sendBuf, &command, sizeof(command));  // 전송 버퍼에 명령어 추가
        memcpy(sendBuf + sizeof(command), buf, strlen(buf) + 1);  // 전송 버퍼에 메시지 추가 (+1은 NULL 문자 포함)

        // 전송
        retval = sendto(sock, sendBuf, sizeof(command) + strlen(buf), 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));

        // 에러 확인
        if (retval < 0) {
            perror("sendto()");
            continue;
        }
    
        // 전송 후 데이터 수신 위해 버퍼 초기화
        memset(buf, 0, sizeof(buf));

        // 데이터 수신
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);

        // 수신된 메시지 출력
        buf[retval] = '\0';
        printf("[Received message] %s\n", buf);

        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);

        // 수신된 메시지 출력
        buf[retval] = '\0';
        printf("[Received message] %s\n", buf);
        
    }

    // 소켓 종료
    close(sock);
    printf("Socket Close\n");

    return 0;
}

// Test
// gcc -o client UDPClient.c
// ./client
