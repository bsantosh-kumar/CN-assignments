#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

#define PRGNO 2
int leftPID = 0, rightPID = 0;

struct message_struct
{
    long mtype;
    char mtext[100];
};

int getNextProcess()
{
    return PRGNO % 4 + 1;
}
int getPreviousProcess()
{
    if ((PRGNO - 1 + 4) % 4 == 0)
        return 4;
    else
        return (PRGNO - 1 + 4) % 4;
}
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
void initHandler(int signo, siginfo_t *info, void *context)
{
    int recievedPID = info->si_pid;
    leftPID = recievedPID;
    printf("P%d go to know it's left is: %d\n", PRGNO, leftPID);
}
sem_t *previousSem, *currSem, *nextSem;
void intializeHandler(int signal, void (*handler)(int, siginfo_t *, void *))
{
    struct sigaction act = {0};
    act.sa_flags = SA_SIGINFO;

    act.sa_sigaction = handler;
    if (sigaction(signal, &act, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void circularHandler(int signo, siginfo_t *info, void *context)
{
    int recievedPID = info->si_pid;
    if (recievedPID == getpid())
        return;
    printf("Recieved signal from process, %d\n", recievedPID);

    fflush(stdout);
}

void intializeSemaphores(int i)
{
    char previousNum[10];
    char currNum[10];
    char nextNum[10];
    sprintf(previousNum, "%d-%d", getPreviousProcess(), i);
    sprintf(currNum, "%d-%d", PRGNO, i);
    sprintf(nextNum, "%d-%d", getNextProcess(), i);
    previousSem = sem_open(previousNum, O_CREAT, 0666, 0);
    currSem = sem_open(currNum, O_CREAT, 0666, 0);
    nextSem = sem_open(nextNum, O_CREAT, 0666, 0);
    // printf("previousSem:%s, currSEm:%s, nextSem:%s\n", previousNum, currNum, nextNum);
}
int getMessageID()
{

    key_t key = ftok("makefile", 69);
    int msgID = msgget(key, 0666 | IPC_CREAT);
    return msgID;
}
void sendMessage(int msgID, struct message_struct message)
{
    msgsnd(msgID, &message, sizeof(message.mtext), 0);
    // printf("P%d sent message with text %s\n", PRGNO, message.mtext);
}
struct message_struct recieveMessage(int msgID, int type)
{
    struct message_struct message;
    msgrcv(msgID, &message, sizeof(message.mtext), type, 0);
    // printf("mtext is:%s, mtype:%d type:%d\n", message.mtext, message.mtype, type);
    return message;
}

void *getLeftFunc(void *voidPtr)
{
    maskSignal(SIGUSR1);
    sem_wait(currSem);
    unmaskSignal(SIGUSR1);
}
void init_code()
{
    for (int i = 1; i <= 4; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            char currName[10];
            sprintf(currName, "%d-%d", i, j);
            sem_unlink(currName);
        }
    }
    int msgID = getMessageID();
    msgctl(msgID, IPC_RMID, NULL);
    printf("Hi I am P%d PID:%d\n", PRGNO, getpid());
    intializeHandler(SIGUSR1, initHandler);
    intializeHandler(SIGUSR2, circularHandler);
    maskSignal(SIGUSR2);
    intializeSemaphores(0);
}
int getP3(int msgID)
{
    return 0;
}
void sendP1(int msgID, int p1ID)
{
    struct message_struct message;
    message.mtype = 1;
    sprintf(message.mtext, "%d", p1ID);
    sendMessage(msgID, message);
}
void *getRightFunc(void *voidPtr)
{
    int *rightPtr = (int *)voidPtr;
    int msgID = getMessageID();
    struct message_struct recievedMessage = recieveMessage(msgID, 3);
    *rightPtr = atoi(recievedMessage.mtext);
    kill(*rightPtr, SIGUSR1);
    sem_post(nextSem);
    printf("P%d right is:%d\n", PRGNO, *rightPtr, recievedMessage.mtype);
}

void sendMessageWithContent(int msgID, int type, int content)
{
    struct message_struct message;
    message.mtype = type;
    sprintf(message.mtext, "%d", content);
    sendMessage(msgID, message);
}
void sendSignalToNext()
{
    kill(rightPID, SIGUSR1);
    sem_post(nextSem);
}
int main()
{
    init_code();
    int msgID = getMessageID();
    // printf("Done initialisation\n");
    fflush(stdout);
    sendMessageWithContent(msgID, 2, getpid());
    fflush(stdout);

    pthread_t leftThread, rightThread;
    // pthread_create(&leftThread, NULL, getLeftFunc, (void *)&leftPID);
    // pthread_create(&rightThread, NULL, getRightFunc, (void *)&rightPID);
    // pthread_join(leftThread, NULL);
    // pthread_join(rightThread, NULL);
    getLeftFunc(&leftPID);
    getRightFunc(&rightPID);
    sendMessageWithContent(msgID, 1, leftPID);
    sendMessageWithContent(msgID, 18, getpid());
    // sendSignalToNext();
    recieveMessage(msgID, PRGNO);
}
