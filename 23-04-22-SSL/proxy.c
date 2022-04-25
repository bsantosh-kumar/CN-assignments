
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int tcp_server_init(char *local_ip, char *port)
{
    int sfd;
    struct sockaddr_in serv_addr;
    printf("tcp init local ip :%s local port:%s\n", local_ip, port);
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Unable to create socket\n");
        return -1;
    }
    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        perror("Error in setsockopt");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    int port_no = atoi(port);
    printf("port no:%d\n", port_no);

    serv_addr.sin_port = htons(port_no);

    if (inet_pton(AF_INET, local_ip, &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid network address or address family\n");
        return 0;
    }

    if (bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Error in bind\n");
        return -1;
    }
    if (listen(sfd, 3) < 0)
    {
        perror("Error in listen\n");
    }
    printf("tcp sfd:%d\n", sfd);
    return sfd;
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "mycert.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "mycert.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}
char *find_url(char *buff)
{
    char *temp = (char *)calloc(65536, sizeof(char));
    strcpy(temp, buff);
    char *url = strtok(temp, " ");
    url = strtok(NULL, " ");
    free(temp);
    return url;
}
struct sockaddr_in find_addr(char *url)
{
    struct sockaddr_in test;
    return test;
}
void *proxy_func(void *arg)
{
    SSL *ssl = (SSL *)arg;
    char *buff = (char *)calloc(65336, sizeof(char));
    int read_bytes = 0;
    if ((read_bytes = SSL_read(ssl, buff, 65335)) == 0)
    {
        free(buff);
        return NULL;
    }
    printf("%s\n", buff);
    char *url = find_url(buff);
    printf("url:%s\n", url);

    // struct sockaddr_in url_addr = find_addr(url);
    while (1)
    {
        if ((read_bytes = SSL_read(ssl, buff, 65335)) == 0)
            break;
        printf("%s\n", buff);
    }
    int nsfd = SSL_get_fd(ssl);
    close(nsfd);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    free(buff);
}
void handle_client(int client, SSL_CTX *ctx, pthread_t all_threads[], int *n_thread)
{
    SSL *ssl;
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client);
    if (SSL_accept(ssl) <= 0)
    {
        printf("SSL_accept failed\n");
        ERR_print_errors_fp(stderr);
    }
    else
    {
        pthread_t *tid = &all_threads[(*n_thread)++];
        pthread_create(tid, NULL, proxy_func, ssl);
    }
}
int main(int argc, char **argv)
{
    int sock;
    SSL_CTX *ctx;

    ctx = create_context();

    configure_context(ctx);

    sock = tcp_server_init("127.0.0.1", "8080");
    pthread_t all_threads[100];
    int n_thread = 0;
    /* Handle connections */
    while (1)
    {
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        char reply[1024];
        printf("BEfore accept\n");
        int client = accept(sock, (struct sockaddr *)&addr, &len);
        printf("After accept\n");
        fflush(stdout);
        if (client < 0)
        {
            perror("Unable to accept");
            continue;
        }
        handle_client(client, ctx, all_threads, &n_thread);
    }

    close(sock);
    SSL_CTX_free(ctx);
}