#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#define CLIENT "/tmp/client"
#define SERVER "/tmp/server"

int uds_set_client(char server[], char client[], struct sockaddr_un *userv_addr, int *userv_len)
{
    int usfd;
    struct sockaddr_un ucli_addr;
    int ucli_len;

    if ((usfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        perror("Error in socket creation");
        exit(0);
    }

    ucli_addr.sun_family = AF_UNIX;
    strcpy(ucli_addr.sun_path, client);

    (*userv_addr).sun_family = AF_UNIX;
    strcpy((*userv_addr).sun_path, server);

    unlink(client);

    ucli_len = sizeof(ucli_addr);
    *userv_len = sizeof(*userv_addr);

    if (bind(usfd, (struct sockaddr *)&ucli_addr, ucli_len) == -1)
    {
        perror("Error in binding");
        exit(0);
    }

    return usfd;
}

int main()
{

    int usfd;

    struct sockaddr_un userv_addr;

    int userv_len;

    usfd = uds_set_client(SERVER, CLIENT, &userv_addr, &userv_len);

    sendto(usfd, "Hello from client!", 19, 0, (struct sockaddr *)&userv_addr, userv_len);

    char buff[19];
    recvfrom(usfd, buff, sizeof(buff), 0, (struct sockaddr *)&userv_addr, (socklen_t *)&userv_len);
    printf("Received : %s\n", buff);

    return 0;
}