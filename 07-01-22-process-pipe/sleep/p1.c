#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main()
{
    int c;
    printf("1) This is parent process(P1)\n");
    c = fork();
    char *arg[] = {NULL};
    if (c > 0)
    {
        printf("2) This is parent process(P1)\n");
        int c2 = fork();
        int pidWait1, pidWait2;
        if (c2 > 0)
        {
            pidWait1 = wait(NULL);
        }
        else
        {
#ifdef _WIN32 //if running on windows we need to p2.exe
            execv("p3.exe", arg);
#else
            execv("p3", arg);
#endif
        }
        pidWait2 = wait(NULL);
        if (pidWait1 == c)
        {
            printf("4a) P2(%d) terminated first\n", pidWait1);
            printf("4b) P3(%d) terminated second\n", pidWait2);
        }
        else
        {
            printf("4a) P3(%d) terminated first\n", pidWait1);
            printf("4b) P2(%d) terminated second\n", pidWait2);
        }
    }
    else
    {
        printf("3) This is child process (P2)\n");
#ifdef _WIN32 //if running on windows we need to p2.exe
        execv("p2.exe", arg);
#else
        execv("p2", arg);
#endif
        printf("6) P2 process terminated\n");
    }
}
/*
    The pattern is (1 2 3 4) or (1 3 2 4)
*/