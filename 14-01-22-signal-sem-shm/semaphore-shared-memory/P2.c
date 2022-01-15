#include <stdio.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
key_t getXKey()
{
    return ftok("makefile", 6);
}
key_t getYKey()
{
    return ftok("makefile", 9);
}
int getxShmID()
{
    key_t xKey = getXKey();
    int xShmID = shmget(xKey, sizeof(int), 0666 | IPC_CREAT);
    return xShmID;
}
int getyShmID()
{
    key_t yKey = getYKey();
    int yShmID = shmget(yKey, sizeof(int), 0666 | IPC_CREAT);
    return yShmID;
}
sem_t *S1, *S2;
void getSemaphores()
{
    S1 = sem_open("S1", O_CREAT, 0666, 0);
    S2 = sem_open("S2", O_CREAT, 0666, 0);
}
int main()
{
    int xShmID = getxShmID();
    int yShmID = getyShmID();
    getSemaphores();
    char c;
    int *x = shmat(xShmID, NULL, 0);
    int *y = shmat(yShmID, NULL, 0);
    while (1)
    {
        printf("I am waiting for S1\n");
        sem_wait(S1);
        *y = *x + 1;
        printf("Value of Y is:%d\n", *y);
        printf("Enter any char to signal S2\n");
        scanf("%c", &c);
        sem_post(S2);
    }
}