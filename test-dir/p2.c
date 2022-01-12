#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/poll.h>
int main()
{
    for (int i = 0; i < 10; i++)
    {
        printf("Hello %d\n", i + 1);
    }
    return 0;
}