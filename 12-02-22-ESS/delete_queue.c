#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>

struct msg_struct_t
{
    long type;
    char msgBuffer[100];
};
typedef struct msg_struct_t msg_struct;
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        perror("Format is ./delete_queue <message queue name>\n");
        exit(EXIT_FAILURE);
    }
    key_t key = ftok(argv[1], 69);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    struct msqid_ds *buf = (struct msqid_ds *)malloc(sizeof(struct msqid_ds));
    int returnVal = msgctl(msgid, IPC_RMID, buf);
    printf("%d", returnVal);
    if (returnVal == -1)
    {
        perror("Error in deleting the queue\n");
        exit(EXIT_FAILURE);
    }
}