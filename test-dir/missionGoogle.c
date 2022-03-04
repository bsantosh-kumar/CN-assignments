#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 80

int set_client(int port)
{
    int sfd;
    struct sockaddr_in serv_addr;
    int serv_addrlen = sizeof(serv_addr);

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error in socket");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, "142.250.196.46", &serv_addr.sin_addr) <= 0)
    {
        perror("Error in network address");
        exit(0);
    }

    if (connect(sfd, (struct sockaddr *)&serv_addr, sizeof serv_addr) == -1)
    {
        perror("Error in connect");
        exit(0);
    }

    if (getsockname(sfd, (struct sockaddr *)&serv_addr, (socklen_t *)&serv_addrlen) == -1)
    {
        perror("Error in getsockname");
        exit(0);
    }

    printf("Local IP Addresss : %s\n", inet_ntoa(serv_addr.sin_addr));
    printf("Local Port : %d\n\n", ntohs(serv_addr.sin_port));

    if (getpeername(sfd, (struct sockaddr *)&serv_addr, (socklen_t *)&serv_addrlen) == -1)
    {
        perror("Error in getpeername");
        exit(0);
    }

    printf("Foreign IP Addresss : %s\n", inet_ntoa(serv_addr.sin_addr));
    printf("Foreign Port : %d\n\n", ntohs(serv_addr.sin_port));

    return sfd;
}

int main()
{
    char buff[1024] = {0};
    char msg[1024] = {0};

    int sfd;

    sfd = set_client(PORT);
    printf("Connected\n");
    close(sfd);
    printf("Enter message for server : ");
    fgets(msg, sizeof(msg), stdin);

    send(sfd, msg, strlen(msg), 0);
    bzero(msg, sizeof(msg));
    recv(sfd, msg, 1024, 0);
    printf("recieved:'%s'\n", msg);
    close(sfd);
    printf("Client Terminated\n");
    return 0;
}
