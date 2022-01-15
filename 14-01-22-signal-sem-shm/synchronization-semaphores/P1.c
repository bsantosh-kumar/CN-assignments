#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#define PRGNO 1
int getNextProcess(int i)
{
    return i % 4 + 1;
}
int getPreviousProcess(int i)
{
    if ((i - 1 + 4) % 4 == 0)
        return 4;
    else
        return (i - 1 + 4) % 4;
}
sem_t *forwardSem, *backwardSem;
void init_code()
{
    for (int i = 1; i <= 4; i++)
    {
        int curr = i, next = getNextProcess(i);
        char currSem[10];
        sprintf(currSem, "S%d%d", curr, next);
        sem_unlink(currSem);
    }
    int adjacent[2][2];
    adjacent[0][0] = getPreviousProcess(PRGNO);
    adjacent[1][1] = getNextProcess(PRGNO);
    adjacent[0][1] = adjacent[1][0] = PRGNO;
    for (int i = 0; i < 2; i++)
    {
        char currSemName[10];
        sprintf(currSemName, "S%d%d", adjacent[i][0], adjacent[i][1]);
        if (i == 0)
            backwardSem = sem_open(currSemName, O_CREAT, 0666, 0);
        else
            forwardSem = sem_open(currSemName, O_CREAT, 0666, 0);
    }
}
int main()
{
    init_code();
    while (1)
    {
        printf("I am P%d. Enter any character to sem-signal(S%d%d)\n", PRGNO, PRGNO, getNextProcess(PRGNO));
        fflush(stdout);
        char c;
        scanf("%c", &c);
        printf("I am signalling semaphore signal of S%d%d\n", PRGNO, getNextProcess(PRGNO));
        fflush(stdout);
        sem_post(forwardSem);
        printf("I am waiting for semaphore S%d%d\n", getPreviousProcess(PRGNO), PRGNO);
        fflush(stdout);
        sem_wait(backwardSem);
        printf("I got semaphore signalling from P%d\n", getPreviousProcess(PRGNO));
        fflush(stdout);
    }
}