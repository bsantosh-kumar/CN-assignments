#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#define PORT 20500
int count = 0;
void *talkWithClient(void *nsFDPtr)
{
    int nsFD = *(int *)nsFDPtr;
    char buffer[128];
    printf("%d Trying to read from nsFD\n", count);
    fflush(stdout);

    int readReturn = read(nsFD, buffer, 128);
    if (readReturn == 0)
    {
        printf("Nothing to read\n");
        exit(0);
    }
    printf("%d Read '%s' from client\n", count, buffer);
    char *reply = "I recieved it";
    printf("Trying to send '%s' to client\n", reply);
    fflush(stdout);

    if (send(nsFD, reply, strlen(reply), 0) < 0)
    {
        printf("Some error happened while sending: %s\n", strerror(errno));
        exit(errno);
    }
    printf("%d Sent message is successful\n", count);
    close(nsFD);
    fflush(stdout);
}
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
    if (bind(sFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
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
    fflush(stdout);

    while (1)
    {
        if ((nsFD = accept(sFD, (struct sockaddr *)&serverAddress, &addressLength)) < 0)
        {
            perror("Error in accept\n");
            exit(0);
        }
        printf("Accepting done\n");
        fflush(stdout);
        int *nsFDPtr = (int *)malloc(sizeof(int));
        *nsFDPtr = nsFD;
        pthread_t handleClient;
        pthread_create(&handleClient, NULL, talkWithClient, nsFDPtr);
        count++;
        printf("Created a thread %d\n", count);
        printf("Enter 'E' to exit\n");
        fflush(stdout);

        char c[10];
        scanf("%s", c);
        if (strcmp(c, "exit") == 0)
        {
            break;
        }
    }
    close(sFD);
}