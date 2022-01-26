#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <semaphore.h>
int main(int argc, char *argv[])
{
    kill(atoi(argv[1]), SIGUSR1);
}