#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int pfd[2], c;
    pipe(pfd);
    c = fork();
    if (c > 0)
    {
        close(pfd[0]);

        dup2(pfd[1], 1);
        char *message = "Testing\n";
        printf("%s", message);

        close(pfd[1]);

        // wait(NULL);
    }
    else
    {
        close(pfd[1]);

        dup2(pfd[0], 0);
        char *arg[] = {NULL};
        execvp("./p2", arg);
    }
    return 0;
}