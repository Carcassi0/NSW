#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>     // for close()
#include <arpa/inet.h>  // for inet_addr() and htons()
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> // for sockaddr_in

#define BUFSIZE 512

char buf[BUFSIZE];
char nickName[BUFSIZE];
char sendBuf[BUFSIZE];
int len;
struct sockaddr_in peeraddr;
socklen_t addrlen;
int server_port = 0;
char server_ip[INET_ADDRSTRLEN];

void *ThreadMain(void *arg); // Main program of a thread
void err_quit(const char *msg);
void *sendMessage(void *arg);
void *rcvMessage(void *arg);

struct ThreadArgs
{
    int clntSock; // Socket descriptor for client
};

int main(int argc, char *argv[])
{

    int retval;
    // 클라이언트 정보
    // 닉네임, 메시지 => 채팅창 출력(전송) 형식: "[닉네임] 채팅 메시지"
    // UDP 서버는 클라이언트가 보낸 닉네임을 바탕으로 채팅 참여자 정보 수집

    // IP 주소와 포트 번호 그리고 채팅 닉네임 입력 받기
    // char server_ip[INET_ADDRSTRLEN];  // IPv4 주소의 최대 길이로 설정
    // int server_port = 0;

    // Server address initialize
    // struct sockaddr_in serveraddr;
    // memset(&serveraddr, 0, sizeof(serveraddr));
    // serveraddr.sin_family = AF_INET;
    // serveraddr.sin_port = htons(server_port);
    // serveraddr.sin_addr.s_addr = inet_addr(server_ip);

    for (;;)
    {
        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));

        pthread_t sendThread;
        pthread_t recvThread; // Create client thread for send
        int sendReturn = pthread_create(&sendThread, NULL, sendMessage, threadArgs);
        int recvReturn = pthread_create(&recvThread, NULL, rcvMessage, threadArgs);

        free(threadArgs);

    }
    return 0;
}

void *ThreadMain(void *threadArgs)
{
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());

    // Extract socket file descriptor from argument
    int clntSock = ((struct ThreadArgs *)threadArgs)->clntSock;
    free(threadArgs); // Deallocate memory for argument

    HandleTCPClient(clntSock);

    return (NULL);
}

void *sendMessage(void *arg)
{ // 발신 스레드
    pthread_detach(pthread_self());

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int send;

    if (sock < 0)
    {
        err_quit("socket()");
    }

    // Server address initialize
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(server_port);
    serveraddr.sin_addr.s_addr = inet_addr(server_ip);

    printf("Enter server IP address: ");
    fgets(server_ip, sizeof(server_ip), stdin);
    server_ip[strcspn(server_ip, "\n")] = 0; // 개행 문자 제거

    printf("Enter server port number: ");
    scanf("%d", &server_port); // server default setting: 9000
    while (getchar() != '\n')
        ; // 입력 버퍼 클리어

    if (server_port <= 0 || server_port > 65535)
    {
        printf("Invalid port number.\n");
        return -1;
    }

    while (1)
    {
        // 사용자 닉네임 받기
        printf("Enter NickName: ");
        scanf("%s", &nickName);

        len = strlen(nickName);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        while (getchar() != '\n')
            ; // 입력 버퍼 클리어

        // 메시지 입력
        printf("\n[enter message] ");
        if (fgets(buf, BUFSIZE, stdin) == NULL)
            break;

        len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        sprintf(sendBuf, "[%s] %s\n", nickName, buf);

        send = sendto(sock, sendBuf, sizeof(sendBuf), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

        memset(buf, 0, sizeof(buf));
        memset(sendBuf, 0, sizeof(sendBuf));
    }
}

void *rcvMessage(void *arg)
{ // 수신 스레드
    pthread_detach(pthread_self());
    int recv;

    while (1)
    {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);

        addrlen = sizeof(peeraddr);

        recv = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);

        // 수신된 메시지 출력
        buf[recv] = '\0';
        printf("[Received message] %s\n", buf);
        memset(buf, 0, sizeof(buf));
    }
}

void err_quit(const char *msg)
{
    perror(msg); // 시스템 오류 메시지 출력
    exit(-1);    // 프로그램 종료
}

// gcc -o server UDPLiveClient.c
// ./client