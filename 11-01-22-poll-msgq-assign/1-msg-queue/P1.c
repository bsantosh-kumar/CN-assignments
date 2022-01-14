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
    for (int i = 2; i <= 4; i++)
    {
        struct message_struct rcvMsg;
        msgrcv(mQId, &rcvMsg, sizeof(rcvMsg.content), i, 0);
        printf("P1 recieved '%s' from P%d\n", rcvMsg.content, i);
        struct message_struct sndMsg;
        sndMsg.type = atoi(rcvMsg.content);
        sprintf(sndMsg.content, "Message from P1 to P%d\n", i);
        msgsnd(mQId, &sndMsg, sizeof(sndMsg.content), 0);
    }
}