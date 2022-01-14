#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>

using namespace std;

int main()
{

    char s1[10] = "FIFO2";
    mkfifo(s1, 0666);
    int fd2 = open(s1, O_RDONLY);

    // char *s2 = "FIFO3";
    // mkfifo(s2, 0666);
    // int fd3 = open(s2, 'r');

    // char *s3 = "FIFO4";
    // mkfifo(s3, 0666);
    // int fd4 = open(s3, 'r');

    struct pollfd pfd[3];

    pfd[0].fd = fd2;
    // pfd[1].fd = fd3;
    // pfd[2].fd = fd4;

    pfd[0].events = POLLIN;
    pfd[1].events = POLLIN;
    pfd[2].events = POLLIN;
    pfd[1].fd = -1;
    pfd[2].fd = -1;
    cout << "Hell\n";
    while (1)
    {
        printf("Before poll\n");
        poll(pfd, 3, -1);
        cout << "Hell-1\n";
        for (int i = 0; i < 1; i++)
        {
            char st[1000] = {0};
            if (pfd[i].revents && POLLIN)
            {
                int c = read(pfd[i].fd, st, 1000);

                cout << "Process P" + to_string(i + 2) + " : " << st << endl;
                fflush(stdout);
            }
            else
            {
                cout << "Hell-else\n";
            }
        }
    }

    return 0;
}