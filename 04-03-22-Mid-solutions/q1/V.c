#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/poll.h>
#include <time.h>
#define SA struct sockaddr

struct sockaddr_in getServAddr(char *servIP, int servPort)
{
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(servIP);
    serv_addr.sin_port = htons(servPort);
    return serv_addr;
}

int connectTcp(char *servIP, int servPort)
{
    int sFD;
    if ((sFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in socket creation\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv_addr = getServAddr(servIP, servPort);
    int serv_len = sizeof(serv_addr);
    if ((connect(sFD, (SA *)&serv_addr, serv_len)) < 0)
    {
        perror("Error in connect\n");
        exit(1);
    }
    return sFD;
}

int main(int argc, char *argv[])
{
    int foreign_port;
    if (argc != 2)
        exit(EXIT_FAILURE);
    foreign_port = atoi(argv[1]);
    int sfd = connectTcp("127.0.0.1", foreign_port);
    char buff[200];
    bzero(buff, sizeof(buff));
    read(sfd, buff, 128);
    printf("%s\n", buff);
    fflush(stdout);
    bzero(buff, sizeof(buff));
    while (1)
    {
        read(sfd, buff, 128);
        if (strcmp(buff, "exit") == 0)
            break;
        printf("%s\n", buff);
        fflush(stdout);
        bzero(buff, sizeof(buff));
        scanf("%s", buff);
        write(sfd, buff, 128);
    }
    close(sfd);
}