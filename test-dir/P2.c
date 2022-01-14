#include <stdio.h>
#include <signal.h>
void childHandler(int signal)
{
    printf("I got signal from parent\n");
    fflush(stdout);
    kill(getppid(), SIGUSR2);
}
void intHandler(int singal)
{
    static int count = 0;
    if (count == 1)
    {
        printf("Changing signal handler\n");
        printf("%d Ctrl+C left to termination\n", 2 - count);

        signal(SIGINT, SIG_DFL);
    }
    else
    {
        printf("%d Ctrl+C left to termination\n", 2 - count);
        count++;
    }
    fflush(stdout);
}
int main()
{
    signal(SIGUSR1, childHandler);
    signal(SIGINT, intHandler);
    printf("In child\n");
    fflush(stdout);
    char message[100];
    while (1)
    {
        int temp = 0;
    }
}