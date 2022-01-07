#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
int main()
{
    char inputFile[] = "f1.txt";
    char outputFile[] = "f2.txt";
    int inputFD = open(inputFile, O_RDONLY);
    int outputFD = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    int noOfIntegers = 0;
    char c;
    int currPos;
    while (1)
    {
        if (read(inputFD, &c, 1) == 0)
            break;
        if (c >= '0' && c <= '9')
        {
            currPos = lseek(outputFD, 0, SEEK_CUR);
            lseek(outputFD, 1000 + noOfIntegers, SEEK_SET);
            write(outputFD, &c, 1);
            noOfIntegers++;
            lseek(outputFD, currPos, SEEK_SET);
        }
        else
        {
            write(outputFD, &c, 1);
        }
    }
    currPos = lseek(outputFD, 0, SEEK_CUR);
    c = ' ';
    while (currPos != 1000)
    {
        write(outputFD, &c, 1);
        currPos++;
    }
}