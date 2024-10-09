#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFSIZE 512

char buf[BUFSIZE];
char nickName[BUFSIZE];
char sendBuf[BUFSIZE];
struct sockaddr_in peeraddr;
struct sockaddr_in serveraddr;
int server_port = 0;
char server_ip[INET_ADDRSTRLEN];

void err_quit(const char *msg);
void *sendMessage(void *arg);
void *rcvMessage(void *arg);

struct threadArgs {
    int clntSock; // Socket descriptor for client
    struct sockaddr_in serveraddr;
};

int main(int argc, char *argv[]) {
    
    printf("Enter server IP address: ");
    fgets(server_ip, sizeof(server_ip), stdin);
    server_ip[strcspn(server_ip, "\n")] = 0; // 개행 문자 제거

    printf("Enter server port number: ");
    scanf("%d", &server_port); // server default setting: 9000
    while (getchar() != '\n'); // 입력 버퍼 클리어

    if (server_port <= 0 || server_port > 65535) {
        printf("Invalid port number.\n");
        exit(EXIT_FAILURE);
    }

    // Server address initialize
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(server_port);
    serveraddr.sin_addr.s_addr = inet_addr(server_ip);

    // Create client socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        err_quit("socket()");
    }

    struct threadArgs *threadArgs = (struct threadArgs *)malloc(sizeof(struct threadArgs));
    threadArgs->clntSock = sock; // 클라이언트 소켓 저장
    threadArgs->serveraddr = serveraddr;

    // 닉네임 입력
    printf("Enter NickName: ");
    scanf("%s", nickName);
    while (getchar() != '\n'); // 입력 버퍼 클리어

    printf("If you want menu, send \"menu\"");

    pthread_t sendThread;
    pthread_t recvThread;

    pthread_create(&sendThread, NULL, sendMessage, threadArgs);
    pthread_create(&recvThread, NULL, rcvMessage, threadArgs);

    pthread_detach(sendThread);
    pthread_detach(recvThread);

    // 메인 스레드는 종료되지 않도록 무한 대기
    while (1) {
        sleep(1); // CPU 사용을 줄이기 위한 대기
    }

    close(sock); // 이 부분은 실행되지 않지만, 프로그램이 끝나면 소켓을 닫아야 합니다.
    free(threadArgs); // 메모리 해제
    return 0;
}

void *sendMessage(void *arg) {
    struct threadArgs *threadArgs = (struct threadArgs *)arg;
    int sock = threadArgs->clntSock; // 클라이언트 소켓 가져오기
    

    while (1) {
        // 메시지 입력
        printf("\n[enter message] ");

        if (fgets(buf, BUFSIZE, stdin) == NULL)
            break;

        buf[strcspn(buf, "\n")] = 0; 

        if(strcmp(buf, "menu")==0){
            printf("Menu\n1. info 2. stat 3. quit\n");
            printf("[select menu]");
            fgets(buf, BUFSIZE, stdin);
            buf[strcspn(buf, "\n")] = 0;

            sprintf(sendBuf, "[%s] %s", nickName, buf);

            // 클라이언트 메뉴 선택 전송: info, stat, quit
            int send = sendto(sock, sendBuf, strlen(sendBuf), 0, (struct sockaddr *)&threadArgs->serveraddr, sizeof(threadArgs->serveraddr));
            continue;
        }

        if (strlen(buf) == 0)
            break;


        // 메시지 전송
        sprintf(sendBuf, "[%s] %s", nickName, buf);
        int send = sendto(sock, sendBuf, strlen(sendBuf), 0, (struct sockaddr *)&threadArgs->serveraddr, sizeof(threadArgs->serveraddr));
        if (send < 0) {
            perror("sendto()");
        }
    }
    return (NULL);
}

void *rcvMessage(void *arg) {
    struct threadArgs *threadArgs = (struct threadArgs *)arg;
    int sock = threadArgs->clntSock; // 클라이언트 소켓 가져오기

    while (1) {
        char buf[BUFSIZE];
        socklen_t addrlen = sizeof(peeraddr);
        int recv = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);
        if (recv < 0) {
            perror("recvfrom()");
            continue;
        }
        buf[recv] = '\0';
        printf("\n%s\n", buf);
    }
}

void err_quit(const char *msg) {
    perror(msg);
    exit(-1);
}



// gcc -o client UDPLiveClient.c
// ./client