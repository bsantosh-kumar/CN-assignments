#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/poll.h>
#define SA struct sockaddr
int bindAndListenTcp(int portNo)
{
    int sFD = 0;
    int *sFDPtr = &sFD;
    printf("Trying to create sfd\n");
    if ((*sFDPtr = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating a socket\n");
        exit(0);
    }
    int True = 1;
    if (setsockopt(*sFDPtr, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(int)) < 0)
    {
        printf("Error in sock option errorno:%d\n error:%s", errno, strerror(errno));
        exit(0);
    }
    printf("sFD creation successful\n");
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(portNo);

    printf("Trying to bind the address\n");
    if (bind(*sFDPtr, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error in bind, error no:%d, error is: %s\n", errno, strerror(errno));
        exit(0);
    }
    printf("Bind successful\n");
    printf("Listening for connections\n");
    if (listen(*sFDPtr, 10) < 0)
    {
        perror("Error in listen\n");
        exit(0);
    }
    printf("Server started listening\n");
    return sFD;
}

void handle_echo(int sFD)
{
    int nsFD = 0;
    struct sockaddr_in tcli_addr;
    int tcli_len = sizeof(tcli_addr);
    if ((nsFD = accept(sFD, (SA *)&tcli_addr, (socklen_t *)&tcli_len)) < 0)
    {
        perror("Error in accept\n");
        return;
    }
    printf("Accepted a client\n");
    char *buff = (char *)calloc(200, sizeof(char));
    while (1)
    {
        bzero(buff, 200);
        read(nsFD, buff, 128);
        if (strlen(buff) == 0)
        {
            break;
        }
        if (strcmp(buff, "exit") == 0)
        {
            break;
        }
        write(nsFD, buff, 128);
    }
    printf("Client exiting\n");
    return;
}

int main(int argc, char *argv[])
{
    int portNo = 50500;
    if (argc == 2)
    {
        portNo = atoi(argv[1]);
    }
    int sFD = bindAndListenTcp(portNo);
    struct pollfd pFDs[1];
    pFDs[0].fd = sFD;
    pFDs[0].events = POLLIN;
    while (1)
    {
        poll(pFDs, 1, -1);
        for (int i = 0; i < 1; i++)
        {
            if (pFDs[i].revents == 0)
                continue;
            else if (pFDs[i].revents & POLLIN)
            {
                handle_echo(sFD);
            }
            else if (pFDs[i].revents & (POLLHUP | POLLERR))
            {
                printf("There is some error in accepting connections\n");
                continue;
            }
        }
    }
    exit(0);
}
