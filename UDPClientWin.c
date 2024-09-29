#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> 
#include <winsock2.h>   // Windows 소켓 관련 헤더
#include <ws2tcpip.h>   // inet_pton() 등 함수

#define BUFSIZE 512

#pragma comment(lib, "ws2_32.lib") // 링크 추가

// 오류 처리 함수
void err_quit(const char *msg) {
    perror(msg);   
    exit(-1);      
}

int main() {
    int retval;
    WSADATA wsaData;

    // 윈속 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        err_quit("WSAStartup() failed");
    }

    // IP 주소와 포트 번호 입력 받기
    char server_ip[INET_ADDRSTRLEN];  
    int server_port = 0;

    printf("Enter server IP address: ");
    fgets(server_ip, sizeof(server_ip), stdin);
    server_ip[strcspn(server_ip, "\n")] = 0;  

    printf("Enter server port number: ");
    scanf("%d", &server_port);

    while (getchar() != '\n'); 

    if (server_port <= 0 || server_port > 65535) {
        printf("Invalid port number.\n");
        return -1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        err_quit("socket()");
    }

    // 서버 주소 세팅
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &serveraddr.sin_addr.s_addr);

    struct sockaddr_in peeraddr;
    socklen_t addrlen;
    char commandBuf[6];
    char buf[BUFSIZE + 1];
    char sendBuf[BUFSIZE + 1 + sizeof(uint16_t)];
    int len;
    uint16_t command;

    while (1) {
        printf("\n[enter command] ");
        if (fgets(commandBuf, BUFSIZE + 1, stdin) == NULL)
            break;

        len = strlen(commandBuf);
        if (commandBuf[len - 1] == '\n')
            commandBuf[len - 1] = '\0';
        if (strlen(commandBuf) == 0)
            break;

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
            continue;  
        }

        if (strcmp(commandBuf, "quit") == 0){
            memcpy(sendBuf, &command, sizeof(command));
            retval = sendto(sock, sendBuf, sizeof(command) + strlen(buf), 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));
            closesocket(sock);
            WSACleanup();  // 윈속 정리
            return 0;
        }

        printf("\n[enter message] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;

        len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        memcpy(sendBuf, &command, sizeof(command));  
        memcpy(sendBuf + sizeof(command), buf, strlen(buf) + 1);  

        retval = sendto(sock, sendBuf, sizeof(command) + strlen(buf), 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));

        if (retval < 0) {
            perror("sendto()");
            continue;
        }
        printf("[Test] sent %dbyte.\n", retval);

        memset(buf, 0, sizeof(buf));

        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);

        buf[retval] = '\0';
        printf("[Test] Received %dbyte.\n", retval);
        printf("[Received message] %s\n", buf);
        
    }

    closesocket(sock);  // Windows의 소켓 종료
    WSACleanup();  // 윈속 정리
    printf("Socket Closed\n");

    return 0;
}
