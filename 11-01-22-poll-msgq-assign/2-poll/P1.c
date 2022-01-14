#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
int main(int argc, char *argv[])
{
    printf("Hello this is %s\n", argv[0]);

    struct pollfd plFD[3];
    for (int i = 2; i <= 4; i++)
    {
        char fifoName[100];
        sprintf(fifoName, "FIFO%d", i);
        mkfifo(fifoName, 0666);
        plFD[i - 2].fd = open(fifoName, O_RDONLY);
        plFD[i - 2].events = POLLIN;
    }
    int nFD = 3;
    int openFDs = 3;
    while (1)
    {
        if (openFDs == 0)
            break;
        poll(plFD, nFD, -1);
        for (int i = 0; i < 3; i++)
        {
            char *message = malloc(100 * sizeof(char));
            if (plFD[i].revents & POLLIN)
            {
                int noOfBytes = read(plFD[i].fd, message, 100);
                if (noOfBytes == 0)
                {
                    printf("Closing connection with P%d due to EOF\n", i + 2);
                    openFDs--;
                    close(plFD[i].fd);
                    plFD[i].fd = -1;
                }
                char *nullMessage = "NULL";
                printf("P%d sent message '%s' to P1\n", i + 2, message);
                if (strcmp(message, nullMessage) == 0)
                {
                    printf("Closing connection with P%d due to NULL\n", i + 2);
                    openFDs--;
                    close(plFD[i].fd);
                    plFD[i].fd = -1;
                }
            }
            else if (plFD[i].revents & POLLHUP)
            {
                printf("Closing connection with P%d due to hungup\n", i + 2);
                openFDs--;
                close(plFD[i].fd);
                plFD[i].fd = -1;
            }
            free(message);
        }
    }
}