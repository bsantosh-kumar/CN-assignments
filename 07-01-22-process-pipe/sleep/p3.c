#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    printf("7a) P3(%d) started and going to sleep\n", getpid());
    sleep(5);
    printf("7b) P3(%d) came from sleep and exiting\n", getpid());
}