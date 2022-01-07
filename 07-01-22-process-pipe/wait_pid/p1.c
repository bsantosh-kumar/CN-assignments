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
            printf("4aA) P1(%d) is waiting to P2(%d) to terminate\n", getpid(), c);
            pidWait1 = waitpid(c, NULL, 0);
            printf("4aB) stopped waiting for P2(%d)\n", c);
        }
        else
        {
#ifdef _WIN32 //if running on windows we need to p2.exe
            execv("p3.exe", arg);
#else
            execv("p3", arg);
#endif
        }
        printf("4bA) P1(%d) is waiting to P3(%d) to terminate\n", getpid(), c2);
        pidWait2 = waitpid(c2, NULL, 0);
        printf("4bB) stopped waiting for P3(%d) \n", c2);
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