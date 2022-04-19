#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

#define PORT 8080

int tcp_client_init(char *foreign_ip, char *port)
{
    int sfd;
    struct sockaddr_in serv_addr;
    int serv_addrlen = sizeof(serv_addr);

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error in socket");
        exit(0);
    }
    printf("sfd:%d\n", sfd);
    serv_addr.sin_family = AF_INET;
    int port_no = atoi(port);
    printf("port no:%d\n", port_no);
    serv_addr.sin_port = htons(port_no);
    printf("foreign_ip:'%s' foreign_port:'%s'\n", foreign_ip, port);
    serv_addr.sin_addr.s_addr = inet_addr(foreign_ip);

    if ((connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) == -1)
    {
        perror("Error in connect tcp client init");
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
    fflush(stdout);
    return sfd;
}

int hostname_to_ip(char *hostname, char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL)
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++)
    {
        // Return the first one;
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }

    return 1;
}

int main()
{
    char buff[50] = {0};
    char msg[50] = {0};
    char *foreign_ip = (char *)malloc(sizeof(char) * 20);
    bzero(foreign_ip, sizeof(foreign_ip));

    int sfd;
    char *port = "443";
    char *hostname = "www.google.com";
    hostname_to_ip(hostname, foreign_ip);
    sfd = tcp_client_init(foreign_ip, port);

    // printf("Enter message for server : ");
    // fgets(msg, sizeof(msg), stdin);

    char *message = "GET / HTTP/2.0\r\n\r\n";
    printf("Came here\n");
    (send(sfd, message, strlen(message), 0) == -1) ? perror("Error in send") : printf("send success\n");
    fflush(stdout);
    printf("buff:%s\n", message);
    fflush(stdout);
    char *buffer = (char *)calloc(1 << 16, sizeof(char));
    while (1)
    {
        bzero(buffer, sizeof(buffer));
        printf("Waiting to recieve\n");
        int n = recv(sfd, buffer, 1 << 16, 0);
        printf("n=%d buff=\n%s\n", n, buffer);
        if (n == 0)
            break;
    }
    printf("Client Terminated\n");

    return 0;
}
