#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("The format is ./P3 <pipe FD to read from> <pipe FD to write to>");
        exit(0);
    }
    int readP1FD = atoi(argv[1]);
    int writeP1FD = atoi(argv[2]);
    int pfd1[2];
    int pfd2[2];
    pipe(pfd1);
    pipe(pfd2);
    int child = fork();
    if (child > 0)
    {
        int writeP3FD = dup(pfd1[1]);
        int readP3Fd = dup(pfd2[0]);
        int terminalRFD = dup(STDIN_FILENO);
        int terminalWFD = dup(STDOUT_FILENO);
        close(pfd1[0]);
        close(pfd1[1]);
        close(pfd2[0]);
        close(pfd2[1]);
        while (1)
        {
            char *currMessage = calloc(100, sizeof(char));
            dup2(readP1FD, STDIN_FILENO);
            scanf("%s", currMessage);
            dup2(terminalWFD, STDOUT_FILENO);
            printf("P2 recieved '%s' from P1, now sending to P3\n", currMessage);
            fflush(stdout);
            dup2(writeP3FD, STDOUT_FILENO);
            printf("%s\n", currMessage);
            fflush(stdout);
            if (strcmp(currMessage, "NULL") == 0)
            {
                break;
            }

            dup2(readP3Fd, STDIN_FILENO);
            scanf("%s", currMessage);
            dup2(terminalWFD, STDOUT_FILENO);
            printf("I am process P2\nP2 recieved '%s' from P3, now sending to P1\n", currMessage);
            fflush(stdout);
            dup2(writeP1FD, STDOUT_FILENO);
            printf("%s\n", currMessage);
            if (strcmp(currMessage, "NULL") == 0)
            {
                break;
            }
        }
        dup2(terminalWFD, STDOUT_FILENO);
        printf("P2 exited\n");
        fflush(stdout);
    }
    else
    {
        char readFD[20];
        char writeFD[20];
        sprintf(readFD, "%d", dup(pfd1[0]));
        sprintf(writeFD, "%d", dup(pfd2[1]));
        close(pfd1[0]);
        close(pfd1[1]);
        close(pfd2[0]);
        close(pfd2[1]);
        char *arg[] = {"./P3", readFD, writeFD, NULL};
        execv(arg[0], arg);
    }
}
