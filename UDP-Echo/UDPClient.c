#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    char buf[BUFSIZE + 1];
    int len;
    int command;

    while(1){
        // Command set
        printf("\n[enter command] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;

        // '\n' 문자 제거
        len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        // Command Generator
        if (strcmp(buf, "echo") == 0) {
            command = 0x01;
        } else if (strcmp(buf, "chat") == 0) {
            command = 0x02;
        } else if (strcmp(buf, "stat") == 0) {
            command = 0x03;
        } else if (strcmp(buf, "quit") == 0) {
            command = 0x04;
        } else {
            printf("Unknown command\n");
            continue;  // 유효하지 않은 명령일 때 루프 다시 시작
        }

        printf("\n[enter message] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;
        
        
        retval = sendto(sock, command+buf, strlen(command+buf), 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));

        // 에러 확인
        if (retval < 0) {
            perror("sendto()");
            continue;
        }
        
        // 전송 확인
        printf("[UDP Client] sent %dbyte.\n", retval);
        printf("[UDP Client] sent: %d.\n", command);

        if (memcmp(&peeraddr, &serveraddr, sizeof(peeraddr))) {
            printf("[error] Wrong receiver!\n");
            continue;
        }

        // 수신된 메시지 출력
        buf[retval] = '\0';
        printf("[UDP Client] Received %dbyte.\n", retval);
        printf("[Received message] %s\n", buf);
        
        // 메시지 전송시 buf 앞에 command 붙여서 전송
    }
    // Protocol
    // echo(0x01), chat(0x02), stat(0x03), quit(0x04)

    // 소켓 종료
    close(sock);

    return 0;
}