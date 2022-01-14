#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MessageQueue "/tmp/sample"
#define ID 101

struct messageBuffer
{
    long mtype;
    char mtext[50];
};

int main()
{
    key_t key;
    int msqid;
    struct messageBuffer message;

    int pid = getpid();

    key = ftok(MessageQueue, ID);

    msqid = msgget(key, 0666 | IPC_CREAT);

    message.mtype = 2;
    sprintf(message.mtext, "%d", pid);

    msgsnd(msqid, &message, sizeof(message.mtext), 0);

    msgrcv(msqid, &message, sizeof(message), pid, 0);

    printf("Message received : %s\n", message.mtext);
    fflush(stdout);

    return 0;
}