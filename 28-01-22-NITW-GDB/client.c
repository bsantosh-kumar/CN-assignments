#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
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
void recieveServiceAnsFromServer(int nsfd)
{
    recieveAndPrintFromServer(nsfd);
}

bool sendProgram(const char *fileName, int sFD)
{
    char buff[200];
    memset(buff, '\0', sizeof(buff) * sizeof(char));
    strcpy(buff, "STARTING");
    send(sFD, buff, 128, 0);
    int fdOpen = open(fileName, O_RDONLY);
    if (fdOpen == -1)
    {
        printf("Error in open\n");
        memset(buff, '\0', sizeof(buff) * sizeof(char));
        strcpy(buff, "ENDING");
        send(sFD, buff, 128, 0);
        return false;
    }
    memset(buff, '\0', sizeof(buff) * sizeof(char));
    while (read(fdOpen, buff, 128) != 0)
    {
        send(sFD, buff, 128, 0);
        memset(buff, '\0', sizeof(buff) * sizeof(char));
    }
    memset(buff, '\0', sizeof(buff) * sizeof(char));
    strcpy(buff, "ENDING");
    send(sFD, buff, 128, 0);
    close(fdOpen);
    return true;
}
int main(int argc, char const *argv[])
{
    if (argc != 2 && argc != 3)
    {
        printf("Format ./client <functionality>\n");
        exit(0);
    }
    int SERVERPORT = 20500;
    int sFD;
    int nsFD;
    // printf("Trying to create sfd\n");
    if ((sFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating a socket\n");
        exit(0);
    }
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
    send(sFD, argv[1], strlen(argv[1]), 0);
    char buff[200];
    memset(buff, '\0', sizeof(buff) * sizeof(char));
    read(sFD, buff, 128);
    printf("Recived from server %s\n", buff);
    if (strcmp(argv[1], "exit") == 0)
    {
        printf("Exiting...\n");
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(argv[1], "program") != 0)
    {
        printf("Not valid argument\n");
        exit(EXIT_FAILURE);
    }
    if (argc != 3)
    {
        printf("Need three arguments with program ./client program <file name>\n");
    }

    int sentStatus = sendProgram(argv[2], sFD);
    recieveAndPrintFromServer(sFD);
    close(sFD);
}