#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
int readIntegerFromFile(int fd)
{
    int ans = 0;
    char c;
    read(fd, &c, 1);
    while (c == ' ' || c == '\n' || c == '\t')
    {
        int temp = read(fd, &c, 1);
        if (temp == 0)
            return -1;
    }
    ans = ans * 10 + (c - '0');
    while (1)
    {
        if (read(fd, &c, 1) == 0) //End of file is reached
        {
            break;
        }
        if (c == ' ' || c == '\t' || c == '\n') //a number is completed
        {
            break;
        }
        ans = ans * 10 + (c - '0');
    }

    return ans;
}
int main()
{
    char fileName1[] = "f1.txt";
    char fileName2[] = "f2.txt";
    char fileName3[] = "f3.txt";
    int fd1 = open(fileName1, O_RDONLY);
    int fd2 = open(fileName2, O_RDONLY);
    int fd3 = open(fileName3, O_WRONLY);
    int n1 = readIntegerFromFile(fd1), n2 = readIntegerFromFile(fd2);
    int n = 0;
    char writeString[20];
    while (n1 != -1 && n2 != -1)
    {
        if (n1 < n2)
        {
            n = n1;
            n1 = readIntegerFromFile(fd1);
        }
        else
        {
            n = n2;
            n2 = readIntegerFromFile(fd2);
        }
        sprintf(writeString, "%d ", n);
        write(fd3, writeString, strlen(writeString));
    }
    while (n1 != -1)
    {
        n = n1;
        n1 = readIntegerFromFile(fd1);
        sprintf(writeString, "%d ", n);
        write(fd3, writeString, strlen(writeString));
    }
    while (n2 != -1)
    {
        n = n2;
        n2 = readIntegerFromFile(fd2);
        sprintf(writeString, "%d ", n);
        write(fd3, writeString, strlen(writeString));
    }
    close(fd1);
    close(fd2);
    close(fd3);
}