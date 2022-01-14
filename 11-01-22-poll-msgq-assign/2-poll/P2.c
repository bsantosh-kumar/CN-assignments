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

    int ffd;
    int number = 2;
    char fifoName[100];
    sprintf(fifoName, "FIFO%d", number);
    ffd = open(fifoName, O_WRONLY);
    while (1)
    {
        char *message = malloc(100 * sizeof(char));
        printf("Enter message into %s to send to P1\n", argv[0]);
        scanf("%s", message);
        write(ffd, message, strlen(message));
        printf("Sent '%s' message successfully\n", message);
        if (strcmp(message, "NULL") == 0)
        {
            printf("Exiting because given NULL\n");
            break;
        }
    }
}