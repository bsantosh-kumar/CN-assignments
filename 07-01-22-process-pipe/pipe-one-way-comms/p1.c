#include <stdio.h>
#include <unistd.h>
#include <string.h>
int main()
{
    printf("1) Currently in parent process\n");
    int pfd[2];
    pipe(pfd);
    int child = fork();
    if (child > 0)
    {
        close(pfd[0]);
        char *sendMessage = "Miles, you got a minute?";
        int sendLength = strlen(sendMessage);
        printf("Sending message '%s' from parent(%d)\n", sendMessage, getpid());
        write(pfd[1], sendMessage, sendLength + 1);
        close(pfd[1]);
    }
    else
    {
        close(pfd[1]);
        char c;
        printf("Child(%d) recieved the following message:\n", getpid());
        while (read(pfd[0], &c, 1))
        {
            printf("%c", c);
        }
        printf("\n");
        close(pfd[0]);
    }
}