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
#define B 2
#define SA struct sockaddr

int bindAndListenTcp(int portNo)
{
    int sFD = 0;
    int *sFDPtr = &sFD;
    printf("Trying to create sfd\n");
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

int connectConctOrntdUSClient(char *localPath, char *foreignPath)
{
    int usfd;
    struct sockaddr_un uforeign_addr, ulocal_addr;

    int uforeign_len, ulocal_len;
    if ((usfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in socket\n");
        exit(0);
    }

    bzero(&ulocal_addr, sizeof(ulocal_addr));
    ulocal_addr.sun_family = AF_UNIX;
    strcpy(ulocal_addr.sun_path, localPath);

    ulocal_len = sizeof(ulocal_addr);
    bzero(&uforeign_addr, sizeof(uforeign_addr));
    uforeign_addr.sun_family = AF_UNIX;
    strcpy(uforeign_addr.sun_path, foreignPath);

    uforeign_len = sizeof(uforeign_addr);
    if (connect(usfd, (struct sockaddr *)&uforeign_addr, uforeign_len) == -1)
    {
        perror("Error in connect\n");
        exit(0);
    }
    return usfd;
}

int send_fd(int socket, int fd_to_send)
{
    struct msghdr socket_message;
    struct iovec io_vector[1];
    struct cmsghdr *control_message = NULL;
    char message_buffer[1]; /* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
    char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
    int available_ancillary_element_buffer_space; /* at least one vector of one byte must be sent */
    message_buffer[0] = 'F';
    io_vector[0].iov_base = message_buffer;
    io_vector[0].iov_len = 1; /* initialize socket message */
    memset(&socket_message, 0, sizeof(struct msghdr));
    socket_message.msg_iov = io_vector;
    socket_message.msg_iovlen = 1; /* provide space for the ancillary data */
    available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
    memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
    socket_message.msg_control = ancillary_element_buffer;
    socket_message.msg_controllen = available_ancillary_element_buffer_space; /* initialize a single ancillary data element for fd passing */
    control_message = CMSG_FIRSTHDR(&socket_message);
    control_message->cmsg_level = SOL_SOCKET;
    control_message->cmsg_type = SCM_RIGHTS;
    control_message->cmsg_len = CMSG_LEN(sizeof(int));
    *((int *)CMSG_DATA(control_message)) = fd_to_send;
    return sendmsg(socket, &socket_message, 0);
}

void serve_vehicle(int nsfd)
{
    sleep(rand() % 3);
}

int main(int argc, char *argv[])
{
    srand(time(0));
    int portNo = 50500 + N;
    if (argc == 2)
    {
        portNo = atoi(argv[1]);
    }
    char *billing_names[] = {"B1.usd", "B2.usd"};
    int bill_nusfd[B];
    for (int i = 0; i < B; i++)
    {
        char name[200];
        bzero(name, sizeof(name));
        sprintf(name, "./D%d.usd", N);

        bill_nusfd[i] = connectConctOrntdUSClient(name, billing_names[i]);
    }
    int sfd = bindAndListenTcp(portNo);
    int nsfd;

    while (1)
    {
        struct sockaddr_in foriegn_addr;
        int foreign_len = sizeof(foriegn_addr);
        if ((nsfd = accept(sfd, (SA *)&foriegn_addr, &foreign_len)) > 0)
        {
            serve_vehicle(nsfd);
            int b_index = rand() % 2;
            char buff[200];
            bzero(buff, sizeof(buff));
            sprintf(buff, "Go to billing %d\n", b_index);
            write(nsfd, buff, 128);
            send_fd(bill_nusfd[b_index], nsfd);
        }
    }
}