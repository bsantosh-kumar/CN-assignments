#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
struct message_struct
{
    long type;
    char content[100];
};
int main(int argc, char *argv[])
{
    printf("Hello I am %s\n", argv[0]);
    key_t keyForPIDQ = ftok("pidQueue", 69);
    int mQId = msgget(keyForPIDQ, 0666 | IPC_CREAT);
    struct message_struct pidSendMsg;
    pidSendMsg.type = 2;
    sprintf(pidSendMsg.content, "%d", getpid());
    msgsnd(mQId, &pidSendMsg, sizeof(pidSendMsg.content), 0);
    printf("%s sent '%s' to P1\n", argv[0], pidSendMsg.content);
    // key_t keyForRcvP1 = ftok("pidQueue",getpid()
    struct message_struct rcvMsg;
    msgrcv(mQId, &rcvMsg, sizeof(rcvMsg.content), getpid(), 0);
    printf("%s recieved message '%s' from P1\n", argv[0], rcvMsg.content);
}