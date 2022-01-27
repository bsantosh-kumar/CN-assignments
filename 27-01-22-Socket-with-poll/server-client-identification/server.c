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
#define PORT 20500

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
int sFD;

void *handleFunc(void *nsFDPtr)
{
    int nsFD = *(int *)nsFDPtr;
    printf("Handler function waiting for input from client....\n");
    char *buff = recieveAndPrintFromClient(nsFD);
    printf("Closing socket from client\n");
    close(nsFD);
    if (strcmp(buff, "exit\n") == 0)
    {
        printf("Exiting....\n");
        close(sFD);
        exit(EXIT_SUCCESS);
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    int nsFD;
    printf("Trying to create sfd\n");
    if ((sFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating a socket\n");
        exit(0);
    }
    int True = 1;
    if (setsockopt(sFD, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(int)) < 0)
    {
        printf("Error in sock option errorno:%d\n error:%s", errno, strerror(errno));
        exit(0);
    }
    printf("sFD creation successful\n");
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(PORT);

    printf("Trying to bind the address\n");
    if (bind(sFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error in bind, error no:%d, error is: %s\n", errno, strerror(errno));
        exit(0);
    }
    printf("Bind successful\n");
    printSockName(sFD);
    printf("Listening for connections\n");
    if (listen(sFD, 10) < 0)
    {
        perror("Error in listen\n");
        exit(0);
    }
    printf("Server started listening\n");
    int addressLength = sizeof(serverAddress);
    int count = 1;
    while (1)
    {
        printf("At start of accept\n");
        fflush(stdout);
        if ((nsFD = accept(sFD, (struct sockaddr *)&serverAddress, &addressLength)) < 0)
        {
            perror("Error in accept\n");
            exit(0);
        }
        printPeerName(nsFD);
        pthread_t clientHandlerThread;
        pthread_create(&clientHandlerThread, NULL, handleFunc, &nsFD);
    }
    close(sFD);
}
