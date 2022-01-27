#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#define PORT 20500
int main(int argc, char const *argv[])
{
    int sFD;
    int nsFD;
    printf("Trying to create sfd\n");
    if ((sFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating a socket\n");
        exit(0);
    }
    int true = 1;
    if (setsockopt(sFD, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) < 0)
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
    if (bind(sFD, &serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error in bind, error no:%d, error is: %s\n", errno, strerror(errno));
        exit(0);
    }
    printf("Bind successful\n");
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
        if ((nsFD = accept(sFD, &serverAddress, &addressLength)) < 0)
        {
            perror("Error in accept\n");
            exit(0);
        }
        printf("Accepting done\n");
        int child = fork();
        if (child != 0)
        {
            printf("Created a child %d\n", child);
            count++;
            close(nsFD);
            printf("Enter 'E' to exit\n");
            char c[10];
            scanf("%s", c);
            if (strcmp(c, "exit") == 0)
            {
                break;
            }
        }
        else
        {
            close(sFD);
            char buffer[128];
            printf("%d Trying to read from nsFD\n", count);
            int readReturn = read(nsFD, buffer, 128);
            if (readReturn == 0)
            {
                printf("Nothing to read\n");
                exit(0);
            }
            printf("%d Read '%s' from client\n", count, buffer);
            char *reply = "I recieved it";
            printf("Trying to send '%s' to client\n", reply);
            if (send(nsFD, reply, strlen(reply), 0) < 0)
            {
                printf("Some error happened while sending: %s\n", strerror(errno));
                exit(errno);
            }
            printf("%d Sent message is successful\n", count);
            close(nsFD);
            exit(0);
        }
    }
    close(sFD);
}