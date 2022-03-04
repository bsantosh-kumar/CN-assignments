#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

int create_fifo (char name[], int type) {
    int fd;

    if ( mkfifo (name, 0666) == -1) {
        perror ("Error in mkfifo");
        exit (0);
    }

    if ( (fd = open (name, type)) == -1) {
        perror ("Error in open");
        exit (0);
    }

    return fd;
}

int main () {
    int fd = create_fifo ("/tmp/green", O_RDONLY);

    return 0;
}