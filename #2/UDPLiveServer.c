#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>    
#include <arpa/inet.h>  
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <time.h>       

#define BUFSIZE 512
#define MAX_USERS 100

struct ThreadArgs
{
    int clientSock;                
    struct sockaddr_in clientaddr; 
};

struct userData
{
    int avail;               
    char nickName[20];       
    struct sockaddr_in addr;
};

char mainBuf[BUFSIZE];
char nickNameBuf[BUFSIZE];
char commandBuf[5];
char alertBuf[BUFSIZE];
int offset = 0; 
struct userData userList[MAX_USERS];
int userCount = 0;
int serverRunning = 0;

// 통계 변수
int messageNumber = 1;

void *ThreadMain(void *arg); 
void *recvMessage(void *arg);
// void *statMessage(void *arg);
void err_quit(const char *msg);
int sameUser(char user[]);
void createUser(char user[], struct sockaddr_in addr);
time_t start, end;
double programTime;

int main(int argc, char *argv[])
{
    int retval;
    int listenSock = socket(AF_INET, SOCK_DGRAM, 0);

    start = time(NULL);

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

    while (serverRunning)
    {
        // 유저 구조체 변수 하나 늘림
        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        // 구조체 변수의 주소 변수의 크기를 클라이언트 주소 정보 크기로 설정
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

        // 버퍼 초기화
        memset(mainBuf, 0, sizeof(mainBuf));
        memset(nickNameBuf, 0, sizeof(nickNameBuf));
        memset(commandBuf, 0, sizeof(commandBuf));

        recv = recvfrom(sock, mainBuf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);
        // if (recv < 0)
        // {
        //     perror("recvfrom()");
        //     continue;
        // }

        mainBuf[recv] = '\0';
        messageNumber += 1;

        // 유저 닉네임 추출 (닉네임 형식: "[nickname] message")
        sscanf(mainBuf, "[%19[^]]] %s", nickNameBuf, commandBuf); // 닉네임 및 명령어 추출

        // 만약 신규 유저라면, 신규 유저 생성
        if (sameUser(nickNameBuf) != 1)
        {
            createUser(nickNameBuf, clientaddr);
        }

        // 명령어 처리
        if (strcmp(commandBuf, "info") == 0)
        {
            offset += sprintf(alertBuf, "클라이언트 수: %d", userCount);

            for (int i = 0; i < userCount; i++)
            {
                offset += sprintf(alertBuf + offset, "\n[%s] %s: %d",
                        userList[i].nickName, inet_ntoa(userList[i].addr.sin_addr), ntohs(userList[i].addr.sin_port));
            }
            sendto(sock, alertBuf, strlen(alertBuf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
            memset(alertBuf, 0, sizeof(alertBuf));
        }
        else if (strcmp(commandBuf, "stat") == 0)
        {
            end = time(NULL);
            double passedTime = difftime(end, start);
            programTime = messageNumber / (passedTime/60.0);
            sprintf(alertBuf, "1분당 평균 메시지 수: %f", programTime);
            sendto(sock, alertBuf, strlen(alertBuf), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
            memset(alertBuf, 0, sizeof(alertBuf));
        }
        else if (strcmp(commandBuf, "quit") == 0)
        {
            sprintf(alertBuf, "\nServer Closed");
            for (int i = 0; i < userCount; i++){
                sendto(sock, alertBuf, strlen(alertBuf), 0, (struct sockaddr *)&userList[i].addr, sizeof(userList[i].addr));
            }    
            printf("Server Closed\n");
            serverRunning = 0;
            return NULL;
        }
        else
        {
            // 일반 메시지 처리: 다른 사용자에게 전달
            for (int i = 0; i < userCount; i++)
            {
                if (strcmp(userList[i].nickName, nickNameBuf) != 0) // 발신자에게는 echo하지 않음
                {
                    sendto(sock, mainBuf, recv, 0, (struct sockaddr *)&userList[i].addr, sizeof(userList[i].addr));
                    memset(alertBuf, 0, sizeof(alertBuf)); 
                    memset(mainBuf, 0, sizeof(mainBuf));

                }
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

// 와이어샤크 패킷 캡쳐 결과, 클라이언트는 정상적으로 발신하고 있으나 
// 간헐적으로 서버가 다른 사용자에게 에코하지 않음(다른 사용자로의 통신 자체가 없었음)
