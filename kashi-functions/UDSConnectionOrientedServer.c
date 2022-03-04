#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#define PATH "/tmp/socket"

int set_server(char name[])
{
    int usfd;

    struct sockaddr_un userv_addr;

    int userv_len;

    if ((usfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("Error in socket creation");
        exit(0);
    }

    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, name);

    unlink(name);

    userv_len = sizeof(userv_addr);

    if (bind(usfd, (struct sockaddr *)&userv_addr, sizeof(userv_addr)) == -1)
    {
        perror("Error in binding");
        exit(0);
    }

    if (listen(usfd, 3) == -1)
    {
        perror("Error in listen");
        exit(0);
    }

    return usfd;
}

int main()
{

    int usfd;
    struct sockaddr_un ucli_addr;
    int ucli_len;

    usfd = set_server(PATH);

    int nusfd;

    if ((nusfd = accept(usfd, (struct sockaddr *)&ucli_addr, &ucli_len)) == -1)
    {
        perror("Error in accept\n");
        exit(0);
    }

    send(nusfd, "Hello from server!", 19, 0);

    char buff[50];
    recv(nusfd, buff, sizeof(buff), 0);
    printf("Received : %s\n", buff);

    return 0;
}