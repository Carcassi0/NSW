#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for inet_ntoa() and htons()
#include <sys/socket.h>  // for socket functions
#include <netinet/in.h>  // for sockaddr_in

#define BUFSIZE 512

// 오류 처리 함수
void err_quit(const char *msg) {
    perror(msg);  // 시스템 오류 메시지 출력
    exit(-1);     // 프로그램 종료
}

int main(int argc, char *argv[]){
    int retval;

    // Socket Initialize
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        err_quit("socket()");
    }

    // Server address set with binding
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval < 0) err_quit("bind()");

    // Variable
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    uint16_t command;

    while(1){
        // Receive Data
        addrlen = sizeof(clientaddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);

        // Error Handler
        if (retval < 0) {
            perror("recvfrom()");
            continue;
        }
         // 커맨드와 메시지를 분리
        if(retval < sizeof(uint16_t)){
            printf("[UDP/%s:%d] 받은 데이터가 너무 짧습니다.\n",
                   inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
            continue;
        }

        // 커맨드 추출
        memcpy(&command, buf, sizeof(uint16_t));
        command = ntohs(command);
        printf("Received Command: %hu\n", command);  // 제대로 받은 커맨드 출력

        // 메시지 추출
        char message[BUFSIZE - sizeof(uint16_t) + 1];
        memcpy(message, buf + sizeof(uint16_t), retval - sizeof(uint16_t));
        message[retval - sizeof(uint16_t)] = '\0'; // 널 종료 문자 추가

        // 커맨드와 메시지 출력
        printf("(%s:%d)가 (%s:%d)로 부터 Command: 0x%04x, Message: %s\n",
               inet_ntoa(serveraddr.sin_addr),
               nthos(serveraddr.sin_port),
               inet_ntoa(clientaddr.sin_addr),
               ntohs(clientaddr.sin_port),
               command,
               message);

        // 커멘드에 따른 서버 응답
        switch(command) {
            case 1:  // Echo 
                printf("[Server] Command 1 (Echo) Detected\n");
                retval = sendto(sock, message, strlen(message) + 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                break;
            case 2: // Chat
                printf("[Server] Command 2 Detected\n");
                printf("\n[enter message] ");
                retval = sendto(sock, message, strlen(message) + 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                break;
            case 3:
                printf("[Server] Command 3 Detected\n");
                retval = sendto(sock, message, strlen(message) + 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                break;
            case 4:
                printf("[Server] Command 4 Detected\n");
                retval = sendto(sock, message, strlen(message) + 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                break;
            default:
                printf("[Server] Unknown Command\n");
        }

        if (retval < 0) {
            perror("sendto()");
            continue;
        }
    }
}

// Test
// gcc -o server UDPServer.c
// ./server


// 9.24 14:32 현재 서버의 조건문(클라이언트 커멘드 식별)이 작동하지 않음 => 사용자가 어떻게 보냈는지 확인하기
// ❯ gcc -o server UDPServer.c
// ❯ ./server
// 1[UDP/127.0.0.1:53232] Command: 0x0001, Message: what