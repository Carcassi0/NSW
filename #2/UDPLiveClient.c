// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <pthread.h>
// #include <unistd.h>     // for close()
// #include <arpa/inet.h>  // for inet_addr() and htons()
// #include <sys/socket.h> // for socket functions
// #include <netinet/in.h> // for sockaddr_in

// #define BUFSIZE 512

// char buf[BUFSIZE];
// char nickName[BUFSIZE];
// char sendBuf[BUFSIZE];
// int len;
// struct sockaddr_in peeraddr;
// socklen_t addrlen;
// int server_port = 0;
// char server_ip[INET_ADDRSTRLEN];

// void err_quit(const char *msg);
// void *sendMessage(void *arg);
// void *rcvMessage(void *arg);

// struct threadArgs
// {
//     int clntSock; // Socket descriptor for client
//     struct sockaddr_in serveraddr;
// };

// int main(int argc, char *argv[])
// {

//     int retval;

//     // Server address initialize
//     struct sockaddr_in serveraddr;
//     memset(&serveraddr, 0, sizeof(serveraddr));
//     serveraddr.sin_family = AF_INET;
//     serveraddr.sin_port = htons(server_port);
//     serveraddr.sin_addr.s_addr = inet_addr(server_ip);

//     printf("Enter server IP address: ");
//     fgets(server_ip, sizeof(server_ip), stdin);
//     server_ip[strcspn(server_ip, "\n")] = 0; // 개행 문자 제거

//     printf("Enter server port number: ");
//     scanf("%d", &server_port); // server default setting: 9000
//     while (getchar() != '\n')
//         ; // 입력 버퍼 클리어

//     if (server_port <= 0 || server_port > 65535)
//     {
//         printf("Invalid port number.\n");
//     }
//     // 클라이언트 정보
//     // 닉네임, 메시지 => 채팅창 출력(전송) 형식: "[닉네임] 채팅 메시지"
//     // UDP 서버는 클라이언트가 보낸 닉네임을 바탕으로 채팅 참여자 정보 수집

//     // IP 주소와 포트 번호 그리고 채팅 닉네임 입력 받기
//     // char server_ip[INET_ADDRSTRLEN];  // IPv4 주소의 최대 길이로 설정
//     // int server_port = 0;

//     for (;;)
//     {
//         struct threadArgs *threadArgs = (struct threadArgs *)malloc(sizeof(struct threadArgs));

//         pthread_t sendThread;
//         pthread_t recvThread; // Create client thread for send
//         int sendReturn = pthread_create(&sendThread, NULL, sendMessage, threadArgs);
//         int recvReturn = pthread_create(&recvThread, NULL, rcvMessage, threadArgs);

//         // free(threadArgs);
//     }
//     return 0;
// }

// void *sendMessage(void *arg)
// { // 발신 스레드
//     pthread_detach(pthread_self());

//     int sock = socket(AF_INET, SOCK_DGRAM, 0);
//     struct threadArgs *threadArgs = (struct threadArgs *)arg;
//     struct sockaddr_in serveraddr = threadArgs->serveraddr; // 서버 주소 가져오기
//     int send;

//     if (sock < 0)
//     {
//         err_quit("socket()");
//     }

//     while (1)
//     {
//         // 사용자 닉네임 받기
//         printf("Enter NickName: ");
//         scanf("%s", nickName);

//         len = strlen(nickName);
//         if (buf[len - 1] == '\n')
//             buf[len - 1] = '\0';
//         if (strlen(buf) == 0)
//             break;

//         while (getchar() != '\n')
//             ; // 입력 버퍼 클리어

//         // 메시지 입력
//         printf("\n[enter message] ");
//         if (fgets(buf, BUFSIZE, stdin) == NULL)
//             break;

//         len = strlen(buf);
//         if (buf[len - 1] == '\n')
//             buf[len - 1] = '\0';
//         if (strlen(buf) == 0)
//             break;

//         sprintf(sendBuf, "[%s] %s\n", nickName, buf);

//         send = sendto(sock, sendBuf, sizeof(sendBuf), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

//         memset(buf, 0, sizeof(buf));
//         memset(sendBuf, 0, sizeof(sendBuf));
//     }
//     close(sock);
//     return (NULL);
// }

// void *rcvMessage(void *arg)
// { // 수신 스레드
//     pthread_detach(pthread_self());
//     int recv;

//     while (1)
//     {
//         int sock = socket(AF_INET, SOCK_DGRAM, 0);

//         addrlen = sizeof(peeraddr);

//         recv = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);

//         // 수신된 메시지 출력
//         buf[recv] = '\0';
//         printf("[Received message] %s\n", buf);
//         memset(buf, 0, sizeof(buf));
//     }
// }

// void err_quit(const char *msg)
// {
//     perror(msg); // 시스템 오류 메시지 출력
//     exit(-1);    // 프로그램 종료
// }


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

            // 클라이언트 메뉴 선택 전송: info, stat, quit
            int send = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&threadArgs->serveraddr, sizeof(threadArgs->serveraddr));
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