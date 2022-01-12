// server
#include <errno.h>
#include <fcntl.h> /* Definition of AT_* constants */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// https://unix.stackexchange.com/questions/564098/named-pipe-buffer-after-process-end

int main()
{
    char *myFIFO = "channel";
    unlink(myFIFO);
    mkfifo(myFIFO, 0666);
    // if (mkfifo(myFIFO, 0666) == -1)
    // {
    //     printf("mkfifo error\n");
    //     char *errorIs = strerror(errno);
    //     perror(errorIs);
    //     return 1;
    // }
    int wffd = open("./channel", O_WRONLY);
    printf("Came here\n");
    if (wffd == -1)
    {
        printf("open error\n");
        close(wffd);
        unlink(myFIFO);
        return 1;
    }
    write(wffd, "hello", 6);
    close(wffd);
    unlink(myFIFO);
}