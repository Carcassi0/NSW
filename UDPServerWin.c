#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>      // for Winsock functions
#include <Ws2tcpip.h>      // for inet_ntoa() and htons()
#pragma comment(lib, "ws2_32.lib")  // Winsock 라이브러리

#define BUFSIZE 512

// 오류 처리 함수
void err_quit(const char *msg) {
    perror(msg);  // 시스템 오류 메시지 출력
    exit(-1);     // 프로그램 종료
}

int main(int argc, char *argv[]){
    int retval;

    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        err_quit("WSAStartup()");
    }

    // Socket Initialize
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == INVALID_SOCKET){
        err_quit("socket()");
    }

    // 서버 주소 세팅과 바인딩
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // 변수
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];
    uint16_t command;
    int len;
    int totalByte = 0;
    int messageNumber = 0;

    while(1){
        // 데이터 수신
        addrlen = sizeof(clientaddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);
        totalByte += retval;
        messageNumber += 1;

        // 에러 처리
        if (retval == SOCKET_ERROR) {
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

        // 커맨드와 메시지 출력
        printf("(%s:%d)가 (%s:%d)로 부터 (%d) 바이트 메시지 수신: %s\n",
               inet_ntoa(serveraddr.sin_addr),
               ntohs(serveraddr.sin_port),
               inet_ntoa(clientaddr.sin_addr),
               ntohs(clientaddr.sin_port),
               retval,
               message);

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
                if(strcmp(message, "bytes") == 0){
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "Total Bytes: %dbyte", totalByte);
                    retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                }
                if(strcmp(message, "number") == 0){
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "Total Message: %d", messageNumber);
                    retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                }
                if(strcmp(message, "both") == 0){
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "Total Message: %d\nTotal Bytes: %dbyte", messageNumber, totalByte);
                    retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                }
                else{
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "Total Message: %d\nTotal Bytes: %dbyte", messageNumber, totalByte);
                    retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                }
                break;
            case 4: // Quit
                closesocket(sock);
                WSACleanup();
                printf("Socket Close\n");
                return 0;
            default: // 기본 세팅 Stat
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "Total Message: %d Total Bytes: %dbyte", messageNumber, totalByte);
                retval = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        }

        // 에러 처리
        if (retval == SOCKET_ERROR) {
            perror("sendto()");
            continue;
        }
    }
}
