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
#define LOCAL_PORT 60000
int bindUDP()
{
    int sfd = -1;
    if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("Error in udp socket creation\n");
        exit(0);
    }
    struct sockaddr_in localAddr;
    bzero(&localAddr, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(LOCAL_PORT);
    localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if ((bind(sfd, (struct sockaddr *)&localAddr, sizeof(localAddr))) == -1)
    {
        printf("Error in udp bind\n");
        printf("error:%s errorno:%d\n", strerror(errno), errno);

        exit(EXIT_FAILURE);
    }

    printf("Bind successful\n");
    return sfd;
}
void sendUdp(struct sockaddr_in serverAddr, int udpSFD, char *message)
{
    char buff[200];
    strcat(buff, message);
    if ((sendto(udpSFD, buff, 128, 0, (struct sockaddr *)&serverAddr, sizeof((serverAddr)))) == -1)
    {
        printf("error in send udp\n");
    }
    else
    {
        printf("Sending %s is successful from UDP\n", message);
    }
}

bool checkValidityOfString(char *message)
{
    regex_t regex;
    if (regcomp(&regex, "^[(][[:digit:]]+[,][[:alnum:] ./-]+[)]", REG_EXTENDED) != 0)
    {
        printf("error in compilation of regex\n");
    }
    if (regexec(&regex, message, 0, NULL, 0) == 0)
        return true;
    else
        return false;
}

/*
    This is program is to send instructions to F.c server
*/
int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddr;
    if (argc != 3)
    {
        printf("Usage ./S0.out <server-ip> <UDP port>\n");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    int udpSfd = bindUDP();
    char *buff = calloc(200, sizeof(char));
    while (1)
    {
        printf("Enter message to send to server:\n");
        bzero(buff, sizeof(buff));
        fgets(buff, 200, stdin);
        buff[strlen(buff) - 1] = '\0';
        if (strcmp(buff, "exit") == 0)
        {
            printf("Exiting\n");
        }
        if (checkValidityOfString(buff) == false)
        {
            printf("It should be of the form '(port_no,path)'\n");
            continue;
        }
        sendUdp(serverAddr, udpSfd, buff);
    }
}