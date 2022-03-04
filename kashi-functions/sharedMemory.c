#include <stdio.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <stdlib.h>

#define SharedMemory "/tmp"
#define ID1 101
//do dt also
int *create_shared_memory(char name[], int id, int size, bool del)
{
    key_t key;
    int shmid;

    if ((key = ftok(name, id)) == -1)
    {
        perror("Error in generating key");
        exit(0);
    }

    if ((shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT)) == -1)
    {
        perror("Error in shmget");
        exit(0);
    }

    if (del)
    {
        if ((shmctl(shmid, IPC_RMID, NULL)) == -1)
        {
            perror("Error in shmctl");
            exit(0);
        }

        if ((shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT)) == -1)
        {
            perror("Error in shmget");
            exit(0);
        }
    }

    int *x = (int *)shmat(shmid, (void *)0, 0);

    return x;
}

int main()
{

    int *x = create_shared_memory(SharedMemory, ID1, sizeof(int), true);

    return 0;
}