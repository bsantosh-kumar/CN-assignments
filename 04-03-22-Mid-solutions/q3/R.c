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

#define N 0

int connectConctOrntdUSClient(char *foreignPath)
{
    int usfd;
    struct sockaddr_un uforeign_addr, ulocal_addr;

    int uforeign_len;
    if ((usfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in socket\n");
        exit(0);
    }
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

int main(int argc, char *argv[])
{
    int next_num_round = (N + 1) % 3;
    int previous_num_round = (3 + N - 1) % 3;
    char next_server_name[200];
    char curr_server_name[200];
    sprintf(next_server_name, "R%d-server.out", next_num_round);
    sprintf(curr_server_name, "R%d-server.out", N);
    char *at_name = "at.usd";
    int at_usfd = connectConctOrntdUSClient(at_name);
    int next_usfd;
    int previous_usfd;
    get_usfds(next_server_name, curr_server_name, &next_usfd, &previous_usfd);
    int mQId;
    getMQId("results", &mQId);
    struct pollfd pfds[3];
    pfds[0].fd = at_usfd;
    pfds[1].fd = next_usfd;
    pfds[2].fd = previous_usfd;
    for (int i = 0; i < 3; i++)
        pfds[i].events = POLLIN;
    while (1)
    {
        poll(pfds, 3, -1);
        for (int i = 0; i < 3; i++)
        {
            if (i == 0)
            {
                char buff[200];
                bzero(buff, sizeof(buff));
                read(pfds[i].fd, buff, sizeof(buff));
                int stu_name;
                stu_name = atoi(buff);
                int nusfd = recv_fd(pfds[i].fd);
                serve_stu(nusfd);
                struct stu_score_t curr_stu;
                curr_stu.rounds |= 1 >> N;
                curr_stu.score = rand() % 10 + 1;
                curr_stu.stu = stu_name;
                write(next_usfd, &curr_stu, sizeof(curr_stu));
                send_fd(next_usfd, nusfd);
            }
        }
    }
}