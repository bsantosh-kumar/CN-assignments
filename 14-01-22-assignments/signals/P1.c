#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <semaphore.h>
#define PRGNO 1
int leftPID = 0, rightPID = 0;

struct message_struct
{
    long mtype;
    char mtext[100];
};
void handler(int signo, siginfo_t *info, void *context)
{
    int recievedPID = info->si_pid;
    leftPID = recievedPID;
    printf("P%d go to know it's left is: %d\n", PRGNO, leftPID);
}
sem_t *previousSem, *currSem, *nextSem;
void intializeHandler()
{
    struct sigaction act = {0};
    act.sa_flags = SA_SIGINFO;

    act.sa_sigaction = &handler;
    if (sigaction(SIGUSR1, &act, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}
void intializeSemaphores()
{
    char previousNum[10];
    char currNum[10];
    char nextNum[10];
    sprintf(previousNum, "%d", (PRGNO - 1 + 4) % 4);
    sprintf(currNum, "%d", PRGNO);
    sprintf(nextNum, "%d", PRGNO % 4 + 1);
    previousSem = sem_open(previousNum, O_CREAT, 0666, 0);
    currSem = sem_open(currNum, O_CREAT, 0666, 0);
    nextSem = sem_open(nextNum, O_CREAT, 0666, 0);
}
int getMessageID()
{
    key_t key = ftok("makefile", 69);
    int msgID = msgget(key, 0666 | IPC_CREAT);
    return msgID;
}
struct message_struct recieveMessage(int msgID, int type)
{
    struct message_struct message;
    msgrcv(msgID, &message, sizeof(message.mtext), type, 0);
    return message;
}
void *getRightFunc(void *voidPtr)
{
    int *rightPtr = (int *)voidPtr;
    int msgID = getMessageID();
    struct message_struct recievedMessage = recieveMessage(msgID, 2);
    *rightPtr = atoi(recievedMessage.mtext);
    kill(*rightPtr, SIGUSR1);
    sem_post(nextSem);
    printf("P%d right is:%d type:%d\n", PRGNO, *rightPtr, recievedMessage.mtype);
}
void *getLeftFunc(void *voidPtr)
{
    sem_wait(currSem);
}
void init_code()
{
    printf("Hi I am P%d PID:%d\n", PRGNO, getpid());
    fflush(stdout);
    intializeHandler();
    intializeSemaphores();
    fflush(stdout);
}

int main()
{
    init_code();
    pthread_t leftThread, rightThread;
    // pthread_create(&rightThread, NULL, getRightFunc, (void *)&rightPID);
    // pthread_join(rightThread, NULL);
    getRightFunc(&rightPID);
    getLeftFunc(&leftPID);
}