#include <stdio.h>
#include <unistd.h>
#include <string.h>
int main()
{
    printf("1) Currently in parent process\n");
    int pfd1[2];
    int pfd2[2];
    pipe(pfd1);
    pipe(pfd2);
    int child = fork();
    if (child > 0)
    {
        close(pfd1[0]);
        close(pfd2[1]);
        char *sendMessageParent = "Miles, you got a minute?\0";
        int sendLength = strlen(sendMessageParent);
        printf("Sending message '%s' from parent(%d) to child(%d)\n", sendMessageParent, getpid(), child);
        write(pfd1[1], sendMessageParent, sendLength + 1);

        char c;
        printf("Reading message in parent(%d) sent from child(%d):\n", getpid(), child);
        while (read(pfd2[0], &c, 1))
        {
            printf("parent:'%c'\n", c);
            if (c == '\0')
            {
                break;
            }
        }
        printf("\n");
        close(pfd2[0]);
        close(pfd1[1]);
    }
    else
    {
        close(pfd1[1]);
        close(pfd2[0]);
        char c;
        char *sendMessageChild = "Needless to say, I keep her in check\0";
        int sendLength = strlen(sendMessageChild);
        printf("In child process\n");
        printf("Sending message '%s' from child(%d) to parent(%d)\n", sendMessageChild, getpid(), getppid());
        write(pfd2[1], sendMessageChild, sendLength + 1);

        printf("Reading message in child(%d) sent from parent(%d):\n", getpid(), getppid());

        while (read(pfd1[0], &c, 1))
        {
            printf("child:'%c'\n", c);
            if (c == '\0')
                break;
        }
        printf("\n");
        close(pfd1[0]);
        close(pfd2[1]);
    }
}