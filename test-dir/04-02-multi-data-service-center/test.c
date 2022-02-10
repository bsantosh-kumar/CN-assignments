#include <stdio.h>
int main()
{
    int count = 0;
    while (1)
    {
        printf("%d hello\n", count);
        count++;
        fflush(stdout);
        sleep(1);
    }
}