#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    printf("5a) P2(%d) started and going to sleep\n", getpid());
    sleep(10);
    printf("5b) P2(%d) came from sleep and exiting\n", getpid());
}