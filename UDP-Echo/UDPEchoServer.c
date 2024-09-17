// // UDPEchoServer.c
// // UDP echo server 
// //
// // �����: ��Ʈ��ũ����Ʈ������
// // ���ִ��б� ����Ʈ�����а�
// // �̵� ��Ƽ�̵�� ���� ��Ʈ��ũ ������ (mmcn.ajou.ac.kr)
// //
// #include <winsock.h>
// #include <stdlib.h>
// #include <stdio.h>

// #define BUFSIZE 512

// // ���� �Լ� ���� ��� �� ����
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

// // ���� �Լ� ���� ���
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

// 	// ���� �ʱ�ȭ
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
	
// 	// ������ ��ſ� ����� ����
// 	SOCKADDR_IN clientaddr;
// 	int addrlen;
// 	char buf[BUFSIZE+1];

// 	// Ŭ���̾�Ʈ�� ������ ���
// 	while(1){
// 		// ������ �ޱ�
// 		addrlen = sizeof(clientaddr);
// 		retval = recvfrom(sock, buf, BUFSIZE, 0, 
// 			(SOCKADDR *)&clientaddr, &addrlen);
// 		if(retval == SOCKET_ERROR){
// 			err_display("recvfrom()");
// 			continue;
// 		}

// 		// ���� ������ ���
// 		buf[retval] = '\0';
// 		printf("[UDP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), 
// 			ntohs(clientaddr.sin_port), buf);

// 		// ������ ������
// 		retval = sendto(sock, buf, retval, 0, 
// 			(SOCKADDR *)&clientaddr, sizeof(clientaddr));
// 		if(retval == SOCKET_ERROR){
// 			err_display("sendto()");
// 			continue;
// 		}
// 	}

// 	// closesocket()
// 	closesocket(sock);

// 	// ���� ����
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

// ���� ó�� �Լ�
void err_quit(const char *msg) {
    perror(msg);  // �ý��� ���� �޽��� ���
    exit(-1);     // ���α׷� ����
}

int main(int argc, char *argv[]) {
    int retval;

    // ���� ����
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) err_quit("socket()");

    // ���� �ּ� ���� �� ���ε�
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval < 0) err_quit("bind()");

    // ��ſ� ����� ����
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];

    // Ŭ���̾�Ʈ�� ������ ���
    while (1) {
        // ������ �ޱ�
        addrlen = sizeof(clientaddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);
        if (retval < 0) {
            perror("recvfrom()");
            continue;
        }

        // ���� ������ ���
        buf[retval] = '\0'; // ���� ���� �߰�
        printf("[UDP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf);

        // ������ ������ (Echo)
        retval = sendto(sock, buf, retval, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        if (retval < 0) {
            perror("sendto()");
            continue;
        }
    }

    // ���� �ݱ�
    close(sock);

    return 0;
}
