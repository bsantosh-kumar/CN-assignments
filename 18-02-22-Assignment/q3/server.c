#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <stdbool.h>
#include <fcntl.h>
#include <regex.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#define SA struct sockaddr
#define TCP_PORT 50000
#define ALL_PORTS 65336
#define FIFO_NAME "P2-FIFO"

int clientFDs[100];
int startClient = 0;
int endClient = -1;

int bindAndListenTcp(int portNo)
{
    int sFD = 0;
    int *sFDPtr = &sFD;
    if ((*sFDPtr = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating a socket\n");
        exit(0);
    }
    int True = 1;
    if (setsockopt(*sFDPtr, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(int)) < 0)
    {
        printf("Error in sock option errorno:%d\n error:%s", errno, strerror(errno));
        exit(0);
    }
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(portNo);

    if (bind(*sFDPtr, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error in bind, error no:%d, error is: %s\n", errno, strerror(errno));
        exit(0);
    }
    if (listen(*sFDPtr, 10) < 0)
    {
        perror("Error in listen\n");
        exit(0);
    }
    return sFD;
}

void acceptClient(int tcpSFD)
{
    int nsfd;
    struct sockaddr_in tcli_addr;
    int tcli_len = sizeof(tcli_addr);
    if ((nsfd = accept(tcpSFD, (SA *)&tcli_addr, (socklen_t *)&tcli_len)) == -1)
    {
        perror("Error in accept\n");
        return;
    }

    clientFDs[++endClient] = nsfd;
    printf("Accepted a client\n");
    fflush(stdout);
}

int getFIFOfd(char fifoName[])
{
    if (mkfifo(fifoName, 0666) == -1)
    {
        printf("mkfifo error\n");
        perror(strerror(errno));
        return 1;
    }
    int rffd = open(fifoName, O_RDONLY);
    return rffd;
}

int getFDofP1()
{
    int pfd[2];
    pipe(pfd);
    int child = fork();
    if (child > 0)
    {
        close(pfd[1]);
        return pfd[0];
    }
    else
    {
        close(pfd[0]);
        char write_end[100];
        sprintf(write_end, "%d", pfd[1]);
        char *args[] = {"./P1.out", write_end, NULL};
        execvp(args[0], args);
        exit(EXIT_FAILURE);
    }
}

int getFDofP2()
{
    return getFIFOfd(FIFO_NAME);
}

int getFDofP3()
{
    return fileno(popen("./P3.out", "r"));
}

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

void sigHandler(int signal)
{
    if (startClient > endClient)
        return;
    int currFD = clientFDs[startClient++];
    int child = fork();
    if (child > 0)
    {
        char buff[200];
        bzero(buff, sizeof(buff));
        strcpy(buff, "toEcho");
        write(currFD, buff, 128);
        return;
    }
    else
    {
        dup2(currFD, STDIN_FILENO);
        dup2(currFD, STDOUT_FILENO);
        char *args[] = {"./echo.out", NULL};
        execvp(args[0], args);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    signal(SIGUSR1, sigHandler);
    unlink(FIFO_NAME);
    int tcpSFD = bindAndListenTcp(TCP_PORT);
    struct pollfd pollFDs[4];
    pollFDs[0].fd = 0;
    pollFDs[1].fd = getFDofP1();
    pollFDs[2].fd = getFDofP2();
    pollFDs[3].fd = getFDofP3();
    pollFDs[4].fd = tcpSFD;

    for (int i = 0; i < 5; i++)
        pollFDs[i].events = POLLIN;
    int npfds = 5;
    char buffPoll[200];
    while (1)
    {
        poll(pollFDs, npfds, -1);
        int currStart = startClient;
        int currEnd = endClient;
        bool toExit = false;
        for (int i = 0; i < npfds; i++)
        {
            if (pollFDs[i].revents == 0)
                continue;
            if (pollFDs[i].fd == -1)
                continue;
            if (pollFDs[i].revents | POLLIN)
            {
                bzero(buffPoll, sizeof(buffPoll));
                if (i == 4)
                {
                    maskSignal(SIGUSR1);
                    acceptClient(pollFDs[i].fd);
                    unmaskSignal(SIGUSR1);
                    continue;
                }
                if (i != 0)
                {
                    read(pollFDs[i].fd, buffPoll, 128);
                }
                else
                {
                    scanf("%s", buffPoll);
                    if (strcmp(buffPoll, "exit") == 0)
                        exit(0);
                }
                maskSignal(SIGUSR1);
                for (int i = startClient; i <= endClient; i++)
                {
                    write(clientFDs[i], buffPoll, 128);
                }
                unmaskSignal(SIGUSR1);
                fflush(stdout);
            }
        }
        if (toExit)
        {
            break;
        }
    }
    exit(0);
}