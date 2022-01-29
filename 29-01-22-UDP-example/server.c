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
#include <fcntl.h>
#include <semaphore.h>
#define PROGRAMNAME "P.cpp"
#define CLIENTOUTPUT "clientOutput.txt"
#define ACTUALINPUT "input.txt"
#define ACTUALOUTPUT "output.txt"

struct ipAddStruct_t
{
    char *ipAdd;
    char *portNo;
};
typedef struct ipAddStruct_t ipAddStruct;

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
}

int openFileToWrite(char *fileName)
{
    FILE *fp = fopen(fileName, "w");
    if (fp != NULL)
        fclose(fp);
    return open(fileName, O_WRONLY | O_CREAT);
}

void recieveFile(int nsFD, int fdProgram)
{
    char buff[200];
    memset(buff, '\0', sizeof(buff) * sizeof(char));
    read(nsFD, buff, 128);
    if (strcmp(buff, "STARTING") != 0)
    {
        return;
    }
    memset(buff, '\0', sizeof(buff) * sizeof(char));
    read(nsFD, buff, 128);
    if (strcmp(buff, "ENDING") == 0)
    {
        close(fdProgram);
        return;
    }
    bool flag = false;
    while (1)
    {
        char temp[200];
        bzero(temp, sizeof(temp));
        strcpy(temp, buff);
        memset(buff, '\0', sizeof(buff) * sizeof(char));
        read(nsFD, buff, 128);
        if (strlen(buff) == 0)
            break;
        if (strcmp(buff, "ENDING") == 0)
        {
            int len = strlen(temp);
            for (int i = len; i < 128; i++)
            {
                temp[i] = ' ';
            }
            flag = true;
        }
        write(fdProgram, temp, 128);
        if (flag == true)
            break;
    }
    close(fdProgram);
}

int compileProgram()
{
    char command[200];
    bzero(command, sizeof(command));
    strcat(command, "g++ -o P.exe ");
    strcat(command, PROGRAMNAME);
    int returnVal = system(command);
    if (returnVal == 0)
        printf("Compilation Successful\n");
    else
        printf("Compilation Unsuccessful %d\n", returnVal);
    return returnVal;
}

void executeProgram()
{
    if (fork() != 0)
    {
        wait(NULL);
    }
    else
    {
        int inputFD = open(ACTUALINPUT, O_RDONLY);
        int outputFD = openFileToWrite(CLIENTOUTPUT);
        dup2(inputFD, STDIN_FILENO);
        dup2(outputFD, STDOUT_FILENO);
        char *args[] = {"./P.exe", NULL};
        execvp(args[0], args);
    }
}

bool compareFiles()
{
    int actualOutput = open(ACTUALOUTPUT, O_RDONLY);
    int clientOutput = open(CLIENTOUTPUT, O_RDONLY);
    while (1)
    {
        char clientBuff[200];
        char actualBuff[200];
        int clientRead = read(clientOutput, clientBuff, 1);
        int actualRead = read(actualOutput, actualBuff, 1);
        if (clientRead == 0 && actualRead == 0)
        {
            close(actualOutput);
            close(clientOutput);
            return true;
        }
        if (clientRead == 0 || actualRead == 0)
        {
            close(actualOutput);
            close(clientOutput);
            return false;
        }
        if (clientBuff[0] != actualBuff[0])
        {
            close(actualOutput);
            close(clientOutput);
            return false;
        }
    }
    close(actualOutput);
    close(clientOutput);
    return false;
}
char *getLocalUdpAdd()
{
    int udpPortNo = 20501;
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
    if ((recvfrom(udpSFD, buff, 128, 0, (struct sockaddr *)&sendSockAdd, (socklen_t *)&sockAddLen)) == -1)
    {
        printf("error in recieve udp\n");
    }
    else
    {
        printf("Recieved %s is successful from UDP\n", buff);
    }
}

int main(int argc, char const *argv[])
{
    int nsFD;
    int tcpPortNo = 20500;
    int sFD;
    // for (int i = 0; i < 3; i++)
    // {
    //     bindAndListenTcp(portNos[i], &sFDs[i]);
    // }
    bindAndListenTcp(tcpPortNo, &sFD);
    struct sockaddr_in serverAddress;

    int addressLength = sizeof(serverAddress);
    printf("Waiting to accept..\n");
    if ((nsFD = accept(sFD, (struct sockaddr *)&serverAddress, (socklen_t *)&addressLength)) < 0)
    {
        perror("Error in accept\n");
        exit(0);
    }
    printf("Accepted a connection\n");
    printf("\n");
    char foreignUdpAdd[200] = {0};
    // memset(foreignUdpAdd, '\0', sizeof(foreignUdpAdd) * sizeof(char));
    read(nsFD, foreignUdpAdd, 128);
    printf("Read %s\n", foreignUdpAdd);
    char *localUdpAdd = getLocalUdpAdd();
    printf("localUdpAdd:%s\n", localUdpAdd);
    send(nsFD, localUdpAdd, 128, 0);

    close(nsFD);
    printf("local udp address:%s\nForeign udp address:%s\n", localUdpAdd, foreignUdpAdd);
    close(sFD);
    ipAddStruct foreignIpAdd;
    divideIpAdd(foreignUdpAdd, &foreignIpAdd);
    ipAddStruct localIpAdd;
    divideIpAdd(localUdpAdd, &localIpAdd);
    int udpSFD;
    bindUdp(&localIpAdd, &udpSFD);
    sem_t *waitSem = sem_open("wait", O_CREAT, 0666, 0);
    sem_wait(waitSem);
    sendUdp(&foreignIpAdd, udpSFD, "Hi from server");
    recieveUdp(&foreignIpAdd, udpSFD);
    printf("Exiting...\n");
}
