#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 8080
#define BUF_SIZE 1024

void init_openssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl()
{
    EVP_cleanup();
}

SSL_CTX *create_context()
{
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(DTLS_server_method());
    if (!ctx)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
        return 0;
    }
    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main()
{
    int sockfd;
    char buffer[BUF_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    init_openssl();
    SSL_CTX *ctx = create_context();
    configure_context(ctx);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("server is running on port number 8080...");

    SSL *ssl = SSL_new(ctx);

    while (1)
    {
        len = sizeof(cliaddr);
        int n = recvfrom(sockfd, (char *)buffer, BUF_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';

        SSL_set_fd(ssl, sockfd);
        SSL_set_session(ssl, SSL_get_session(ssl)); 
        if (SSL_accept(ssl) <= 0)
        {
            ERR_print_errors_fp(stderr);
            continue;
        }

        printf("Client: %s\n", buffer);

        SSL_write(ssl, buffer, n);
        SSL_shutdown(ssl);
    }

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    close(sockfd);
    return 0;
}

// gcc DTLSUDPEchoServer.c -o server -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
