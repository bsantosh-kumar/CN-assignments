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

#define PRGNO 4

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
    printf("P%d sent message with text %s type:%d\n", PRGNO, message.mtext, message.mtype);
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
struct message_struct recieveMessage(int msgID, int type)
{
    struct message_struct message;
    msgrcv(msgID, &message, sizeof(message.mtext), type, 0);
    return message;
}
void *getRightFunc(void *voidPtr)
{
    int *rightPtr = (int *)voidPtr;
    int msgID = getMsgID();
    struct message_struct recievedMessage = recieveMessage(msgID, 1);
    *rightPtr = atoi(recievedMessage.mtext);
    kill(*rightPtr, SIGUSR1);
    printf("P%d right is:%d", PRGNO, *rightPtr);
}
void init_code()
{
    printf("Hi I am P%d PID:%d\n", PRGNO, getpid());
    intializeHandler();
}

int main()
{
    init_code();
    printf("Done initialisation\n");
    fflush(stdout);

    int leftPID = 0, rightPID = 0;
    // pthread_t leftThread, rightThread;
    // pthread_create(&leftThread, NULL, getLeftFunc, (void *)&leftPID);
    int msgID = getMsgID();
    struct message_struct message;
    message.mtype = 4;
    sprintf(message.mtext, "%d", getpid());
    sendMessage(msgID, message);
    // pthread_join(leftThread, NULL);
    // pthread_join(rightThread, NULL);
    getLeftFunc(&leftPID);
    getRightFunc(&rightPID);
}