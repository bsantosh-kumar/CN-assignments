#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define FIFO_NAME "P2-FIFO"
int getFIFOfd(char fifoName[])
{
    if (mkfifo(fifoName, 0666) == -1)
    {
        // printf("mkfifo error\n");
        // perror(strerror(errno));
        // return 1;
    }
    int wffd = open(fifoName, O_WRONLY);
    return wffd;
}

int main()
{
    int ffd = getFIFOfd(FIFO_NAME);
    char *buff = (char *)malloc(200 * sizeof(char));
    strcpy(buff, "Hi Iam P2\n");
    while (1)
    {
        if (write(ffd, buff, 128) == 0)
            break;
        sleep(2);
    }
    exit(0);
}