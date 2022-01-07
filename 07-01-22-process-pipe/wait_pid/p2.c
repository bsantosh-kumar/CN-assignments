#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    printf("5) P2(%d) started and going to sleep\n", getpid());
    sleep(10);
    printf("8) P2(%d) came from sleep and exiting\n", getpid());
}