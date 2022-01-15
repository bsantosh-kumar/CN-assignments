#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#define PRGNO 4
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

        printf("I am P%d. I am waiting for semaphore S%d%d\n", PRGNO, getPreviousProcess(PRGNO), PRGNO);
        fflush(stdout);
        sem_wait(backwardSem);
        printf("I got semaphore signalling from P%d\n", getPreviousProcess(PRGNO));
        printf("Enter any character to sem-signal(S%d%d)\n", PRGNO, getNextProcess(PRGNO));
        fflush(stdout);
        char c;
        scanf("%c", &c);
        printf("I am signalling semaphore signal of S%d%d\n", PRGNO, getNextProcess(PRGNO));
        fflush(stdout);
        sem_post(forwardSem);
    }
}