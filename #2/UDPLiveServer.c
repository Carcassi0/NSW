#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for inet_addr() and htons()
#include <sys/socket.h>  // for socket functions
#include <netinet/in.h>  // for sockaddr_in

void *ThreadMain(void *arg); // Main program of a thread
void err_quit(const char *msg);

struct ThreadArgs {
  int clntSock; // Socket descriptor for client
};

int main(int argc, char *argv[]){


    int retval;
    int listenSock = socket(PF_INET, SOCK_DGRAM, 0);

    if(listenSock < 0){
         err_quit("socket()");
    }

    // Server address initialize and binding
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(listenSock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval < 0) err_quit("bind()");

    for(;;){

    }
    return 0;
}


void *ThreadMain(void *threadArgs) {
  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());

  // Extract socket file descriptor from argument
  int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
  free(threadArgs); // Deallocate memory for argument

  HandleTCPClient(clntSock);

  return (NULL);
}


void err_quit(const char *msg) {
    perror(msg);  // 시스템 오류 메시지 출력
    exit(-1);     // 프로그램 종료
}