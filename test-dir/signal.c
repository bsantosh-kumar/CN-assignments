#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void parentHandler(int signal)
{
    printf("Got signal from child\n");
}
void printMenu()
{
    printf("Menu:\n 1-SIGUSR1\n 2-SIGINT\n");
}
main()
{
    int child = fork();

    if (child > 0)
    {
        signal(SIGUSR2, parentHandler);
        sleep(2);
        while (1)
        {
            int status;
            int returnValue = waitpid(child, &status, WNOHANG);
            if (WIFEXITED(status))
            {
                printf("Child exited\n");
            }
            printMenu();
            int temp;
            scanf("%d", &temp);
            switch (temp)
            {
            case 1:
                kill(child, SIGUSR1);
                break;
            case 2:
                int r = kill(child, SIGINT);
                break;
            default:
                printf("Accepted values are 1 or 2\n");
                break;
            }
        }
    }
    else
    {
        char *arg = {NULL};
        execv("./P2", arg);
    }
}