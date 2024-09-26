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

int main(int argc, char *argv[]){

    int retval;
    

    // Socket initialize
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        err_quit("socket()");
    }

    // Server address set
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Variable
    struct sockaddr_in peeraddr;
    socklen_t addrlen;
    char commandBuf[6];
    char buf[BUFSIZE + 1];
    char sendBuf[BUFSIZE + 1 + sizeof(uint16_t)];
    int len;
    uint16_t command; 

    while(1){
        // Command set
        printf("\n[enter command] ");
        if (fgets(commandBuf, BUFSIZE + 1, stdin) == NULL)
            break;

        // '\n' 문자 제거
        len = strlen(commandBuf);
        if (commandBuf[len - 1] == '\n')
            commandBuf[len - 1] = '\0';
        if (strlen(commandBuf) == 0)
            break;

        // Command Generator
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
            continue;  // 유효하지 않은 명령일 때 루프 다시 시작
        }

        if (strcmp(commandBuf, "quit") == 0){
            memcpy(sendBuf, &command, sizeof(command));
            retval = sendto(sock, sendBuf, sizeof(command) + strlen(buf) + 1, 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));
            close(sock);
            return 0;
        }

        printf("\n[enter message] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;

        // message 가공
        len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;
        
        
        // 새로운 버퍼에 command와 message 결합
        memcpy(sendBuf, &command, sizeof(command));  // 버퍼에 명령어 추가
        memcpy(sendBuf + sizeof(command), buf, strlen(buf) + 1);  // 버퍼에 메시지 추가 (+1은 NULL 문자 포함)

        // 전송
        retval = sendto(sock, sendBuf, sizeof(command) + strlen(buf) + 1, 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));

        // 에러 확인
        if (retval < 0) {
            perror("sendto()");
            continue;
        }
        // 전송 확인
        printf("[Test] sent %dbyte.\n", retval);
    
        memset(buf, 0, sizeof(buf));

        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);

        // 수신된 메시지 출력
        buf[retval] = '\0';
        printf("[Test] Received %dbyte.\n", retval);
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