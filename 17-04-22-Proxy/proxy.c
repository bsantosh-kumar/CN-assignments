#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#define BUF_MAX 65536
#define PORT 8080

void *proxy_in_between(void *nsfd_ptr)
{
    int nsfd = *(int *)nsfd_ptr;
    char buff[BUF_MAX];
    while (1)
    {
        recv(nsfd, buff, sizeof(buff), 0);
        printf("Message Received : \n%s\n", buff);
        fflush(stdout);
    }
    close(nsfd);
}

int set_server(int port)
{
    int sfd;
    struct sockaddr_in serv_addr;
    int serv_addrlen = sizeof(serv_addr);
    int opt = 1;

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error in socket");
        exit(0);
    }

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        perror("Error in setsockopt");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("Error in network address");
        exit(0);
    }

    if (bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Error in bind\n");
        return -1;
    }

    if (getsockname(sfd, (struct sockaddr *)&serv_addr, (socklen_t *)&serv_addrlen) == -1)
    {
        printf("Unable to get socket\n");
        return 0;
    }

    printf("Local IP Addresss : %s\n", inet_ntoa(serv_addr.sin_addr));
    printf("Local Port : %d\n\n", ntohs(serv_addr.sin_port));

    if (listen(sfd, 3) == -1)
    {
        printf("Error in listen\n");
        return -1;
    }

    return sfd;
}
int handle_request(int nsfd)
{
    struct sockaddr_in cli_addr;
    int cli_addrlen = sizeof(cli_addr);
    char *buff = (char *)calloc(1 << 16 + 1, sizeof(char));
    if (getpeername(nsfd, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_addrlen) == -1)
    {
        printf("Unable to get socket\n");
        return 0;
    }

    printf("Foreign IP Addresss : %s\n", inet_ntoa(cli_addr.sin_addr));
    printf("Foreign Port : %d\n\n", ntohs(cli_addr.sin_port));

    while (1)
    {
        int n;
        bzero(buff, 1 << 16);
        (n = recv(nsfd, buff, 1 << 16, 0)) ? printf("Message Received : \n%s\n", buff) : perror("No message received\n");
        if (n == 0)
        {
            printf("Exiting\n");
            break;
        }

        fflush(stdout);
    }
}
int main()
{
    int sfd, nsfd, valrecv;
    struct sockaddr_in cli_addr;
    int cli_addrlen = sizeof(cli_addr);

    sfd = set_server(PORT);
    int count = 0;
    while (1)
    {
        printf("count:%d\n", count++);
        char buff[BUF_MAX] = {0};

        if ((nsfd = accept(sfd, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_addrlen)) == -1)
        {
            printf("Error in accept\n");
            return -1;
        }
        handle_request(nsfd);
        fflush(stdout);
        close(nsfd);

        printf("Client was terminated\n\n\n");

        if (strcmp("exit\n", buff) == 0)
        {
            printf("Server Terminated\n");
            break;
        }
    }

    return 0;
}
