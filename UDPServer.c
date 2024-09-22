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

        // 메시지 추출
        char message[BUFSIZE - sizeof(uint16_t) + 1];
        memcpy(message, buf + sizeof(uint16_t), retval - sizeof(uint16_t));
        message[retval - sizeof(uint16_t)] = '\0'; // 널 종료 문자 추가해야됨

        // 커맨드와 메시지 출력
        printf("[UDP/%s:%d] Command: 0x%04x, Message: %s\n",
               inet_ntoa(clientaddr.sin_addr),
               ntohs(clientaddr.sin_port),
               command,
               message);

        // // Print received data
        // buf[retval] = '\0'; // 종료 문자 추가
        // printf("[UDP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf);

        // // Echo
        // retval = sendto(sock, buf, retval, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        // if (retval < 0) {
        //     perror("sendto()");
        //     continue;
        // }
        
    }
}

// Test
// gcc -o server UDPServer.c
// ./server

