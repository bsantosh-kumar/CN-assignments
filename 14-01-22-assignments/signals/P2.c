#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define PRGNO 2

struct message_struct
{
    long mtype;
    char mtext[100];
};

void handler(int signo, siginfo_t *info, void *context)
{
    int recievedPID = info->si_pid;
    printf("In handler P%d recieved signal from %d\n", PRGNO, recievedPID);
}
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

int getMsgID()
{

    key_t key = ftok("makefile", 69);
    int msgID = msgget(key, 0666 | IPC_CREAT);
    return msgID;
}
void sendMessage(int msgID, struct message_struct message)
{
    msgsnd(msgID, &message, sizeof(message.mtext), 0);
    printf("P%d sent message with text %s\n", PRGNO, message.mtext);
}
struct message_struct recieveMessage(int msgID, int type)
{
    struct message_struct message;
    msgrcv(msgID, &message, sizeof(message.mtext), type, 0);
    printf("mtext is:%s, mtype:%d type:%d\n", message.mtext, message.mtype, type);
    return message;
}

void *getLeftFunc(void *voidPtr)
{
    int *leftPtr = (int *)voidPtr;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    siginfo_t rcvSignalInfo;
    int waitReturnVal = sigwaitinfo(&set, &rcvSignalInfo);
    *leftPtr = rcvSignalInfo.si_pid;
    printf("P%d go to know it's left is: %d\n", PRGNO, *leftPtr);
}
void init_code()
{
    int msgID = getMsgID();
    msgctl(msgID, IPC_RMID, NULL);
    printf("Hi I am P%d PID:%d\n", PRGNO, getpid());
    intializeHandler();
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
void sendUSR1ToP3(int p3ID)
{
    kill(p3ID, SIGUSR1);
}

void *getRightFunc(void *voidPtr)
{
    int *rightPtr = (int *)voidPtr;
    int msgID = getMsgID();
    struct message_struct recievedMessage = recieveMessage(msgID, 3);
    *rightPtr = atoi(recievedMessage.mtext);
    kill(*rightPtr, SIGUSR1);
    printf("P%d right is:%d", PRGNO, *rightPtr);
}
int main()
{
    init_code();
    int msgID = getMsgID();
    printf("Done initialisation\n");
    fflush(stdout);
    int leftPID = 0, rightPID = 0;

    struct message_struct message;
    message.mtype = 2;
    sprintf(message.mtext, "%d", getpid());
    sendMessage(msgID, message);
    fflush(stdout);

    pthread_t leftThread, rightThread;
    // pthread_create(&leftThread, NULL, getLeftFunc, (void *)&leftPID);
    // pthread_create(&rightThread, NULL, getRightFunc, (void *)&rightPID);
    // pthread_join(leftThread, NULL);
    // pthread_join(rightThread, NULL);
    getLeftFunc(&leftPID);
    getRightFunc(&rightPID);
    sendP1(msgID, leftPID);
}