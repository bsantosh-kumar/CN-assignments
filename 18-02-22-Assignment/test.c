#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void getPortAndPath(char *buff, int *portPtr, char *path)
{
    char *portStart = strtok(buff, "(,)");
    char *portEnd = strtok(NULL, "(,)");
    *portPtr = atoi(portStart);
    strtok(NULL, "(,)");
    strcpy(path, portEnd);
}
int main()
{
    char *buff = (char *)calloc(100, sizeof(char));
    strcpy(buff, "(69,./S1.out)");
    int *portPtr = (int *)malloc(sizeof(int));
    char *path = (char *)calloc(100, sizeof(char));
    getPortAndPath(buff, portPtr, path);
}