#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
struct msg_struct_t
{
    long type;
    char msgBuffer[100];
};
typedef struct msg_struct_t msg_struct;
int main()
{
    key_t key = ftok("channel2", 69);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    int senderType = 69;
    while (1)
    {
        msg_struct message;
        msgrcv(msgid, &message, sizeof(message.msgBuffer), senderType, 0);
        printf("Message recieved '%s'\n", message.msgBuffer);
    }
}