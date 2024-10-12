#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 8080
#define BUF_SIZE 1024

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX *create_context() {
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(DTLS_client_method());
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

int main() {
    int sockfd;
    char buffer[BUF_SIZE];
    struct sockaddr_in servaddr;
    socklen_t len;

    init_openssl();
    SSL_CTX *ctx = create_context();


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));


    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    SSL *ssl = SSL_new(ctx);

    while (1) {
        printf("Enter message: ");
        fgets(buffer, BUF_SIZE, stdin);

        SSL_set_fd(ssl, sockfd);
        if (SSL_connect(ssl) <= 0) {
            fprintf(stderr, "SSL_connect error\n");
            ERR_print_errors_fp(stderr);
            continue; 
        }


        SSL_write(ssl, buffer, strlen(buffer));

        len = sizeof(servaddr);
        int n = SSL_read(ssl, buffer, BUF_SIZE);
        buffer[n] = '\0';

        printf("Echo: %s\n", buffer);
        SSL_shutdown(ssl);
    }

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    close(sockfd);
    return 0;
}






// gcc DTLSUDPEchoClient.c -o client -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto