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
    (*currMessagePtr) = (char *)(malloc(noOfPackets * PACKET_SIZE * sizeof(char)));
    for (int i = 0; i < noOfPackets; i++)
    {
        read(rfd, (*currMessagePtr) + i * PACKET_SIZE, PACKET_SIZE);
    }
    char *readEndingMessage = calloc(PACKET_SIZE, sizeof(char));
    read(rfd, readEndingMessage, PACKET_SIZE);
    return readStartingMessage;
}

int main()
{
    char *recieveFIFOStr = "recieve"; //recieve wrt to the server
    char *sendFIFOStr = "send";       //send wrt to the server
    char *writeChar = "w";
    char *readChar = "r";
    unlink(recieveFIFOStr);
    unlink(sendFIFOStr);
    mkfifo(recieveFIFOStr, 0666);
    mkfifo(sendFIFOStr, 0666);
    int recieveFD = open(recieveFIFOStr, O_RDONLY);
    int sendFD = open(sendFIFOStr, O_WRONLY);
    int keyboard = dup(STDIN_FILENO);
    int screen = dup(STDOUT_FILENO);
    while (1)
    {
        char *currMessage = malloc(PACKET_SIZE * 10 * sizeof(char));
        char *senderAddress = readFromPipe(recieveFD, &currMessage);
        printf("Server recieved message from %s message is:'%s'\n", senderAddress, currMessage);
        fflush(stdout);
        writeToPipe(sendFD, currMessage, senderAddress, senderAddress);
        free(currMessage);
        free(senderAddress);
    }
}