#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
int main()
{
    int fd = open("/dev/pts/4", O_WRONLY);
    while (1)
    {
        // printf("Enter the string to echo:");
        fflush(stdout);
        char *buff = (char *)calloc(200, sizeof(char));
        write(fd, "before scanf\n", 14);
        scanf("%s", buff);
        char temp[100];
        // printf("buff is :%s\n", buff);
        if (strcmp(buff, "exit") == 0)
        {
            printf("Exiting\n");
            break;
        }
        printf("'%s'\n", buff);
        fflush(stdout);
    }
}