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

void circularHandler(int signo, siginfo_t *info, void *context)
{
    int recievedPID = info->si_pid;
    if (recievedPID == getpid())
        return;
    printf("Recieved signal from process, %d\n", recievedPID);

    fflush(stdout);
}

void sendMessage(int msgID, struct message_struct message)
{
    msgsnd(msgID, &message, sizeof(message.mtext), 0);
    // printf("P%d sent message with text %s\n", PRGNO, message.mtext);
}
sem_t *previousSem, *currSem, *nextSem;

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
    printf("P%d right is:%d\n", PRGNO, *rightPtr, recievedMessage.mtype);
}
void *getLeftFunc(void *voidPtr)
{
    maskSignal(SIGUSR1);
    sem_wait(currSem);
    unmaskSignal(SIGUSR1);
}
void init_code()
{
    printf("Hi I am P%d PID:%d\n", PRGNO, getpid());
    fflush(stdout);
    intializeHandler(SIGUSR1, initHandler);
    intializeHandler(SIGUSR2, circularHandler);
    maskSignal(SIGUSR2);
    intializeSemaphores(0);
    fflush(stdout);
}

int allPIDs[4] = {0};
void makeGroupFromMessages()
{
    int gpid = getpid();
    int msgID = getMessageID();
    if (setpgid(getpid(), gpid) == -1)
        printf("Unsucessful setting group of %d as %d\n", getpid(), gpid);
    else
        printf("Sucessful setting group of %d as %d\n", getpid(), gpid);
    allPIDs[0] = getpid();
    for (int i = 0; i < 3; i++)
    {
        struct message_struct currMessage = recieveMessage(msgID, 18);
        int currProcess = atoi(currMessage.mtext);
        if (setpgid(currProcess, gpid) == -1)
            printf("Unsucessful setting group of %d as %d\n", getpid(), gpid);
        else
            printf("Sucessful setting group of %d as %d\n", getpid(), gpid);
        allPIDs[i + 1] = currProcess;
    }
}
void sendMessageWithContent(int msgID, int type, int content)
{
    struct message_struct message;
    message.mtype = type;
    sprintf(message.mtext, "%d", content);
    sendMessage(msgID, message);
}
void sendGroupIDToOthers()
{
    int msgID = getMessageID();
    int gpid = getpid();
    for (int i = 2; i <= 4; i++)
        sendMessageWithContent(msgID, i, gpid);
}
int main()
{
    init_code();
    pthread_t leftThread, rightThread;
    // pthread_create(&rightThread, NULL, getRightFunc, (void *)&rightPID);
    // pthread_join(rightThread, NULL);
    getRightFunc(&rightPID);
    getLeftFunc(&leftPID);
    makeGroupFromMessages();
    sendGroupIDToOthers();
}