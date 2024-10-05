#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>     // for close()
#include <arpa/inet.h>  // for inet_addr() and htons()
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> // for sockaddr_in

#define BUFSIZE 512

char mainBuf[BUFSIZE];
char nickNameBuf[BUFSIZE];
char sendBuf[BUFSIZE];
struct userData userList[BUFSIZE];
int userCount = 0;
int len;

// 통계 변수
int messageNumber = 0;

struct sockaddr_in clientaddr;
struct sockaddr_in serveraddr;

// memset(&serveraddr, 0, sizeof(serveraddr));
// serveraddr.sin_family = AF_INET;
// serveraddr.sin_port = htons(9000);
// serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

void *ThreadMain(void *arg); // Main program of a thread
void err_quit(const char *msg);
int sameUser(char user);
void createUser(char user);

struct ThreadArgs
{
  int clientSock; // Socket descriptor for client
};

struct userData
{
  int avail;
  char nickName[20];
  struct sockaddr_in addr;
};

int main(int argc, char *argv[])
{

  int retval;
  int listenSock = socket(PF_INET, SOCK_DGRAM, 0);

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

  for (;;)
  {

    struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(
        sizeof(struct ThreadArgs));

    pthread_t mainThread;
    int mainReturn = pthread_create(&mainThread, NULL, ThreadMain, threadArgs);
  }
  return 0;
}

void *ThreadMain(void *threadArgs)
{
  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());

  // Extract socket file descriptor from argument
  int clientSock = ((struct ThreadArgs *)threadArgs)->clientSock;

  pthread_t sendThread;
  pthread_t recvThread; // Create client thread for send
  int sendReturn = pthread_create(&sendThread, NULL, send, threadArgs);
  int recvReturn = pthread_create(&recvThread, NULL, recv, threadArgs);

  // free(threadArgs); // Deallocate memory for argument 양 스레드 끝나는 시점에 free 필요, 혹은 양 스레드 내부에서 처리

  return (NULL);
}

void *send(void *arg)
{
  pthread_detach(pthread_self());

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
  {
    err_quit("socket()");
  }

  while (1)
  { // broadcast
    for (int i = 0; sizeof(userList); i++)
    {

    }
  }
}

void *recv(void *arg)
{
  pthread_detach(pthread_self());

  int recv;
  int nameLength = 0;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  while (1)
  {

    int addrlen = sizeof(clientaddr);

    recv = recvfrom(sock, mainBuf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);
    if (recv >= 0)
    {
      messageNumber += 1;
    }
    if (recv < 0)
    {
      perror("recvfrom()");
      continue;
    }

    for (int i = 1; mainBuf[i] == ']'; i++)
    {
      nameLength += 1;
      nickNameBuf[i] = mainBuf[i];
    }
    if(sameUser(nickNameBuf)!=1){ // 새로운 유저라면
      userCount += 1; // 유저 수 하나 늘리고
      createUser(nickNameBuf); // 유저 DB에 정보 저장(닉네임, IP주소, 포트 번호)
    }

    char messageBuf[BUFSIZE - nameLength];
    memcpy(&messageBuf, mainBuf, nameLength + 1);
  }
}

void err_quit(const char *msg)
{
  perror(msg); // 시스템 오류 메시지 출력
  exit(-1);    // 프로그램 종료
}

int sameUser(char user) // 데이터베이스에 존재하는 유저인지 체크
{
  for (int i = 0; sizeof(userList); i++)
  {
    if(strcmp(userList[i].nickName, user)==0){
      return 1;
    } else {
      return -1;
    }
  }
}
void createUser(char user){

}

// gcc -o server UDPLiveServer.c
// ./server