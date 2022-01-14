#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

int main()
{

    mkfifo("FIFO2", 0666);
    int fd = open("FIFO2", O_WRONLY);
    printf("FIFO opened\n");
    while (1)
    {
        printf("Enter message to send to P1:\n");
        char c[100] = {0};

        read(0, c, 100);
        write(fd, c, strlen(c));

        fflush(stdout);
    }

    return 0;
}