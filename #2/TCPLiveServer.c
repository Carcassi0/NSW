#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

#define PORT 9000
#define BUFSIZE 512
#define MAX_USERS 100

struct ThreadArgs
{
    int clientSock;
    struct sockaddr_in clientaddr;
};

struct userData
{
    int clientSockFd;
    char nickName[20];
    struct sockaddr_in addr;
    int isOut;
};

char mainBuf[BUFSIZE];

char commandBuf[5];
char alertBuf[BUFSIZE];
int offset = 0;
struct userData userList[MAX_USERS];
int userCount = 0;
int totalUserCount = 0;
int serverRunning = 1;

// 통계 변수
int messageNumber = 0;

void *ThreadMain(void *arg);
void *userThread(void *arg);
void err_quit(const char *msg);
int sameUser(char user[]);
void createUser(char user[], struct sockaddr_in addr, int sock);
time_t start, end;
double programTime;

int main(int argc, char *argv[])
{

    int retval, serverSock, sock, *addSock;

    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        return 0;
    }
    start = time(NULL);

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t addressLen = sizeof(struct sockaddr_in);

    bind(serverSock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));


    printf("Waiting for clients...\n");

    while (serverRunning)
    {
        pthread_t mainThread;
        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        socklen_t addrlen = sizeof(threadArgs->clientaddr);

        if (listen(serverSock, MAX_USERS) < 0)
        {
            perror("listen failed");
            close(serverSock);
            return 1;
        }

        sock = accept(serverSock, (struct sockaddr *)&threadArgs->clientaddr, &addrlen);

        threadArgs->clientSock = sock;
        // addSock = malloc(1);
        // *addSock = clientSock;

        pthread_create(&mainThread, NULL, ThreadMain, threadArgs);
    } 

    sprintf(alertBuf, "\nServer Closed");

    // for (int i = 0; i < userCount; i++)
    // {
    //     sendto(sock, alertBuf, strlen(alertBuf), 0, (struct sockaddr *)&userList[i].addr, sizeof(userList[i].addr));
    // }
    printf("Server Closed\n");
    close(serverSock);
    return 0;
}

void err_quit(const char *msg)
{
    perror(msg); // 시스템 오류 메시지 출력
    exit(-1);    // 프로그램 종료
}

int sameUser(char user[])
{
    for (int i = 0; i < totalUserCount; i++)
    {
        if (strcmp(userList[i].nickName, user) == 0)
        {
            return 1; // 존재하는 유저
        }
    }
    return 0; // 존재하지 않는 유저
}

void *ThreadMain(void *arg)
{
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());

    // Extract socket and client address from argument
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)arg;
    int sock = threadArgs->clientSock;

    userThread(threadArgs);

    // 메모리 해제
    free(threadArgs);
    close(sock);
    return NULL;
}

void createUser(char user[], struct sockaddr_in addr, int sock)
{
    if (totalUserCount < MAX_USERS) // 최대 유저 수 확인
    {
        strcpy(userList[totalUserCount].nickName, user); // 유저 닉네임 저장
        userList[totalUserCount].addr = addr;            // 유저 주소 저장
        userList[totalUserCount].clientSockFd = sock;
        userList[totalUserCount].isOut = 0;
        totalUserCount++; // 유저 수 증가
        userCount++;
    }
}

void *userThread(void *arg)
{
    pthread_detach(pthread_self());

    char nickNameBuf[30];
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)arg;
    struct sockaddr_in clientaddr = threadArgs->clientaddr;
    int sock = threadArgs->clientSock;
    int recv;

    while ((recv = read(sock, mainBuf, sizeof(mainBuf))) > 0)
    {
        mainBuf[recv] = '\0';
        messageNumber += 1;

        // 유저 닉네임 추출 (닉네임 형식: "[nickname] message")
        sscanf(mainBuf, "[%19[^]]] %s", nickNameBuf, commandBuf); // 닉네임 및 명령어 추출

        // 만약 신규 유저라면, 신규 유저 생성
        if (sameUser(nickNameBuf) != 1)
        {
            sprintf(alertBuf, "\nUser \'%s\' joined channel ", nickNameBuf);
            createUser(nickNameBuf, clientaddr, threadArgs->clientSock);

            // 신규 유저 접속 사실을 기존 사용자들에게 에코
            for (int i = 0; i < totalUserCount; i++)
            {
                if (userList[i].isOut != 1)
                {
                    send(userList[i].clientSockFd, alertBuf, strlen(alertBuf), 0);
                }
            }
        }

        memset(alertBuf, 0, sizeof(alertBuf));

        // 명령어 처리
        if (strcmp(commandBuf, "info") == 0)
        {
            offset += sprintf(alertBuf, "클라이언트 수: %d", userCount);

            for (int i = 0; i < totalUserCount; i++)
            {
                if (userList[i].isOut != 1)
                {
                    offset += sprintf(alertBuf + offset, "\n[%s] %s: %d",
                                      userList[i].nickName, inet_ntoa(userList[i].addr.sin_addr), ntohs(userList[i].addr.sin_port));
                }
            }
            send(sock, alertBuf, strlen(alertBuf), 0);
            memset(alertBuf, 0, sizeof(alertBuf));
        }
        else if (strcmp(commandBuf, "stat") == 0)
        {
            end = time(NULL);
            double passedTime = difftime(end, start);
            programTime = messageNumber / (passedTime/60.0);
            sprintf(alertBuf, "1분당 평균 메시지 수: %f", programTime);
            send(sock, alertBuf, strlen(alertBuf), 0);
            memset(alertBuf, 0, sizeof(alertBuf));
        }
        else if (strcmp(commandBuf, "quit") == 0)
        {
            userCount--; // totalUserCount는 변경하지 않음
            sprintf(alertBuf, "\nUser \'%s\' disconnected from channel ", nickNameBuf);
            for (int i = 0; i < totalUserCount; i++)
            {
                if (strcmp(nickNameBuf, userList[i].nickName) == 0)
                {
                    userList[i].isOut = 1; // 종료 유저 플래그
                }
            }
            for (int i = 0; i < totalUserCount; i++)
            {
                if (userList[i].isOut != 1)
                {
                    send(userList[i].clientSockFd, alertBuf, strlen(alertBuf), 0);
                }
            }
            printf("\nUser \'%s\' disconnected from channel ", nickNameBuf);
            memset(alertBuf, 0, sizeof(alertBuf));
            return NULL;
        }
        else
        {
            // 일반 메시지 처리: 다른 사용자에게 전달
            for (int i = 0; i < userCount; i++)
            {
                if (strcmp(userList[i].nickName, nickNameBuf) != 0) // 발신자에게는 echo하지 않음
                {
                    send(userList[i].clientSockFd, mainBuf, recv, 0);
                }
            }
            memset(alertBuf, 0, sizeof(alertBuf));
            memset(mainBuf, 0, sizeof(mainBuf));
        }
    }
}

// gcc -o tcpserver TCPLiveServer.c
// ./tcpserver
