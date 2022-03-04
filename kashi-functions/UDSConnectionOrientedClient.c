#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

#define PATH "/tmp/socket"

int set_client (char name[]) {
    int usfd;

    struct sockaddr_un userv_addr;

    int userv_len;

    if ( (usfd = socket (AF_UNIX, SOCK_STREAM, 0)) == -1 ){
        perror ("Error in socket creation");
        exit (0);
    }

    userv_addr.sun_family = AF_UNIX;
    strcpy (userv_addr.sun_path, name);

    userv_len = sizeof (userv_addr);

    if ( connect (usfd, (struct sockaddr *)&userv_addr, userv_len) == -1 ){
        perror ("Error in accept");
        exit (0);
    }

    return usfd;
}

int main () {

    int usfd;

    usfd = set_client (PATH);

    char buff[50];
    recv (usfd, buff, sizeof (buff), 0);
    printf ("Received : %s\n", buff);

    send (usfd, "Hello from client!", 19, 0);

    return 0;
}