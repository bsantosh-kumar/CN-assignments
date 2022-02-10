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
#define MAX_CLIENTS 100

int recieveFromClient(int nsfd, char *buffer)
{
    recv(nsfd, buffer, 128, 0);
}
char *recieveAndPrintFromClient(int nsfd)
{
    char *buff = (char *)calloc(200, sizeof(char));
    bzero(buff, sizeof(buff));
    recieveFromClient(nsfd, buff);
    printf("Recived from client:'%s'\n", buff);
    fflush(stdout);
    return buff;
}
void printSockName(int sFD)
{
    struct sockaddr_in address;
    int addLen = sizeof(address);
    if (getsockname(sFD, (struct sockaddr *)&address, &addLen) == -1)
    {
        perror("getsockname() failed\n");
        exit(0);
    }

    printf("Local IP address is :%s\n", inet_ntoa(address.sin_addr));
    printf("Local port is :%d\n", (int)ntohs(address.sin_port));
}
void printPeerName(int nsFD)
{
    struct sockaddr_in address;
    int addLen = sizeof(address);
    if (getpeername(nsFD, (struct sockaddr *)&address, &addLen) == -1)
    {
        perror("getpeername() failed\n");
        exit(0);
    }
    printf("Connected with a client:\n");
    printf("Foreign IP address is :%s\n", inet_ntoa(address.sin_addr));
    printf("Foreign port is :%d\n", (int)ntohs(address.sin_port));
    fflush(stdout);
}

void bindAndListenTcp(int portNo, int *sFDPtr)
{
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
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(portNo);

    if (bind(*sFDPtr, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error in bind, error no:%d, error is: %s\n", errno, strerror(errno));
        exit(0);
    }
    if (listen(*sFDPtr, 10) < 0)
    {
        perror("Error in listen\n");
        exit(0);
    }
    printf("Server started listening\n");
}

void acceptHandler(int sFD, int nsFDArr[], int *arrSize)
{
    int nsFD;
    struct sockaddr_in serverAddress;
    int addressLength = sizeof(serverAddress);
    if ((nsFD = accept(sFD, (struct sockaddr *)&serverAddress, &addressLength)) < 0)
    {
        perror("Error in accept\n");
        return;
    }
    printPeerName(nsFD);
    nsFDArr[(*arrSize)++] = nsFD;
}

void swap(char **a, char **b)
{
    char *temp = *a;
    *a = *b;
    *b = temp;
}

void sendToAllClients(int nsFDs[], int nsFDsSize, int exclude, int index, char *buff)
{
    char *temp = (char *)calloc(200, sizeof(char));
    bzero(temp, sizeof(temp));
    sprintf(temp, "Recieved from %d:'", index);
    fflush(stdout);
    strcat(temp, buff);
    buff = temp;
    strcat(buff, "'");
    for (int i = 0; i < nsFDsSize; i++)
    {
        if (nsFDs[i] == -1 || nsFDs[i] == exclude)
            continue;
        send(nsFDs[i], buff, 128, 0);
    }
    fflush(stdout);
    printf("Sent to all of them\n");
}
int main(int argc, char const *argv[])
{
    int sFD;
    int portNo = 50500;

    bindAndListenTcp(portNo, &sFD);

    int arrPollSize = 0;
    struct sockaddr_in serverAddress;
    struct pollfd pFDs[MAX_CLIENTS + 1];

    pFDs[arrPollSize].fd = sFD;
    pFDs[arrPollSize].events = POLLIN;
    arrPollSize++;
    int nsFDs[MAX_CLIENTS];
    int nsFDsSize = 0;
    while (1)
    {
        fflush(stdout);
        if (nsFDsSize + 1 != arrPollSize)
        {
            for (int i = arrPollSize - 1; i < nsFDsSize; i++)
            {
                pFDs[arrPollSize].fd = nsFDs[i];
                fflush(stdout);
                pFDs[arrPollSize].events = POLLIN;
                arrPollSize++;
            }
        }
        printf("Waiting to poll\n");
        int ready = poll(pFDs, arrPollSize, -1);
        if (ready == -1)
        {
            printf("Error in poll\n");
            exit(0);
        }
        if (pFDs[0].revents != 0)
        {
            if (pFDs[0].fd != -1 && pFDs[0].revents & POLLIN)
            {
                acceptHandler(pFDs[0].fd, nsFDs, &nsFDsSize);
            }
            if (pFDs[0].revents & (POLLHUP))
            {
                printf("Accept hangup\n");
                fflush(stdout);
                pFDs[0].fd = -1;
            }
            if (pFDs[0].revents & POLLERR)
            {
                printf("Got some error\n");
                fflush(stdout);
                pFDs[0].fd = -1;
            }
        }
        for (int i = 1; i < arrPollSize; i++)
        {
            if (pFDs[i].fd == -1 || pFDs[i].revents == 0)
                continue;
            printf("  fd=%d; events: %s%s%s\n", pFDs[i].fd,
                   (pFDs[i].revents & POLLIN) ? "POLLIN " : "",
                   (pFDs[i].revents & POLLHUP) ? "POLLHUP " : "",
                   (pFDs[i].revents & POLLERR) ? "POLLERR " : "");
            // if ((pFDs[i].revents & POLLHUP||POLL))
            // {
            //     int d;
            //     printf("POLLERR occured\n");
            //     scanf("%d\n", d);
            // }
            if (pFDs[i].revents & POLLIN)
            {
                fflush(stdout);
                char *buff = recieveAndPrintFromClient(pFDs[i].fd);
                if (strlen(buff) == 0)
                {
                    printf("EOF for %d,nsfd:%d\n", i, nsFDs[i - 1]);
                    pFDs[i].fd = -1;
                    nsFDs[i - 1] = -1;
                    continue;
                }
                sendToAllClients(nsFDs, nsFDsSize, pFDs[i].fd, i, buff);
                if (strcmp(buff, "exit") == 0)
                {
                    printf("%d,nsfd:%d is exiting\n", i, nsFDs[i - 1]);
                    close(nsFDs[i - 1]);
                    pFDs[i].fd = -1;
                    nsFDs[i - 1] = -1;
                }
                free(buff);
            }
            else
            {
                printf("Removing %d, nsfd:%d\n", i, nsFDs[i - 1]);
                close(nsFDs[i - 1]);
                pFDs[i].fd = -1;
                nsFDs[i - 1] = -1;
            }
        }
        fflush(stdout);
    }
    exit(0);
}
