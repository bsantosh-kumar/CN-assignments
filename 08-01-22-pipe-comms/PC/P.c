#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
/*
    We are going to follow a protocol for sending data in the pipe, 
    Then we are going to send the string '1059f8f0fdd21a5ce489a14341bb2c9a4e333ab6d56e929bf8d6ec477649a96b' //This is SHA-256 for 'Starting the message'
    We are first going to send an integer(4 bytes) denoting the size of the text that we are going to send through the pipe(including the 'end message')
    Then we are going to divide the text into our own sub-strings (lets call them as packets for simplicity), with each packet having a maximum size of 1023 bytes
    Next we are going to send the size of the packet first and the packet itself. 
    We continue this process until the entire message is sent out. 
    At the end of the message we are going to send, '0f9000c3144982aa6d2bb447817ebaf502d5e8d58efa3f9d71566a4a0b670cb1' //This is SHA-256 for 'This message is complete'
    If the connection is being closed by one or the other the process, 
    we are going to send the size as 0, and 'd67eae3a8b82917679e024f2446cfffb14f022cb3da897c1fac2002acaaf31d6', //This is SHA-256 for 'Closing this connection'
    instead of 'Starting this message'
*/
#define PACKET_SIZE 1
void writeToPipe(int wfd, char *currMessage, int noOfPackets, char *startingMessage, char *endingMessage)
{ //wfd -> write fd
    // printf("%d Writing to %d\n", getpid(), wfd);
    write(wfd, startingMessage, 65);
    // printf("Came to write\n");
    write(wfd, &noOfPackets, sizeof(noOfPackets));
    for (int i = 0; i < noOfPackets; i++)
    {
        write(wfd, currMessage + i * PACKET_SIZE, PACKET_SIZE);
    }
    write(wfd, endingMessage, 65);
}
bool readFromPipe(int rfd, char **currMessagePtr, int *messageSizePtr, char *startingMessage, char *endingMessage, char *closingConnection)
{
    /*
        rfd -> read fd
        return 1 if read is done, 0 if closingConnection message is sent

    */
    // printf("%d Reading from %d\n", getpid(), rfd);
    char *readStartingMessage = (char *)calloc(65, sizeof(char));
    read(rfd, readStartingMessage, 65);
    if (strcmp(readStartingMessage, closingConnection) == 0)
    {
        return 0;
    }
    int noOfPackets = 0;
    read(rfd, &noOfPackets, sizeof(noOfPackets));
    (*currMessagePtr) = (char *)(malloc(noOfPackets * PACKET_SIZE * sizeof(char)));
    for (int i = 0; i < noOfPackets; i++)
    {
        read(rfd, (*currMessagePtr) + i * PACKET_SIZE, PACKET_SIZE);
    }
    char *readEndingMessage = calloc(65, sizeof(char));
    read(rfd, readEndingMessage, 65);
    return 1;
}
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
int main()
{
    int pfd1[2], pfd2[2];
    char *input = calloc(10, sizeof(char));
    int size = 10;
    pipe(pfd1);
    pipe(pfd2);
    int child = fork();
    if (child > 0)
    {
        close(pfd1[0]);
        close(pfd2[1]);
        char *startingMessage = "1059f8f0fdd21a5ce489a14341bb2c9a4e333ab6d56e929bf8d6ec477649a96b\0";
        char *endingMessage = "0f9000c3144982aa6d2bb447817ebaf502d5e8d58efa3f9d71566a4a0b670cb1\0";
        char *closingConnection = "d67eae3a8b82917679e024f2446cfffb14f022cb3da897c1fac2002acaaf31d6\0";
        char *nullString = "NULL\n";
        bool shouldExit = false;
        while (1)
        {
            char *currMessage = malloc(sizeof(char));
            printf("Enter message to be sent from P to P':");
            int messageSize = 1;
            getline(&currMessage, &messageSize, stdin);
            messageSize = strlen(currMessage);
            if (strcasecmp(currMessage, nullString) == 0)
            {
                printf("Process P is sending message to close the connection\n");
                free(currMessage);
                currMessage = malloc(sizeof(char));
                startingMessage = closingConnection;
                messageSize = 0;
                shouldExit = true;
            }
            else
            {
            }
            int noOfPackets = myCeil(messageSize, PACKET_SIZE);
            currMessage = realloc(currMessage, noOfPackets * PACKET_SIZE * sizeof(char));
            printf("Process P is sending message '%s' to P'\n", currMessage);
            writeToPipe(pfd1[1], currMessage, noOfPackets, startingMessage, endingMessage);
            if (shouldExit == 1)
            {
                printf("Process P is exiting\n");
                break;
            }
            free(currMessage);
            printf("Process P is waiting for P' to send the message \n");
            shouldExit = !readFromPipe(pfd2[0], &currMessage, &messageSize, startingMessage, endingMessage, closingConnection);
            if (shouldExit == 1)
            {
                printf("Process P is exiting\n");
                break;
            }
            printf("Process P had recieved the following message from P':'%s'\n", currMessage);
        }
        close(pfd1[1]);
        close(pfd2[0]);
    }
    else
    {
        close(pfd2[0]);
        close(pfd1[1]);
        char *startingMessage = "1059f8f0fdd21a5ce489a14341bb2c9a4e333ab6d56e929bf8d6ec477649a96b\0";
        char *endingMessage = "0f9000c3144982aa6d2bb447817ebaf502d5e8d58efa3f9d71566a4a0b670cb1\0";
        char *closingConnection = "d67eae3a8b82917679e024f2446cfffb14f022cb3da897c1fac2002acaaf31d6\0";
        char *nullString = "NULL\n";
        bool shouldExit = false;
        while (1)
        {
            printf("Process P' is waiting for P to send the message \n");
            char *currMessage = malloc(sizeof(char));
            int messageSize = 1;
            shouldExit = !readFromPipe(pfd1[0], &currMessage, &messageSize, startingMessage, endingMessage, closingConnection);
            if (shouldExit == 1)
            {
                printf("Process P' is exiting\n");
                break;
            }
            printf("Process P' had recieved the following message from P:'%s'\n", currMessage);

            printf("Enter message to be sent from P' to P:");

            getline(&currMessage, &messageSize, stdin);
            messageSize = strlen(currMessage);
            if (strcasecmp(currMessage, nullString) == 0)
            {
                printf("Process P' is sending message to close the connection\n");
                startingMessage = closingConnection;
                messageSize = 0;
                shouldExit = true;
            }
            else
            {
            }
            int noOfPackets = myCeil(messageSize, PACKET_SIZE);
            currMessage = realloc(currMessage, noOfPackets * PACKET_SIZE * sizeof(char));
            printf("Process P' is sending message '%s' to P\n", currMessage);
            writeToPipe(pfd2[1], currMessage, noOfPackets, startingMessage, endingMessage);

            if (shouldExit == 1)
            {
                printf("Process P' is exiting\n");
                break;
            }
            free(currMessage);
        }
        close(pfd2[1]);
        close(pfd1[0]);
    }
}