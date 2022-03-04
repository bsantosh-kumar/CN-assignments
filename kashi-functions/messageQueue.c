#include <stdio.h>
#include <sys/msg.h>
#include <stdbool.h>

char *MessageQueue = "/tmp/sample";
#define ID 101

struct messageBuffer{
    long mtype;
    char mtext[50];
};

int create_message_queue (char name[], int id, bool del) {
    key_t key;
    int msqid;

    if ( (key = ftok (name, id)) == -1) {
        perror ("Error in generating key");
        exit (0);
    }

    if ( (msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror ("Error in msgget");
        exit (0);
    }

    if (del) {
        if ( (msgctl(msqid, IPC_RMID, NULL)) == -1) {
            perror ("Error in msgctl");
            exit (0);
        }

        if ( (msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
            perror ("Error in msgget");
            exit (0);
        }
    }

    return msqid;
}

int main () {
    int msqid;
    struct messageBuffer message;

    msqid = create_message_queue (MessageQueue, ID, true);

    return 0;
}