#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
int main()
{
    char inputFile[] = "f1.txt";
    char outputFile[] = "f2.txt";
    int inputFD = open(inputFile, O_RDONLY);
    int outputFD = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    char c;
    while (1)
    {
        if (read(inputFD, &c, 1) == 0)
            break;
        if (c >= 'a' && c <= 'z')
            c = c - 'a' + 'A';
        write(outputFD, &c, 1);
    }
    return 0;
}