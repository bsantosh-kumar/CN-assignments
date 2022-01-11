#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#define PACKET_SIZE 100
int myCeil(int a, int b)
{
    int ans = 0;
    while (a >= b)
    {
        ans++;
        a -= b;
    }
    if (a != 0)
        ans++;
    return ans;
}

int findNoOfPackets(char *currMessage)
{
    int messageSize = strlen(currMessage);
    int noOfPackets = myCeil(messageSize, PACKET_SIZE);

    currMessage = realloc(currMessage, noOfPackets * PACKET_SIZE * sizeof(char));
    return noOfPackets;
}
void writeToPipe(int wfd, char *currMessage, char *startingMessage, char *endingMessage)
{ //wfd -> write fd
    // printf("%d Writing to %d\n", getpid(), wfd);
    write(wfd, startingMessage, PACKET_SIZE);
    int noOfPackets = findNoOfPackets(currMessage);
    // printf("Came to write\n");
    write(wfd, &noOfPackets, sizeof(int));
    printf("Sending %d packets\n with content '%s'", noOfPackets, currMessage);
    fflush(stdout);
    for (int i = 0; i < noOfPackets; i++)
    {
        write(wfd, currMessage + i * PACKET_SIZE, PACKET_SIZE);
    }
    write(wfd, endingMessage, PACKET_SIZE);
}
char *readFromPipe(int rfd, char **currMessagePtr)
{
    /*
        rfd -> read fd
        return 1 if read is done, 0 if closingConnection message is sent

    */
    // printf("%d Reading from %d\n", getpid(), rfd);
    char *readStartingMessage = (char *)calloc(PACKET_SIZE, sizeof(char));
    read(rfd, readStartingMessage, PACKET_SIZE);
    int noOfPackets = 0;
    read(rfd, &noOfPackets, sizeof(int));
    printf("Recieving %d packets\n", noOfPackets);
    fflush(stdout);
    (*currMessagePtr) = (char *)(malloc(noOfPackets * PACKET_SIZE * sizeof(char)));
    for (int i = 0; i < noOfPackets; i++)
    {
        read(rfd, (*currMessagePtr) + i * PACKET_SIZE, PACKET_SIZE);
    }
    char *readEndingMessage = calloc(PACKET_SIZE, sizeof(char));
    read(rfd, readEndingMessage, PACKET_SIZE);
    printf("Starting message is: '%s'\n", readStartingMessage);
    return readStartingMessage;
}
void *recieveFunction(void *voidClientNo)
{
    int clientNo = *((int *)voidClientNo);
    char *recieveFIFOStr = "send"; //send wrt to the server
    mkfifo(recieveFIFOStr, 0666);
    int readFD = open(recieveFIFOStr, O_RDONLY);
    char *clientToken = malloc(PACKET_SIZE * sizeof(char));
    sprintf(clientToken, "clientToken%d", clientNo);
    findNoOfPackets(clientToken);
    printf("Client token:%s\n", clientToken);
    fflush(stdout);
    while (1)
    {
        char *currMessage = malloc(PACKET_SIZE * sizeof(char));
        char *currToken = readFromPipe(readFD, &currMessage);
        if (strcmp(currToken, clientToken) == 0)
        {
            free(currToken);
            free(currMessage);
            printf("Client %d ignoring '%s'\n", clientNo, currMessage);
            fflush(stdout);
            continue;
        }
        printf("Client %d recieved '%s'\n", clientNo, currMessage);
        fflush(stdout);
        free(currToken);
        free(currMessage);
    }
}
void *sendFunction(void *voidClientNo)
{
    int clientNo = *((int *)voidClientNo);
    char *sendFIFOStr = "recieve"; //recieve wrt to the server
    mkfifo(sendFIFOStr, 0666);
    int sendFD = open(sendFIFOStr, O_WRONLY);
    char *clientToken = malloc(PACKET_SIZE * sizeof(char));
    sprintf(clientToken, "clientToken%d", clientNo);
    findNoOfPackets(clientToken);

    while (1)
    {
        char *currMessage = malloc(PACKET_SIZE * 10 * sizeof(char));
        int messageSize = PACKET_SIZE * 10;
        printf("Enter message to send from %d client:\n", clientNo);
        fflush(stdout);
        getline(&currMessage, &messageSize, stdin);
        writeToPipe(sendFD, currMessage, clientToken, clientToken);
        free(currMessage);
    }
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("formar is ./client <client no>\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    int clientNo = atoi(argv[1]);
    pthread_t sendThread, recieveThread;
    pthread_create(&recieveThread, NULL, recieveFunction, &clientNo);
    pthread_create(&sendThread, NULL, sendFunction, &clientNo);
    pthread_join(recieveThread, NULL);
    pthread_join(sendThread, NULL);
}