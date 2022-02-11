#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>

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

int main()
{
    int usfd = bindConctOrntdUSServer("first-unix-socket");
    int nusfd;
    struct sockaddr_un ucli_addr;
    int cli_len;
    cli_len = sizeof(ucli_addr);
    char *buff = (char *)malloc(200 * sizeof(char));
    bzero(buff, sizeof(buff));
    // while (1)
    {
        printf("Server is waiting to accept\n");
        if ((nusfd = accept(usfd, (struct sockaddr *)&ucli_addr, &cli_len)) < 0)
        {
            perror("Error in accept\n");
            exit(0);
        }
        printf("Connected\n");

        int fileFD = open("temp.txt", O_RDONLY);
        read(fileFD, buff, 5);
        printf("Read '%s'\n", buff);
        send_fd(nusfd, fileFD);
    }
}