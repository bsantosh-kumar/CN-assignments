#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#define PORT 20500

int sendToClient(int nsfd, char *message)
{
    send(nsfd, message, strlen(message), 0);
}
int recieveFromClient(int nsfd, char *buffer)
{
    read(nsfd, buffer, 128);
}
void recieveAndPrintFromClient(int nsfd)
{
    char *buff = (char *)calloc(200, sizeof(char));
    recieveFromClient(nsfd, buff);
    printf("Recived from client:%s\n", buff);
    fflush(stdout);
    free(buff);
}
void sendMenuToClient(int nsfd)
{
    char *message = (char *)calloc(200, sizeof(char));
    sprintf(message, "1)Small to captial\n2)Captial to small\n3)Exit\n");
    sendToClient(nsfd, message);
    free(message);
}
int recieveMenuResponse(int nsfd)
{
    char *buffer = (char *)calloc(200, sizeof(char));
    recieveFromClient(nsfd, buffer);
    int response = atoi(buffer);
    printf("Recieved from client as menu response:'%s'\n", buffer);
    free(buffer);
    if (response >= 1 && response <= 3)
        return response;
    else
        return -1;
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
    int True = 1;
    if (setsockopt(sFD, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(int)) < 0)
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
    int count = 1;
    while (1)
    {
        printf("At start of accept\n");
        fflush(stdout);
        if ((nsFD = accept(sFD, (struct sockaddr *)&serverAddress, &addressLength)) < 0)
        {
            perror("Error in accept\n");
            exit(0);
        }
        int child = fork();
        if (child > 0)
        {
            close(nsFD);
            wait(NULL);
        }
        else
        {
            close(sFD);
            sendMenuToClient(nsFD);
            bool flag = false;
            int menuAnswer = recieveMenuResponse(nsFD);

            if (menuAnswer == -1)
            {
                sendToClient(nsFD, "INVALID");
                flag = false;
            }
            else
            {
                sendToClient(nsFD, "VALID");
                flag = true;
            }

            if (flag == false)
            {
                exit(EXIT_FAILURE);
            }
            if (menuAnswer == 3)
            {
                close(nsFD);
                exit(EXIT_SUCCESS);
            }
            printf("Menu answer:%d\n", menuAnswer);
            fflush(stdout);
            dup2(nsFD, STDIN_FILENO);
            dup2(nsFD, STDOUT_FILENO);
            if (menuAnswer == 1)
            {
                char *arg[] = {"./smallToCapital", NULL};
                execvp(arg[0], arg);
            }
            else
            {
                char *arg[] = {"./capitalToSmall", NULL};
                execvp(arg[0], arg);
            }
            exit(1);
        }
    }
    close(sFD);
}
