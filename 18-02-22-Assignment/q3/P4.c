#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int get_pid_from_name(char pName[])
{
    char command[200];
    bzero(command, sizeof(command));
    strcat(command, "pidof ");
    strcat(command, pName);
    int temp = fileno(popen(command, "r"));
    char buff[200];
    bzero(buff, sizeof(buff));
    read(temp, buff, 128);
    return atoi(buff);
}
int main()
{

    char buff[200];
    while (1)
    {
        fgets(buff, 200, stdin);
        if (strcmp(buff, "exit\n") == 0)
            break;
        int pidOfServer = get_pid_from_name("./server.out");
        kill(pidOfServer, SIGUSR1);
        printf("Sent signal\n");
        fflush(stdout);
    }
}