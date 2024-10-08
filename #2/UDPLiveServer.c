#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>     // for close()
#include <arpa/inet.h>  // for inet_addr() and htons()
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> // for sockaddr_in
#include <time.h>       // for time stat

#define BUFSIZE 512
#define MAX_USERS 100

struct ThreadArgs
{
    int clientSock;                // Socket descriptor for client
    struct sockaddr_in clientaddr; // Client address
};

struct userData
{
    int avail;               // Availability flag
    char nickName[20];       // User nickname
    struct sockaddr_in addr; // User address
};

char mainBuf[BUFSIZE];
char nickNameBuf[BUFSIZE];
char commandBuf[5];
char alertBuf[BUFSIZE];
int offset = 0; // 문자열 이어 쓰기를 위한 변수
struct userData userList[MAX_USERS];
int userCount = 0;

// 통계 변수
int messageNumber = 0;

void *ThreadMain(void *arg); // Main program of a thread
void *recvMessage(void *arg);
void err_quit(const char *msg);
int sameUser(char user[]);
void createUser(char user[], struct sockaddr_in addr);
clock_t start, end;
double cpu_time_used;

int main(int argc, char *argv[])
{
    int retval;
    int listenSock = socket(PF_INET, SOCK_DGRAM, 0);

    start = clock();

    if (listenSock < 0)
    {
        err_quit("socket()");
    }

    // Server address initialize and binding
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(listenSock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval < 0)
        err_quit("bind()");

    printf("Server is running on port 9000...\n");

    for (;;)
    {
        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        socklen_t addrlen = sizeof(threadArgs->clientaddr);
        threadArgs->clientSock = listenSock;

        // 수신 대기 및 클라이언트 주소 저장
        recvfrom(listenSock, mainBuf, BUFSIZE, 0, (struct sockaddr *)&threadArgs->clientaddr, &addrlen);

        // 새로운 스레드 생성
        pthread_t mainThread;
        int mainReturn = pthread_create(&mainThread, NULL, ThreadMain, threadArgs);
        if (mainReturn != 0)
        {
            free(threadArgs);
            err_quit("pthread_create()");
        }
    }

    close(listenSock);
    return 0;
}

void *ThreadMain(void *arg)
{
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());

    // Extract socket and client address from argument
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)arg;
    int sock = threadArgs->clientSock;

    // 메시지 수신 스레드 생성
    recvMessage(threadArgs);

    // 메모리 해제
    free(threadArgs);
    return NULL;
}

void *recvMessage(void *arg)
{
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)arg;
    struct sockaddr_in clientaddr = threadArgs->clientaddr;
    int sock = threadArgs->clientSock;
    int recv;

    while (1)
    {
        socklen_t addrlen = sizeof(clientaddr);
        recv = recvfrom(sock, mainBuf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);
        if (recv < 0)
        {
            perror("recvfrom()");
            continue;
        }

        mainBuf[recv] = '\0';
        messageNumber += 1;

        // 유저 닉네임 추출 (닉네임 형식: "[nickname] message")
        sscanf(mainBuf, "[%19[^]]] %s", nickNameBuf, &mainBuf[21]); // 21: 닉네임 + ']' + ' '

        // 통계 커맨드 여부를 확인하기 위해 메시지 4글자 추출
        sscanf(mainBuf, "[%*[^]]] %4s", commandBuf);

        if (strcmp(commandBuf, "info"))
        {
            offset += sprintf(alertBuf, "클라이언트 수: %d", userCount);
            for (int i = 0; i < userCount; i++)
            {
                sprintf(alertBuf + offset, "\n[%s] %s: %d",
                        userList[i].nickName, inet_ntoa(userList[i].addr.sin_addr), userList[i].addr.sin_port);
            }
        }
        if (strcmp(commandBuf, "stat"))
        { // 분당 평균 메시지 수 제공
            end = clock();
            cpu_time_used = messageNumber/(((double)(end - start)) / CLOCKS_PER_SEC * 60); 
            printf("1분당 평균 메시지 수: %d", (int)cpu_time_used);
        }
        if (strcmp(commandBuf, "quit"))
        { // 종료 커맨드 => 서버 종료
            sprintf(alertBuf, "\nServer Closed");
            printf("Server Closed");
            close(sock);
            return 0;
        }

        // 만약 신규 유저라면, 신규 유저 생성
        if (sameUser(nickNameBuf) != 1)
        {
            createUser(nickNameBuf, clientaddr);
        }

        // 유저들에게 메시지 전송
        for (int i = 0; i < userCount; i++)
        {
            if (strcmp(userList[i].nickName, nickNameBuf) != 0) // 발신자에게는 echo하지 않음
            {
                sendto(sock, mainBuf, recv, 0, (struct sockaddr *)&userList[i].addr, sizeof(userList[i].addr));
            }
        }
    }
}

void err_quit(const char *msg)
{
    perror(msg); // 시스템 오류 메시지 출력
    exit(-1);    // 프로그램 종료
}

int sameUser(char user[])
{
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(userList[i].nickName, user) == 0)
        {
            return 1; // 존재하는 유저
        }
    }
    return 0; // 존재하지 않는 유저
}

void createUser(char user[], struct sockaddr_in addr)
{
    if (userCount < MAX_USERS) // 최대 유저 수 확인
    {
        userList[userCount].avail = 1;              // 유저 사용 가능으로 설정
        strcpy(userList[userCount].nickName, user); // 유저 닉네임 저장
        userList[userCount].addr = addr;            // 유저 주소 저장
        userCount++;                                // 유저 수 증가
    }
}
// gcc -o server UDPLiveServer.c
// ./server

// 통계 기능을 위한 서버 시작 시, 시간 측정
//