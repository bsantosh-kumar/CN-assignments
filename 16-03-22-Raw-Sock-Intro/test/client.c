#define __USE_BSD       /* use bsd'ish ip header */
#include <sys/socket.h> /* these headers are for a Linux system, but */
#include <netinet/in.h> /* the names on other systems are easy to guess.. */
#include <netinet/ip.h>
#define __FAVOR_BSD /* use bsd'ish tcp header */
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define P 9013

int main()
{
    int rsfd;
    struct sockaddr_in sin;
    int addrlen = sizeof(sin);

    char msg[4096] = {0};

    if ((rsfd = socket(PF_INET, SOCK_RAW, 200)) == -1)
    {
        perror("Error in socket");
        exit(0);
    }

    sin.sin_family = PF_INET;

    if (inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr) <= 0)
    {
        perror("Error in network address");
        exit(0);
    }
    sin.sin_port = htons(6969);
    fgets(msg, sizeof(msg), stdin);
    printf("Sending %s\n", msg);
    sendto(rsfd, msg, 4096, 0, (struct sockaddr *)&sin, sizeof(sin));

    return 0;
}