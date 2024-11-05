#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>     // uint16_t 사용
#include <unistd.h>     // for close()
#include <arpa/inet.h>  // for inet_addr() and htons()
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> // for sockaddr_in

#define BUFSIZE 512

// 오류 처리 함수
void err_quit(const char *msg)
{
    perror(msg); // 시스템 오류 메시지 출력
    exit(-1);    // 프로그램 종료
}

int main()
{
    int retval;

    // IP 주소와 포트 번호 입력 받기
    char server_ip[INET_ADDRSTRLEN]; // IPv4 주소의 최대 길이로 설정
    int server_port = 0;

    printf("Enter server IP address: ");
    fgets(server_ip, sizeof(server_ip), stdin);
    server_ip[strcspn(server_ip, "\n")] = 0; // 개행 문자 제거

    printf("Enter server port number: ");
    scanf("%d", &server_port); // server default setting: 9000

    // Server port 입력 후 입력 버퍼 클리어
    while (getchar() != '\n')
        ; // 남아있는 모든 문자를 읽어버리면서 버퍼를 클리어

    if (server_port <= 0 || server_port > 65535)
    {
        printf("Invalid port number.\n");
        return -1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        err_quit("socket()");
    }

    // 서버 주소 세팅
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(server_port);
    serveraddr.sin_addr.s_addr = inet_addr(server_ip);

    // 변수
    struct sockaddr_in peeraddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    char sendBuf[BUFSIZE + 1 + sizeof(uint16_t)];
    char recvBuf[BUFSIZE + 1 + sizeof(uint16_t)];
    int len;
    uint16_t command;      // for user command
    uint16_t sequence = 0; // for sequence number
    struct timeval timeout;
    int length = 0;
    int quentity = 0;
    srand(time(NULL));

    // 타임아웃 설정
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    // 타임아웃을 위한 소켓 옵션 설정
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (1)
    {
        // 버퍼 초기화
        memset(buf, 0, sizeof(buf));
        memset(sendBuf, 0, sizeof(sendBuf));

        printf("Enter your message ('quit' to exit): ");
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = 0;

        uint16_t netSequence = htons(sequence);

        memcpy(sendBuf, &netSequence, sizeof(netSequence));
        memcpy(sendBuf + sizeof(netSequence), buf, strlen(buf) + 1);

        // 전송
        retval = sendto(sock, sendBuf, sizeof(command) + strlen(buf) + 1, 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        // 에러 확인
        if (retval < 0)
        {
            perror("sendto()");
            continue;
        }

        // 전송 후 데이터 수신 위해 버퍼 초기화
        memset(recvBuf, 0, sizeof(recvBuf));

        // 데이터 수신
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, recvBuf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);

        if (retval < 0) // 타임아웃 발생
        {
            while (1)
            {

                // 이전 버퍼에 있던 내용 그대로 다시 발신
                sendto(sock, sendBuf, sizeof(command) + strlen(buf) + 1, 0,
                       (struct sockaddr *)&serveraddr, sizeof(serveraddr));

                printf("Retransmit due to timeout!!!\n"); // 성공적으로 발신했을 때

                memset(recvBuf, 0, sizeof(recvBuf));

                retval = recvfrom(sock, recvBuf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);

                if (retval > 0)
                {
                    // 타임아웃이 아니라면, 수신된 메시지 출력
                    recvBuf[retval] = '\0';
                    printf("[Received message] %s\n", recvBuf);

                    sequence += strlen(buf);

                    // 다음 메시지 발신을 위한 버퍼 초기화
                    memset(buf, 0, sizeof(buf));
                    memset(sendBuf, 0, sizeof(sendBuf));
                    break;
                }
                else
                {
                    continue; // 다시 로스 발생
                }
            }

            if (strcmp(recvBuf, "quit") == 0 || strcmp(recvBuf, "QUIT") == 0)
            {
                break;
            }
        }
        else
        { // 정상 수신
            recvBuf[retval] = '\0';
            printf("[Received message] %s\n", recvBuf);

            sequence += strlen(buf);

            if (strcmp(recvBuf, "quit") == 0 || strcmp(recvBuf, "QUIT") == 0)
            {
                break;
            }
            else
            {
                continue;
            }
        }
    }

    // 소켓 종료
    close(sock);
    printf("Socket Close\n");

    return 0;
}

// Test
// gcc -o mclient ARQClient.c
// ./mclient
