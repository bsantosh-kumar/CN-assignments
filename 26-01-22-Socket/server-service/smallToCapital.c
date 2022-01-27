#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
int main()
{
    char buff[200];
    printf("Enter the strin:\n");
    fflush(stdout);
    int fd = open("/dev/pts/3", O_WRONLY);
    write(fd, "Did printf\n", 12);
    scanf("%s", buff);
    int len = strlen(buff);
    for (int i = 0; i < len; i++)
    {
        if (buff[i] >= 'a' && buff[i] <= 'z')
        {
            buff[i] = buff[i] - 'a' + 'A';
        }
    }
    printf("%s", buff);
    fflush(stdout);

    return 0;
}