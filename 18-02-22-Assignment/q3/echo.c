#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <stdbool.h>
#include <fcntl.h>
#include <regex.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

int main()
{
    while (1)
    {
        char buff[200] = {0};
        bzero(buff, sizeof(buff));
        scanf("%s", buff);
        if (strcmp(buff, "exit") == 0)
            break;
        printf("%s", buff);
        fflush(stdout);
    }
    exit(0);
}