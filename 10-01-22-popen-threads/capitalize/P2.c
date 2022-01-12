#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#define SIZE 101
int main()
{
    char *writeChar = "w";

    FILE *writeFile = popen("./P3", writeChar);
    int fd = fileno(writeFile);
    int screen = dup(1);
    dup2(fd, 1);
    while (1)
    {
        char buffer[100];
        scanf("%s", buffer);
        printf("%s ", buffer);
        fflush(stdout);
        if (strcmp(buffer, "NULL") == 0)
        {
            break;
        }
    }
    dprintf(screen, "P2 is exiting\n");
    close(fd);
}