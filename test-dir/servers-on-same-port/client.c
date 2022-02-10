#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
int sendToServer(int nsfd, char *message)
{
    send(nsfd, message, strlen(message), 0);
}
int recieveFromServer(int nsfd, char *buffer)
{
    read(nsfd, buffer, 128);
}
char *recieveAndPrintFromServer(int nsfd)
{
    char *buff = (char *)calloc(200, sizeof(char));
    recieveFromServer(nsfd, buff);
    printf("Recived from server:\n'%s'\n", buff);
    fflush(stdout);
    return buff;
}
int recieveMenuFromServer(int nsfd)
{
    recieveAndPrintFromServer(nsfd);
}
int sendMenuResponse(int nsfd)
{
    char *response = (char *)calloc(200, sizeof(char));
    scanf("%s", response);
    printf("Sending %s\n", response);
    sendToServer(nsfd, response);
    int returnVal = atoi(response);
    if (returnVal >= 1 && returnVal <= 3)
        return returnVal;
    else
        return -1;
}
char *sendInputToServer(int nsfd)
{
    printf("Enter string to send to server:\n");
    char *buff = (char *)calloc(200, sizeof(char));
    scanf("%s", buff);
    buff[strlen(buff)] = '\n';
    sendToServer(nsfd, buff);
    printf("Sent '%s' to server\n", buff);
    return buff;
}
void recieveServiceAnsFromServer(int nsfd)
{
    recieveAndPrintFromServer(nsfd);
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

void *readFunc(void *nsFDPtr)
{
    int nsFD = *(int *)nsFDPtr;
    char *buff = (char *)calloc(200, sizeof(char));
    while (1)
    {
        bzero(buff, 200);
        read(nsFD, buff, 128);
        printf("\n%s\n", buff);
    }
}

void *writeFunc(void *nsFDPtr)
{
    int nsFD = *(int *)nsFDPtr;
    printf("nsFD:%d\n", nsFD);
    char *buff = (char *)calloc(200, sizeof(char));
    while (1)
    {
        bzero(buff, 200);
        write(1, "Write to send to others:", 25);
        size_t buffLen = 128;
        getline(&buff, &buffLen, stdin);
        buff[strlen(buff) - 1] = '\0';
        // scanf("%s", buff);
        write(nsFD, buff, 128);
        if (strcmp(buff, "exit") == 0)
        {
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    int SERVERPORT = atoi(argv[1]);
    int sFD;
    int nsFD;
    // printf("Trying to create sfd\n");
    if ((sFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating a socket\n");
        exit(0);
    }
    printf("sFD:%d\n", sFD);
    // printf("sFD creation successful\n");
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVERPORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    // serverAddress.sin_addr.s_addr = INADDR_LOOPBACK;
    // printf("Trying to connect\n");
    if (connect(sFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Connection error\n");
        exit(0);
    }
}
