#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#define PATH "/tmp/server"

int set_server (char name[]) {
    int usfd;
    struct sockaddr_un userv_addr;
    int userv_len;

    if ( (usfd = socket (AF_UNIX, SOCK_DGRAM, 0)) == -1 ){
        perror ("Error in socket creation");
        exit (0);
    }

    userv_addr.sun_family = AF_UNIX;
    strcpy (userv_addr.sun_path, name);

    unlink (PATH);

    userv_len = sizeof (userv_addr);

    if ( bind (usfd, (struct sockaddr *)&userv_addr, userv_len) == -1 ){
        perror ("Error in binding");
        exit (0);
    }

    return usfd;
}

int main () {

    int usfd;

    struct sockaddr_un ucli_addr;
    int ucli_len;

    usfd = set_server (PATH);

    ucli_len = sizeof (ucli_addr);

    char buff[50];
    recvfrom (usfd, buff, sizeof (buff), 0, (struct sockaddr *)&ucli_addr, (socklen_t *)&ucli_len);
    printf ("Received : %s\n", buff);

    sendto (usfd, "Hello from server!", 19, 0, (struct sockaddr *)&ucli_addr, ucli_len);

    return 0;
}