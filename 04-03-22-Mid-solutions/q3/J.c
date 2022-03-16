#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <sys/poll.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <signal.h>
#define R 3
#define MAX_STU 100
#define MAX_SEL 20
#define SA struct sockaddr
bool getMQId(char *name, int *mPtr) //create a msg queue if not present, it returns whether it was present previously
{
    key_t keyForPIDQ = ftok(name, 69);
    int mQId = msgget(keyForPIDQ, 0666 | IPC_CREAT | IPC_EXCL);
    bool ans = false;
    if (mQId == -1)
    {
        mQId = msgget(keyForPIDQ, 0);
        ans = true;
    }
    *mPtr = mQId;
    return ans;
}

struct stu_score_t
{

    int score;
    int stu;
    int rounds;
};

struct message_struct_t
{
    long type;
    struct stu_score_t stu_info;
};
typedef struct message_struct_t message_struct;

int get_pid_from_name(char pName[])
{
    char command[200];
    bzero(command, sizeof(command));
    strcat(command, "pidof ");
    strcat(command, pName);
    int temp = fileno(popen(command, "r"));
    char buff[200];
    bzero(buff, sizeof(buff));
    read(temp, buff, 128);
    return atoi(buff);
}

int main(int argc, char *argv[])
{
    int stu_scores[MAX_STU][2]; //0 is for number of rounds, 1 is the score
    int selected_stus = 0;
    int threshold = 20; //out of 30 the score to be achieved
    bzero(stu_scores, sizeof(stu_scores));
    int at_pid = get_pid_from_name("AT.out");
    int mQId;
    getMQId("results", &mQId);
    while (1)
    {
        message_struct rcvMsg;
        if (selected_stus >= MAX_SEL)
            signal(at_pid, SIGUSR1);
        msgrcv(mQId, &rcvMsg, sizeof(rcvMsg.stu_info), 420, 0);
        struct stu_score_t curr_stu;
        if (stu_scores[curr_stu.stu][0] < 3 && stu_scores[curr_stu.stu][0] != -1)
        {
            stu_scores[curr_stu.stu][1] += curr_stu.score;
            stu_scores[curr_stu.stu][0]++;
        }
        else
        {
            printf("%d already evaluated\n", curr_stu.stu);
            fflush(stdout);
        }
        if (stu_scores[curr_stu.stu][0] == 3)
        {
            stu_scores[curr_stu.stu][0] = -1;
            if (stu_scores[curr_stu.stu][1] >= threshold)
            {
                printf("%d is selected\n", curr_stu.stu);
                fflush(stdout);
                selected_stus++;
            }
        }
    }
}