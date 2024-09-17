// // UDPEchoServer.c
// // UDP echo server 
// //
// // 과목명: 네트워크소프트웨설계
// // 아주대학교 소프트웨어학과
// // 이동 멀티미디어 융합 네트워크 연구실 (mmcn.ajou.ac.kr)
// //
// #include <winsock.h>
// #include <stdlib.h>
// #include <stdio.h>

// #define BUFSIZE 512

// // 소켓 함수 오류 출력 후 종료
// void err_quit(char *msg)
// {
// 	LPVOID lpMsgBuf;
// 	FormatMessage( 
// 		FORMAT_MESSAGE_ALLOCATE_BUFFER|
// 		FORMAT_MESSAGE_FROM_SYSTEM,
// 		NULL, WSAGetLastError(),
// 		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
// 		(LPTSTR)&lpMsgBuf, 0, NULL);
// 	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
// 	LocalFree(lpMsgBuf);
// 	exit(-1);
// }

// // 소켓 함수 오류 출력
// void err_display(char *msg)
// {
// 	LPVOID lpMsgBuf;
// 	FormatMessage( 
// 		FORMAT_MESSAGE_ALLOCATE_BUFFER|
// 		FORMAT_MESSAGE_FROM_SYSTEM,
// 		NULL, WSAGetLastError(),
// 		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
// 		(LPTSTR)&lpMsgBuf, 0, NULL);
// 	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
// 	LocalFree(lpMsgBuf);
// }

// int main(int argc, char* argv[])
// {
// 	int retval;

// 	// 윈속 초기화
// 	WSADATA wsa;
// 	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
// 		return -1;

// 	// socket()
// 	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
// 	if(sock == INVALID_SOCKET) err_quit("socket()");	
	
// 	// bind()
// 	SOCKADDR_IN serveraddr;
// 	ZeroMemory(&serveraddr, sizeof(serveraddr));
// 	serveraddr.sin_family = AF_INET;
// 	serveraddr.sin_port = htons(9000);
// 	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
// 	retval = bind(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
// 	if(retval == SOCKET_ERROR) err_quit("bind()");
	
// 	// 데이터 통신에 사용할 변수
// 	SOCKADDR_IN clientaddr;
// 	int addrlen;
// 	char buf[BUFSIZE+1];

// 	// 클라이언트와 데이터 통신
// 	while(1){
// 		// 데이터 받기
// 		addrlen = sizeof(clientaddr);
// 		retval = recvfrom(sock, buf, BUFSIZE, 0, 
// 			(SOCKADDR *)&clientaddr, &addrlen);
// 		if(retval == SOCKET_ERROR){
// 			err_display("recvfrom()");
// 			continue;
// 		}

// 		// 받은 데이터 출력
// 		buf[retval] = '\0';
// 		printf("[UDP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), 
// 			ntohs(clientaddr.sin_port), buf);

// 		// 데이터 보내기
// 		retval = sendto(sock, buf, retval, 0, 
// 			(SOCKADDR *)&clientaddr, sizeof(clientaddr));
// 		if(retval == SOCKET_ERROR){
// 			err_display("sendto()");
// 			continue;
// 		}
// 	}

// 	// closesocket()
// 	closesocket(sock);

// 	// 윈속 종료
// 	WSACleanup();
// 	return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for inet_ntoa() and htons()
#include <sys/socket.h>  // for socket functions
#include <netinet/in.h>  // for sockaddr_in

#define BUFSIZE 512

// 오류 처리 함수
void err_quit(const char *msg) {
    perror(msg);  // 시스템 오류 메시지 출력
    exit(-1);     // 프로그램 종료
}

int main(int argc, char *argv[]) {
    int retval;

    // 소켓 생성
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) err_quit("socket()");

    // 서버 주소 설정 및 바인딩
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval < 0) err_quit("bind()");

    // 통신에 사용할 변수
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];

    // 클라이언트와 데이터 통신
    while (1) {
        // 데이터 받기
        addrlen = sizeof(clientaddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);
        if (retval < 0) {
            perror("recvfrom()");
            continue;
        }

        // 받은 데이터 출력
        buf[retval] = '\0'; // 종료 문자 추가
        printf("[UDP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf);

        // 데이터 보내기 (Echo)
        retval = sendto(sock, buf, retval, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        if (retval < 0) {
            perror("sendto()");
            continue;
        }
    }

    // 소켓 닫기
    close(sock);

    return 0;
}
