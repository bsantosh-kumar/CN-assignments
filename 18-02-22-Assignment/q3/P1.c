#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Format ./P1.out <pipe-fd>\n");
        exit(EXIT_FAILURE);
    }
    int pfd = atoi(argv[1]);
    char buff[200] = "Hi I am P1\n";
    while (1)
    {
        write(pfd, buff, 128);
        sleep(2);
    }
}