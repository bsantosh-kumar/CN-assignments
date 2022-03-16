#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/poll.h>
#include <time.h>
#define N 0
#define D 3
#define SA struct sockaddr

int bindConctOrntdUSServer(char *pathName) //bind connection oriented unix socket server
{
    int usfd;
    struct sockaddr_un userv_addr, ucli_addr;
    int userv_len, ucli_len;

    if ((usfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in socket creation\n");
        exit(0);
    }
    bzero(&userv_addr, sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, pathName);
    unlink(pathName);
    userv_len = sizeof(userv_addr);
    if (bind(usfd, (struct sockaddr *)&userv_addr, userv_len) == -1)
    {
        perror("server: bind");
        exit(0);
    }
    if (listen(usfd, 5) < 0)
    {
        printf("Error in listen\n");
        exit(0);
    }
    return usfd;
}

int recv_fd(int socket)
{
    int sent_fd, available_ancillary_element_buffer_space;
    struct msghdr socket_message;
    struct iovec io_vector[1];
    struct cmsghdr *control_message = NULL;
    char message_buffer[1];
    char ancillary_element_buffer[CMSG_SPACE(sizeof(int))]; /* start clean */
    memset(&socket_message, 0, sizeof(struct msghdr));
    memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int))); /* setup a place to fill in message contents */
    io_vector[0].iov_base = message_buffer;
    io_vector[0].iov_len = 1;
    socket_message.msg_iov = io_vector;
    socket_message.msg_iovlen = 1; /* provide space for the ancillary data */
    socket_message.msg_control = ancillary_element_buffer;
    socket_message.msg_controllen = CMSG_SPACE(sizeof(int));
    if (recvmsg(socket, &socket_message, MSG_CMSG_CLOEXEC) < 0)
        return -1;
    if (message_buffer[0] != 'F')
    { /* this did not originate from the above function */
        return -1;
    }
    if ((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
    { /* we did not provide enough space for the ancillary element array */
        return -1;
    }
    /* iterate ancillary elements */ for (control_message = CMSG_FIRSTHDR(&socket_message); control_message != NULL; control_message = CMSG_NXTHDR(&socket_message, control_message))
    {
        if ((control_message->cmsg_level == SOL_SOCKET) && (control_message->cmsg_type == SCM_RIGHTS))
        {
            sent_fd = *((int *)CMSG_DATA(control_message));
            return sent_fd;
        }
    }
    return -1;
}

void serve_vehicle(int nsfd, int *total_amount)
{
    char buff[200];

    int amount = 0;
    while (amount == 0)
    {
        bzero(buff, sizeof(buff));
        strcpy(buff, "What is the amount?(if entered zero/invalid you will promted again)");
        write(nsfd, buff, 128);
        bzero(buff, sizeof(buff));
        read(nsfd, buff, 128);
        amount = atoi(buff);
        *total_amount += amount;
    }
    bzero(buff, sizeof(buff));
    strcpy(buff, "exit");
    write(nsfd, buff, 128);
    printf("Total amount is: %d\n", *total_amount);
    return;
}

int main(int argc, char *argv[])
{
    int portNo = 40500 + N;
    if (argc == 2)
        portNo = atoi(argv[1]);
    char name[200];
    bzero(name, sizeof(name));
    sprintf(name, "B%d.usd", N + 1);
    int usfd = bindConctOrntdUSServer(name);
    struct pollfd pfd[D];
    for (int i = 0; i < D; i++)
    {
        struct sockaddr_un uforeign_addr;
        int uforeign_len = sizeof(uforeign_addr);
        if ((pfd[i].fd = accept(usfd, (SA *)&uforeign_addr, &uforeign_len)) == -1)
        {
            perror("Error in accept\n");
        }
        pfd[i].events = POLLIN;
    }
    printf("Connected with every Dispensary\n");
    fflush(stdout);
    int total_amount = 0;
    while (1)
    {
        poll(pfd, D, -1);
        for (int i = 0; i < D; i++)
        {
            if (pfd[i].fd == -1)
                continue;
            if (pfd[i].revents & POLLIN)
            {
                int nsfd = recv_fd(pfd[i].fd);
                serve_vehicle(nsfd, &total_amount);
            }
            if (pfd[i].revents & POLLHUP)
            {
                pfd[i].fd = -1;
            }
        }
    }
}