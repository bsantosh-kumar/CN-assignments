#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <semaphore.h>
void maskSignal(int signal)
{
    sigset_t new_mask;

    sigemptyset(&new_mask);
    sigaddset(&new_mask, signal);
    sigprocmask(SIG_BLOCK, &new_mask, NULL);
}
void unmaskSignal(int signal)
{
    sigset_t new_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, signal);
    sigprocmask(SIG_UNBLOCK, &new_mask, NULL);
}
int main()
{
    maskSignal(SIGINT);
    sem_t sem;
    sem_init(&sem, 0, 0);
    sem_wait(&sem);
}