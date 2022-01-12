
/*
gcc p2.c -o p2  && gcc main.c  && ./a.out
*/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <stdlib.h>

int main()
{
  int fds[3];
  fds[0] = 0;
  FILE *f = popen("./p2", "r");
  fds[1] = fileno(f);
  fds[2] = open("./temp.txt", O_RDONLY | O_CREAT, 0666);

  printf("%d %d %d\n", fds[0], fds[1], fds[2]);

  struct pollfd plfd[3];
  for (int i = 0; i < 3; i++)
  {
    plfd[i].fd = fds[i];
    plfd[i].events = POLLIN;
  }
  int nfds = 3;
  int noepnfds = 3;
  while (noepnfds > 0)
  {
    int ready = poll(plfd, nfds, -1);
    if (ready == -1)
      return 0;
    for (int i = 0; i < 3; i++)
    {
      // https://www.geeksforgeeks.org/difference-between-malloc-and-calloc-with-examples/
      printf("Came until here\n");
      char *buff = (char *)calloc(110, sizeof(char));

      if (plfd[i].revents != 0)
      {
        printf("  fd=%d; events: %s%s%s\n", plfd[i].fd,
               (plfd[i].revents & POLLIN) ? "POLLIN " : "",
               (plfd[i].revents & POLLHUP) ? "POLLHUP " : "",
               (plfd[i].revents & POLLERR) ? "POLLERR " : "");
        fflush(stdout);
        if (plfd[i].revents & POLLHUP)
        {
          close(plfd[i].fd);
          printf("Closed1\n");
          plfd[i].fd = -1;
          noepnfds--;
        }
        else if (plfd[i].revents & POLLIN)
        {
          int x = read(plfd[i].fd, buff, 100);
          if (x == 0)
          {
            close(plfd[i].fd);
            printf("Closed2\n");
            plfd[i].fd = -1;
            noepnfds--;
          }
          printf("%s\n", buff);
          fflush(stdout);
        }
      }
      else
      {
        printf("i: %d revent is zero\n", i);
      }
      free(buff);
    }
  }
  return 0;
}
// https://man7.org/linux/man-pages/man2/poll.2.html

/*
Thank you for using our services...

Service charges : 500/-
G-Pay : 6302643684

:D
*/