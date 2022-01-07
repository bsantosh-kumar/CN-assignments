#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
int main()
{
    int c;
    printf("1) This is parent process(P1)\n");
    c = fork();
    if (c > 0)
    {
        printf("2) This is parent process(P1)\n");
        wait();
        printf("4) Child has terminated\n");
    }
    else
    {
        printf("3) This is child process (P2)\n");
        char *arg[] = {NULL};
#ifdef _WIN32 //if running on windows we need to p2.exe
        execv("p2.exe", arg);
#else
        execv("p2", arg);
#endif
    }
}
/*
    The pattern is (1 2 3 4) or (1 3 2 4)
*/