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
    int alert;

    // Socket Initialize
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        err_quit("socket()");
    }

    // 서버 주소 세팅과 바인딩
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    // serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_addr.s_addr = inet_addr("172.30.1.73");

    retval = bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval < 0) err_quit("bind()");

    // 변수
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char alertBuf[BUFSIZE];
    char buf[BUFSIZE + 1];
    uint16_t command;
    uint16_t sequence;
    int len;
    int totalByte = 0;
    int messageNumber = 0;

    while(1){
        // 데이터 수신
        memset(buf, 0, sizeof(buf));
        memset(alertBuf, 0, sizeof(alertBuf));

        addrlen = sizeof(clientaddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);
        totalByte += retval;
        messageNumber += 1;

        // 에러 처리
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
        message[retval - sizeof(uint16_t)] = '\0'; // 널 종료 문자 추가

        // 수신 성공시 클라이언트에게 메시지 전송
        sprintf(alertBuf, "(%s:%d)가 (%s:%d)로 부터 (%d) 바이트 메시지 수신: %s\n",
               inet_ntoa(serveraddr.sin_addr),
               ntohs(serveraddr.sin_port),
               inet_ntoa(clientaddr.sin_addr),
               ntohs(clientaddr.sin_port),
               retval,
               message);

        alert = sendto(sock, alertBuf, strlen(alertBuf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));

        usleep(100 * 1000);

        // 커멘드에 따른 서버 응답
        switch(command) {
            case 1:  // Echo
                retval = sendto(sock, message, strlen(message), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                break;

            case 2: // Chat
                printf("\n[enter message]");
                if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
                break;

                len = strlen(buf);
                if (buf[len - 1] == '\n')
                buf[len - 1] = '\0'; // 추후 strcmp와 같이 사용을 위해 종단문자를 추가하여 저장
                if (strlen(buf) == 0)
                break;

                retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                break;

            case 3: // Stat
                if(strcmp(message, "bytes")==0){
                    sprintf(buf, "Total Bytes: %dbyte", totalByte);
                    retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                    break;
                }
                if(strcmp(message, "number")==0){
                    sprintf(buf, "Total Message: %d", messageNumber);
                    retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                    break;
                }
                if(strcmp(message, "both")==0){
                    sprintf(buf, "Total Message: %d\nTotal Bytes: %dbyte", messageNumber, totalByte);
                    retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                    break;
                }
                else{
                    sprintf(buf, "Total Message: %d\nTotal Bytes: %dbyte", messageNumber, totalByte);
                    retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                    break;
                }
            case 4: // Quit
                close(sock);
                printf("Socket Close\n");
                return 0;
        }

        memset(buf, 0, sizeof(buf));

        
        // 에러 처리
        if (retval < 0) {
            perror("sendto()");
            continue;
        }
    }
}

// Test
// gcc -o server UDPServer.c
// ./server

