#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#define SIZE 100
int main()
{
    mkfifo("green", 0666);
    int ffd = open("green", O_WRONLY);
    int writeFD = ffd;
    while (1)
    {
        char buffer[100];
        scanf("%s", buffer);
        if (strcmp(buffer, "NULL") == 0)
        {
            write(writeFD, buffer, strlen(buffer) + 1);
            break;
        }
        int lenBuffer = strlen(buffer);
        for (int i = 0; i < lenBuffer; i++)
        {
            if (buffer[i] >= 'a' && buffer[i] <= 'z')
            {
                buffer[i] = buffer[i] - 'a' + 'A';
            }
        }
        printf("In P3 '%s'\n", buffer);
        strcat(buffer, " from P3 ");
        write(writeFD, buffer, lenBuffer + 10);
    }
    close(writeFD);
    printf("P3 is exiting\n");
}