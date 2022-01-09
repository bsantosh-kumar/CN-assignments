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
    int readFD = atoi(argv[1]);
    int writeFD = atoi(argv[2]);
    int terminalRFD = dup(STDIN_FILENO);
    int terminalWFD = dup(STDOUT_FILENO);
    bool shouldExit = false;
    while (1)
    {
        char *currMessage = calloc(100, sizeof(char));
        //step - 1 read from pipe
        dup2(terminalWFD, STDOUT_FILENO);
        dup2(terminalRFD, STDIN_FILENO);
        dup2(readFD, STDIN_FILENO);

        scanf("%s", currMessage);
        if (strcmp(currMessage, "NULL") == 0)
        {
            break;
        }
        printf("I am process P3\n");
        fflush(stdout);

        printf("P2 sent '%s' message to P3\n", currMessage);
        fflush(stdout);

        printf("Enter message to send to P2 from P3:");
        fflush(stdout);

        dup2(terminalRFD, STDIN_FILENO);

        free(currMessage);
        currMessage = calloc(100, sizeof(char));
        scanf("%s", currMessage);
        if (strcmp(currMessage, "NULL") == 0)
        {
            shouldExit = true;
        }
        dup2(writeFD, STDOUT_FILENO);
        printf("%s \n", currMessage);
        fflush(stdout);
        if (shouldExit)
        {
            break;
        }
        for (int i = 0; i < 100; i++)
        {
            int temp = 0;
        }
        dup2(terminalWFD, STDOUT_FILENO);
    }
    fflush(stdout);
    dup2(terminalWFD, STDOUT_FILENO);
    printf("P3 exited\n");
    fflush(stdout);
}
