#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#define SERVERPORT 20500
#define PORT 6942
int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Format ./client <message>\n");
        exit(0);
    }
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
    if (connect(sFD, &serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Connection error\n");
        exit(0);
    }
    printf("Connection successful\n");
    char *sendMessage = argv[1];
    send(sFD, sendMessage, strlen(sendMessage), 0);
    printf("Sent message\n");
    char buffer[128];
    read(sFD, buffer, 128);
    printf("Read from server:'%s'\n", buffer);
    close(sFD);
}