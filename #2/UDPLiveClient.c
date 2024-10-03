#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for inet_addr() and htons()
#include <sys/socket.h>  // for socket functions
#include <netinet/in.h>  // for sockaddr_in

#define BUFSIZE 512

void *ThreadMain(void *arg); // Main program of a thread
void err_quit(const char *msg);

struct ThreadArgs {
  int clntSock; // Socket descriptor for client
};

int main(int argc, char *argv[]){


    int retval;
    struct sockaddr_in peeraddr;
    socklen_t addrlen;
    // 클라이언트 정보
    // 닉네임, 메시지 => 채팅창 출력(전송) 형식: "[닉네임] 채팅 메시지"
    // UDP 서버는 클라이언트가 보낸 닉네임을 바탕으로 채팅 참여자 정보 수집
    


    // IP 주소와 포트 번호 입력 받기
    char server_ip[INET_ADDRSTRLEN];  // IPv4 주소의 최대 길이로 설정
    int server_port = 0;

    printf("Enter server IP address: ");
    fgets(server_ip, sizeof(server_ip), stdin);
    server_ip[strcspn(server_ip, "\n")] = 0;  // 개행 문자 제거

    printf("Enter server port number: ");
    scanf("%d", &server_port); // server default setting: 9000

    // Server port 입력 후 입력 버퍼 클리어
    while (getchar() != '\n'); // 남아있는 모든 문자를 읽어버리면서 버퍼를 클리어


    if (server_port <= 0 || server_port > 65535) {
        printf("Invalid port number.\n");
        return -1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0){
         err_quit("socket()");
    }

    // Server address initialize
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(server_port);
    serveraddr.sin_addr.s_addr = inet_addr(server_ip);

    char *servPort = argv[1]; // First arg:  local port

    int servSock = SetupTCPServerSocket(servPort); 

    for(;;){
        int clntSock = AcceptTCPConnection(servSock);

        struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(
        sizeof(struct ThreadArgs));

        threadArgs->clntSock = clntSock;

        pthread_t sendThread; // Create client thread for send
        int sendReturn = pthread_create(&sendThread, NULL, sendMessage, threadArgs); // ThreadMain을 send 스레드 함수로 바꾸기

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

void *sendMessage(void *arg){ // 발신 스레드
    pthread_detach(pthread_self());

    char buf[BUFSIZE + 1];
    int len;

    while(1){
        printf("\n[enter message] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;

        len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;
    }
    

}

void *rcvMessage(void *arg){ // 수신 스레드
    pthread_detach(pthread_self());
}


void err_quit(const char *msg) {
    perror(msg);  // 시스템 오류 메시지 출력
    exit(-1);     // 프로그램 종료
}