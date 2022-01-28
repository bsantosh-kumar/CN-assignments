#include <stdio.h>
int main()
{
    int a, b;
    scanf("%d %d", &a, &b);
    int sum = a + b;
    printf("%d\n", sum);
    scanf("%d %d", &a, &b);
    int diff = a - b;
    printf("%d\n", diff);
    scanf("%d %d", &a, &b);
    int prod = a * b;
    printf("%d\n", prod);
    fflush(stdout);
}