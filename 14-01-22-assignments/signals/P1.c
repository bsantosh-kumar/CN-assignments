#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <pthread.h>
#define PRGNO 1
struct message_struct
{
    long mtype;
    char mtext[100];
};

void handler(int signo, siginfo_t *info, void *context)
{
    int recievedPID = info->si_pid;
    printf("P%d recieved signal from %d\n", PRGNO, recievedPID);
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
    printf("P%d right is:%d type:%d\n", PRGNO, *rightPtr, recievedMessage.mtype);
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
    printf("Hi I am P%d PID:%d\n", PRGNO, getpid());
    fflush(stdout);
    intializeHandler();
    fflush(stdout);
}

int main()
{
    init_code();
    int leftPID = 0, rightPID = 0;
    pthread_t leftThread, rightThread;
    // pthread_create(&rightThread, NULL, getRightFunc, (void *)&rightPID);
    // pthread_join(rightThread, NULL);
    getRightFunc(&rightPID);
    getLeftFunc(&leftPID);
}