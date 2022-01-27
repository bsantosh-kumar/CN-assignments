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

int sendToClient(int nsfd, char *message)
{
    send(nsfd, message, strlen(message), 0);
}
int recieveFromClient(int nsfd, char *buffer)
{
    read(nsfd, buffer, 128);
}
char *recieveAndPrintFromClient(int nsfd)
{
    char *buff = (char *)calloc(200, sizeof(char));
    recieveFromClient(nsfd, buff);
    printf("Recived from client:%s\n", buff);
    fflush(stdout);
    return buff;
}
void sendMenuToClient(int nsfd)
{
    char *message = (char *)calloc(200, sizeof(char));
    sprintf(message, "1)Echo Service\n2)Small to captial\n3)Captial to small\n4)Exit\n");
    sendToClient(nsfd, message);
    free(message);
}
int recieveMenuResponse(int nsfd)
{
    char *buffer = (char *)calloc(200, sizeof(char));
    recieveFromClient(nsfd, buffer);
    int response = atoi(buffer);
    printf("Recieved from client as menu response:'%s'\n", buffer);
    free(buffer);
    if (response >= 1 && response <= 4)
        return response;
    else
        return -1;
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

    printf("Foreign IP address is :%s\n", inet_ntoa(address.sin_addr));
    printf("Foreign port is :%d\n", (int)ntohs(address.sin_port));
}

void *handleFunc(void *arrPtr)
{
    int *arr = (int *)arrPtr;
    fflush(stdout);
    int nsFD = arr[0];
    int response = arr[1];
    {
        // printf("Handler function waiting for input from client....\n");
        // int response = 0;
        // do
        // {
        //     sendMenuToClient(nsFD);
        //     response = recieveMenuResponse(nsFD);
        //     printf("Recieved response as:%d\n", response);
        //     if (response == -1)
        //     {
        //         printf("Reponse is invalid\n");
        //         sendToClient(nsFD, "INVALID");
        //     }
        //     else
        //     {
        //         printf("Response is valid\n");
        //         sendToClient(nsFD, "VALID");
        //     }
        // } while (response == -1);
        // if (response == 4)
        // {
        //     printf("Exiting...\n");
        //     exit(EXIT_SUCCESS);
        // }
        if (fork() == 0)
        {
            char *arg[3][2] = {{"./echoService", NULL}, {"./smallToCapital", NULL}, {"./capitalToSmall", NULL}};
            fflush(stdout);
            dup2(nsFD, STDIN_FILENO);
            dup2(nsFD, STDOUT_FILENO);
            execvp(arg[response - 1][0], arg[response - 1]);
        }
    }
}

void bindAndListen(int portNo, int *sFDPtr)
{
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
    printSockName(*sFDPtr);
    printf("Listening for connections\n");
    if (listen(*sFDPtr, 10) < 0)
    {
        perror("Error in listen\n");
        exit(0);
    }
    printf("Server started listening\n");
}

void acceptHandler(int sFD, int i)
{
    int nsFD;
    struct sockaddr_in serverAddress;
    int addressLength = sizeof(serverAddress);
    if ((nsFD = accept(sFD, (struct sockaddr *)&serverAddress, &addressLength)) < 0)
    {
        perror("Error in accept\n");
        exit(0);
    }
    printPeerName(nsFD);
    pthread_t clientHandlerThread;
    int *arr = (int *)calloc(3, sizeof(int));
    arr[0] = nsFD;
    arr[1] = i;
    printf("Before creating thiread\n");
    fflush(stdout);
    pthread_create(&clientHandlerThread, NULL, handleFunc, arr);
}

int main(int argc, char const *argv[])
{
    int nsFD;
    int portNos[3] = {20500, 20501, 20502};
    int sFDs[3];
    for (int i = 0; i < 3; i++)
    {
        bindAndListen(portNos[i], &sFDs[i]);
    }
    struct sockaddr_in serverAddress;

    int addressLength = sizeof(serverAddress);
    int count = 1;
    struct pollfd pFD[3];
    for (int i = 0; i < 3; i++)
    {
        pFD[i].fd = sFDs[i];
        pFD[i].events = POLLIN;
    }

    while (1)
    {
        printf("At start of accept\n");
        fflush(stdout);
        int nFDs = 3;
        int ready = poll(pFD, nFDs, -1);
        if (ready == -1)
            exit(0);
        for (int i = 0; i < 3; i++)
        {
            if (pFD[i].fd == -1)
                continue;
            if (pFD[i].revents & POLLIN)
            {
                acceptHandler(pFD[i].fd, i + 1);
            }
        }
    }
    close(sFDs[0]);
}
