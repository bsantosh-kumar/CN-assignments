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
#include <regex.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/poll.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddr;
    if (argc != 3)
    {
        printf("Usage ./S0.out <server-ip> <server-port>\n");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

    int sfd;
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating a socket\n");
        exit(0);
    }

    if (connect(sfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Connection error\n");
        exit(0);
    }
    char *buff = (char *)calloc(200, sizeof(char));
    while (1)
    {
        bzero(buff, sizeof(buff));
        printf("Enter to send to server:\n");
        fflush(stdout);
        fgets(buff, 200, stdin);
        buff[strlen(buff) - 1] = '\0';
        write(sfd, buff, 128);
        if (strcmp(buff, "exit") == 0)
        {
            printf("Exiting\n");
            break;
        }
        bzero(buff, sizeof(buff));

        read(sfd, buff, 128);
        printf("Recieved from server '%s'\n", buff);
    }
}