#include <stdio.h>
#include <string.h>

int main()
{
    char buff[200];
    printf("Enter the strin:\n");
    fflush(stdout);

    scanf("%s", buff);
    int len = strlen(buff);
    for (int i = 0; i < len; i++)
    {
        if (buff[i] >= 'A' && buff[i] <= 'A')
        {
            buff[i] = buff[i] - 'A' + 'a';
        }
    }
    printf("%s", buff);
    fflush(stdout);

    return 0;
}