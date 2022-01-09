#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("The format is ./P2 <pipe FD to read from> <pipe FD to write to>");
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
        printf("Startin of P2:\n");
        fflush(stdout);

        scanf("%s", currMessage);
        if (strcmp(currMessage, "NULL") == 0)
        {
            break;
        }
        printf("In P2 \"%s\"\n", currMessage);
        fflush(stdout);

        printf("P1 sent '%s' message to P2\n", currMessage);
        fflush(stdout);

        printf("Enter message to send to P1 from P2:");
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
        printf("Sent the message '%s'\n", currMessage);
        fflush(stdout);
    }
    fflush(stdout);
    dup2(terminalWFD, STDOUT_FILENO);
    printf("P2 exited\n");
}