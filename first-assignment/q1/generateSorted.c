#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
void intToString(char s[], int n, int *index)
{
    if (n == 0)
        return;
    int i = n % 10;
    n = n / 10;
    intToString(s, n, index);
    s[*index] = i + '0';
}
int main(int argc, char *argv[])
{
    int n = 100;
    if (argc == 1)
    {
        printf("Require atleast one argument <file name>\n\
            One optional argument <number of numbers to write to file>\n\
            Format : ./generateSorted <file name> <n>");
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
        Format : ./generateSorted <file name> <n>");
        exit(0);
    }
    srand(time(0));
    char *fileName = argv[1];

    int fd = open(fileName, O_WRONLY);
    int currNumber = rand() % 100;
    int increment = 0;
    char currString[20];
    for (int i = 0; i < n; i++)
    {
        int index = 0;
        sprintf(currString, "%d ", currNumber);
        write(fd, currString, strlen(currString));
        increment = rand() % 10;
        currNumber += increment;
    }
    close(fd);
    return 0;
}