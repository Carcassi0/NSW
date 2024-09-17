// // UDPEchoClient.c
// // UDP echo client
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
	
// 	// ���� �ּ� ����ü �ʱ�ȭ
// 	SOCKADDR_IN serveraddr;
// 	ZeroMemory(&serveraddr, sizeof(serveraddr));
// 	serveraddr.sin_family = AF_INET;
// 	serveraddr.sin_port = htons(9000);
// 	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

// 	// ������ ��ſ� ����� ����
// 	SOCKADDR_IN peeraddr;
// 	int addrlen;
// 	char buf[BUFSIZE+1];
// 	int len;

// 	// ������ ������ ���
// 	while(1){
// 		// ������ �Է�
// 		printf("\n[���� ������] ");
// 		if(fgets(buf, BUFSIZE+1, stdin) == NULL)
// 			break;

// 		// '\n' ���� ����
// 		len = strlen(buf);
// 		if(buf[len-1] == '\n')
// 			buf[len-1] = '\0';
// 		if(strlen(buf) == 0)
// 			break;

// 		// ������ ������
// 		retval = sendto(sock, buf, strlen(buf), 0, 
// 			(SOCKADDR *)&serveraddr, sizeof(serveraddr));
// 		if(retval == SOCKET_ERROR){
// 			err_display("sendto()");
// 			continue;
// 		}
// 		printf("[UDP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);

// 		// ������ �ޱ�
// 		addrlen = sizeof(peeraddr);
// 		retval = recvfrom(sock, buf, BUFSIZE, 0, 
// 			(SOCKADDR *)&peeraddr, &addrlen);
// 		if(retval == SOCKET_ERROR){
// 			err_display("recvfrom()");
// 			continue;
// 		}

// 		// �۽����� IP �ּ� üũ
// 		if(memcmp(&peeraddr, &serveraddr, sizeof(peeraddr))){
// 			printf("[����] �߸��� �������Դϴ�!\n");
// 			continue;
// 		}

// 		// ���� ������ ���
// 		buf[retval] = '\0';
// 		printf("[UDP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
// 		printf("[���� ������] %s\n", buf);
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
#include <arpa/inet.h>   // for inet_addr() and htons()
#include <sys/socket.h>  // for socket functions
#include <netinet/in.h>  // for sockaddr_in

#define BUFSIZE 512

// ���� ó�� �Լ�
void err_quit(const char *msg) {
    perror(msg);   // �ý��� ���� �޽��� ���
    exit(-1);      // ���α׷� ����
}

int main(int argc, char *argv[]) {
    int retval;

    // ���� ����
    int sock = socket(AF_INET, SOCK_DGRAM, 0); // AF_INET: Domaine, SOCK_DGRAM: UDP, 0: Protocol
    if (sock < 0) err_quit("socket()");

    // ���� �ּ� ����
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // ��ſ� ����� ������
    struct sockaddr_in peeraddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    int len;

    // �޽��� ���� ����
    while (1) {
        // ����� �Է� �ޱ�
        printf("\n[enter message] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;

        // '\n' ���� ����
        len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        // �޽��� ����
        retval = sendto(sock, buf, strlen(buf), 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (retval < 0) {
            perror("sendto()");
            continue;
        }
        printf("[UDP Client] sent %dbyte.\n", retval);

        // �����κ��� �޽��� ����
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);
        if (retval < 0) {
            perror("recvfrom()");
            continue;
        }
        
        // �۽����� IP �ּ� Ȯ��
        if (memcmp(&peeraddr, &serveraddr, sizeof(peeraddr))) {
            printf("[error] Wrong receiver!\n");
            continue;
        }

        // ���ŵ� �޽��� ���
        buf[retval] = '\0';
        printf("[UDP Client] Received %dbyte.\n", retval);
        printf("[Received message] %s\n", buf);
    }

    // ���� ����
    close(sock);

    return 0;
}
