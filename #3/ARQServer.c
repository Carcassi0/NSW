#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>     // for close()
#include <arpa/inet.h>  // for inet_ntoa() and htons()
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> // for sockaddr_in

#define BUFSIZE 512

// 오류 처리 함수
void err_quit(const char *msg)
{
    perror(msg); // 시스템 오류 메시지 출력
    exit(-1);    // 프로그램 종료
}

int main(int argc, char *argv[])
{
    int retval;
    int alert;

    // Socket Initialize
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        err_quit("socket()");
    }

    // 서버 주소 세팅과 바인딩
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval < 0)
        err_quit("bind()");

    // 변수
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char alertBuf[BUFSIZE];
    char buf[BUFSIZE + 1];
    uint16_t sequence = 0;
    int len;
    int n1 = 0;
    int n2 = 0; // 재전송한 메시지 총 수
    int n3 = 0; 
    int ratio = 0; // n2/n3  <= double 형으로 바꿔야 할 수도..?
    double defalutNum = 0.5; // 값 변경하면서 보고서 작성(최소 3개)

    while (1)
    {
        // 데이터 수신
        memset(buf, 0, sizeof(buf));
        memset(alertBuf, 0, sizeof(alertBuf));

        addrlen = sizeof(clientaddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);


        // 에러 처리
        if (retval < 0)
        {
            perror("recvfrom()");
            continue;
        }

        // 커맨드와 메시지를 분리
        // 시퀀스 넘버 추출
        memcpy(&sequence, buf, sizeof(uint16_t));

        // 재전송된 메시지의 경우 sequence 넘버 증가시키지 않음
        if (sequence != ntohs(sequence))
        {
            sequence += ntohs(sequence);
        }

        // 메시지 추출
        char message[BUFSIZE - sizeof(uint16_t) + 1];
        memcpy(message, buf + sizeof(uint16_t), retval - sizeof(uint16_t));
        message[retval - sizeof(uint16_t)] = '\0'; // 널 종료 문자 추가

        // 메시지 수신 시 출력
        printf("Client(%s:%d)로 부터 메시지:[%hu][%s]\n",
               inet_ntoa(clientaddr.sin_addr),
               ntohs(clientaddr.sin_port),
               sequence,
               message);

        // timeout 상황 적용을 위한 로스 확률 생성
        srand(time(NULL));
        double random_num = (double)rand() / RAND_MAX;

        if (random_num < defalutNum)
        {
            if (strcmp(message, "quit") == 0 || strcmp(message, "QUIT"))
            {
                if (sendto(sock, buf, retval, 0, (struct sockaddr *)&clientaddr, addrlen) > 0)
                {
                    printf("Client(%s:%d)종료\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
                    printf("통계정보:p=%f,N1=%d,N2=%d,N3=%d,R=%d\n",defalutNum, n1, n2, n1+n2, n2%n3);
                    break;
                }
            }
            // 만약 로스가 나지 않으면 그대로  echo
            if (sendto(sock, buf, retval, 0, (struct sockaddr *)&clientaddr, addrlen) > 0)
            {
                printf("메시지 echo 완료");
            }
        }
        else
        {
            printf("메시지 손실 처리");
            continue;
            ++n2;
        }

        // 에러 처리
        if (retval < 0)
        {
            perror("sendto()");
            continue;
        }
    }

    close(sock);
    printf("Socket Close\n");

    return 0;
}


// Test
// gcc -o server ARQServer.c
// ./server
