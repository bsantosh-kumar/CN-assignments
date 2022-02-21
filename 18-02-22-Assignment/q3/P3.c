#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
int main()
{
    char *buff = (char *)malloc(200 * sizeof(char));
    strcpy(buff, "Hi I am P3\n");
    while (1)
    {
        printf("%s\n", buff);
        fflush(stdout);
        sleep(2);
    }
}