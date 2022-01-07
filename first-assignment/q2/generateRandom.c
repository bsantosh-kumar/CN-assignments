#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
int main(int argc, char *argv[])
{
    int n = 100;
    if (argc == 1)
    {
        printf("Require atleast one argument <file name>\n\
            One optional argument <number of numbers to write to file>\n\
            Format : ./generateRandom <file name> <n>");
        exit(0);
    }
    else if (argc == 2)
    {
        printf("Taking first argument as file name\n");
    }
    else if (argv == 3)
    {
        n = atoi(argv[3]);
    }
    else if (argc > 3)
    {
        printf("Needed only 2 arguments\n\
        Format : ./generateRandom <file name> <n>");
        exit(0);
    }
    srand(time(0));
    char *fileName = argv[1];
    int fd = open(fileName, O_WRONLY);

    char buffer[100];
    int index = 0;
    for (char c = 'a'; c <= 'z'; c++)
        buffer[index++] = c;
    for (char c = 'A'; c <= 'Z'; c++)
        buffer[index++] = c;
    for (char c = '0'; c <= '9'; c++)
        buffer[index++] = c;
    int currIndex = 0;
    for (int i = 0; i < n; i++)
    {
        int currIndex = rand() % 62;
        write(fd, &buffer[currIndex], 1);
    }
}