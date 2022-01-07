#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    printf("7) P3(%d) started and going to sleep\n", getpid());
    sleep(5);
    printf("9) P3(%d) came from sleep and exiting\n", getpid());
}