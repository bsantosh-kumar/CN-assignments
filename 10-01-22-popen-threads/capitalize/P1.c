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
void *writeToPipe(void *voidWriteFD)
{
    fflush(stdout);

    int writeFD = *((int *)(voidWriteFD));
    fflush(stdout);
    char buffer[SIZE];
    while (1)
    {
        printf("Enter string to P1:\n");
        fflush(stdout);
        scanf("%s", buffer);
        int lenBuffer = strlen(buffer);
        buffer[lenBuffer] = ' ';
        buffer[lenBuffer + 1] = '\0';
        write(writeFD, buffer, lenBuffer + 1);
        fflush(stdout);
        if (strcmp(buffer, "NULL ") == 0)
        {
            close(writeFD);
            break;
        }
    }
    return NULL;
}
void *readFromPipe(void *voidReadFD)
{
    int readFD = *((int *)voidReadFD);
    char buffer[SIZE];
    while (1)
    {
        read(readFD, buffer, SIZE);
        printf("P1 read '%s' from 'green' FIFO\n", buffer);
        if (strcmp(buffer, "NULL") == 0)
            break;
        fflush(stdout);
    }
    close(readFD);
    return NULL;
}
int main()
{
    pthread_t sendThread;
    pthread_t recieveThread;
    int writeFD = 1;
    int readFD = 0;
    char *writeChar = "w";
    FILE *writeFile = popen("./P2", writeChar);
    writeFD = fileno(writeFile);
    mkfifo("green", 0666);
    int ffd = open("green", O_RDONLY);
    readFD = ffd;
    pthread_create(&sendThread, NULL, writeToPipe, &writeFD);
    pthread_create(&recieveThread, NULL, readFromPipe, &readFD);
    pthread_join(sendThread, NULL);
    pthread_join(recieveThread, NULL);
    printf("P1 exiting\n");
}