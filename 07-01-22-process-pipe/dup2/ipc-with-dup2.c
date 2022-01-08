#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
int main()
{
    int fd1[2];
    int fd2[2];
    pipe(fd1);
    pipe(fd2);
    int child = fork();
    if (child > 0)
    {
        close(fd1[0]);
        close(fd2[1]);
        char *sendMessage = "This_is_a_message";
        char recievedMessageParent[50];
        write(fd1[1], sendMessage, strlen(sendMessage) + 1);
        close(fd1[1]);

        wait(NULL);

        printf("Child process exited\n");
        char c;
        printf("Child output is as follows:\n\"");
        while (read(fd2[0], &c, 1))
        {
            printf("%c", c);
        }
        printf("\"\n");
    }
    else
    {
        close(fd1[1]);
        close(fd2[0]);
        dup2(fd1[0], 0);
        dup2(fd2[1], 1);
        char *arg[] = {NULL};
        execv("p2", arg);
    }
}