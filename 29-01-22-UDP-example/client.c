#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <stdbool.h>
#include <fcntl.h>
#define SERVERPORT 20500

struct ipAddStruct_t
{
    char *ipAdd;
    char *portNo;
};
typedef struct ipAddStruct_t ipAddStruct;

int sendToServer(int nsfd, char *message)
{
    send(nsfd, message, strlen(message), 0);
}
int recieveFromServer(int nsfd, char *buffer)
{
    read(nsfd, buffer, 128);
}
void recieveAndPrintFromServer(int nsfd)
{
    char *buff = (char *)calloc(200, sizeof(char));
    recieveFromServer(nsfd, buff);
    printf("Recived from server:\n%s\n", buff);
    fflush(stdout);
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
void sendInputToServer(int nsfd)
{
    printf("Enter string to send to server:\n");
    char *buff = (char *)calloc(200, sizeof(char));
    scanf("%s", buff);
    buff[strlen(buff)] = '\n';
    sendToServer(nsfd, buff);
    printf("Sent '%s' to server\n", buff);
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

char *getLocalUdpAdd()
{
    int udpPortNo = 20502;
    char udpPortNoStr[100];
    sprintf(udpPortNoStr, "%d", udpPortNo);
    char udpIPStr[100];
    strcat(udpIPStr, "127.0.0.1");
    char *udpAddress = (char *)calloc(200, sizeof(char));
    bzero(udpAddress, 200);
    strcat(udpAddress, udpIPStr);
    strcat(udpAddress, ":");
    strcat(udpAddress, udpPortNoStr);
    return udpAddress;
}

void divideIpAdd(char *ipAdd, ipAddStruct *ans)
{
    ans->ipAdd = strtok(ipAdd, ":");
    ans->portNo = strtok(NULL, ":");
}

void printIpAddStruct(ipAddStruct *ipAdd)
{
    printf("add:%s port:%s\n", ipAdd->ipAdd, ipAdd->portNo);
}

struct sockaddr_in getSockAdd(ipAddStruct *ipAdd)
{
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(ipAdd->portNo));
    serverAddr.sin_addr.s_addr = inet_addr(ipAdd->ipAdd);
    return serverAddr;
}

void bindUdp(ipAddStruct *ipAdd, int *sFDPtr)
{
    if ((*sFDPtr = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("Error in udp socket creation\n");
        exit(0);
    }
    struct sockaddr_in serverAddr = getSockAdd(ipAdd);
    if ((bind(*sFDPtr, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) == -1)
    {
        printf("Error in udp bind\n");
        printf("error:%s errorno:%d\n", strerror(errno), errno);

        exit(EXIT_FAILURE);
    }
    printf("Bind successful\n");
}

void sendUdp(ipAddStruct *ipAdd, int udpSFD, char *message)
{
    char buff[200];
    strcat(buff, message);
    struct sockaddr_in sendSockAdd = getSockAdd(ipAdd);
    if ((sendto(udpSFD, buff, 128, 0, (struct sockaddr *)&sendSockAdd, sizeof((sendSockAdd)))) == -1)
    {
        printf("error in send udp\n");
    }
    else
    {
        printf("Sending %s is successful from UDP\n", message);
    }
}

char *recieveUdp(ipAddStruct *ipAdd, int udpSFD)
{
    char *buff = (char *)calloc(200, sizeof(char));
    bzero(buff, sizeof(buff));
    struct sockaddr_in sendSockAdd = getSockAdd(ipAdd);
    int sockAddLen = sizeof(sendSockAdd);
    printf("Trying to recieve\n");
    if ((recvfrom(udpSFD, buff, 128, 0, (struct sockaddr *)&sendSockAdd, (socklen_t *)&sockAddLen)) == -1)
    {
        printf("error in recieve udp\n");
    }
    else
    {
        printf("Recieved %s is successful from UDO\n", buff);
    }
}

int main(int argc, char *argv[])
{
    int sFD;
    int nsFD;
    printf("Trying to create sfd\n");
    if ((sFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating a socket\n");
        exit(0);
    }
    printf("sFD creation successful\n");
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVERPORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    // serverAddress.sin_addr.s_addr = INADDR_LOOPBACK;
    printf("Trying to connect\n");
    if (connect(sFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Connection error\n");
        exit(0);
    }
    printf("Connection successful\n");
    printSockName(sFD);
    printPeerName(sFD);
    nsFD = sFD;
    char *localUdpAdd = getLocalUdpAdd();
    send(nsFD, localUdpAdd, 128, 0);
    printf("Sent %s\n", localUdpAdd);
    char *foreignUdpAdd = (char *)calloc(200, sizeof(char));
    read(nsFD, foreignUdpAdd, 128);
    close(sFD);
    printf("local udp address:%sForeign udp address:%s\n", localUdpAdd, foreignUdpAdd);
    ipAddStruct foreignIpAdd;
    divideIpAdd(foreignUdpAdd, &foreignIpAdd);
    ipAddStruct localIpAdd;
    divideIpAdd(localUdpAdd, &localIpAdd);
    int udpSFD;
    sem_t *waitSem = sem_open("wait", O_CREAT, 0666, 0);
    bindUdp(&localIpAdd, &udpSFD);
    sem_post(waitSem);
    recieveUdp(&foreignIpAdd, udpSFD);
    sendUdp(&foreignIpAdd, udpSFD, "Hi from client");

    printf("Exiting...\n");
}