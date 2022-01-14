#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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

    int p2;
    int p3;
    int p4;

    key = ftok(MessageQueue, ID);

    msqid = msgget(key, 0666 | IPC_CREAT);

    msgrcv(msqid, &message, sizeof(message.mtext), 2, 0);
    p2 = atoi(message.mtext);
    printf("PID for P2 : %d\n", p2);
    fflush(stdout);

    // msgrcv (msqid, &message, sizeof(message), 3, 0);
    // p3 = atoi (message.mtext);
    // printf ("PID for P3 : %d\n", p3);
    // fflush (stdout);

    // msgrcv (msqid, &message, sizeof(message), 4, 0);
    // p4 = atoi (message.mtext);
    // printf ("PID for P4 : %d\n", p4);
    // fflush (stdout);

    message.mtype = p2;
    sprintf(message.mtext, "Message from P1 to P2");
    msgsnd(msqid, &message, sizeof(message), 0);

    // message.mtype = p3;
    // sprintf (message.mtext, "%d", "Message from P1 to P3");
    // msgsnd(msqid, &message, sizeof(message), 0);

    // message.mtype = p4;
    // sprintf (message.mtext, "%d", "Message from P1 to P4");
    // msgsnd(msqid, &message, sizeof(message), 0);

    return 0;
}