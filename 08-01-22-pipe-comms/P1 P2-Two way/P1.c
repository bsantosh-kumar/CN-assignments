#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
int main()
{
    int fd1[2];
    int fd2[2];
    pipe(fd1);
    pipe(fd2);
    int child = fork();
    int terminalRFD = dup(STDIN_FILENO);
    int terminalWFD = dup(STDOUT_FILENO);

    if (child > 0)
    {
        close(fd1[0]);
        close(fd2[1]);
        int writeFD = dup(fd1[1]);
        int readFD = dup(fd2[0]);
        close(fd1[1]);
        close(fd2[0]);
        bool shouldExit = false;
        while (1)
        {
            char *currMessage = calloc(100, sizeof(char));
            printf("Write string to send to P2:");
            dup2(terminalWFD, STDOUT_FILENO);
            dup2(terminalRFD, STDIN_FILENO);
            //step - 1 read from screen
            scanf("%s", currMessage);

            if (strcmp(currMessage, "NULL") == 0)
                shouldExit = true;
            dup2(writeFD, STDOUT_FILENO);
            //step - 2 write to pipe
            printf("%s \n", currMessage);
            fflush(stdout);
            if (shouldExit)
            {
                break;
            }
            dup2(readFD, STDIN_FILENO);
            free(currMessage);
            currMessage = calloc(100, sizeof(char));
            scanf("%s", currMessage);
            if (strcmp(currMessage, "NULL") == 0)
            {
                break;
            }
            dup2(terminalWFD, STDOUT_FILENO);

            dup2(terminalRFD, STDIN_FILENO);
            printf("P1 read the message '%s' from P2\n", currMessage);
            fflush(stdout);
        }
        fflush(stdout);
        dup2(terminalWFD, STDOUT_FILENO);
        printf("P1 exited\n");
        fflush(stdout);
    }
    else
    {
        close(fd1[1]);
        close(fd2[0]);
        char readFD[20];
        char writeFD[20];
        sprintf(readFD, "%d", dup(fd1[0]));
        sprintf(writeFD, "%d", dup(fd2[1]));
        close(fd1[0]);
        close(fd2[1]);
        char *arg[] = {"./P2", readFD, writeFD, NULL};
        execv(arg[0], arg);
    }
}