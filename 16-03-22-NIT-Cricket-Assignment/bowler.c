#define __USE_BSD /* use bsd'ish ip header */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#define __FAVOR_BSD /* use bsd'ish tcp header */
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <signal.h>
#include <time.h>
#define SA struct sockaddr
#define BOW_UMP "bowler_umpire.uds"
#define BAT_UMP "batsman_umpire.uds"
#define UMP "umpire.uds"
#define BOW "bowler.uds"
#define BAT_BOW "batsman_bowler.uds"
int uds_set_server(char local_name[])
{
    int usfd;

    struct sockaddr_un uloc_addr;

    int uloc_len;

    if ((usfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("Error in socket creation");
        exit(0);
    }

    uloc_addr.sun_family = AF_UNIX;
    strcpy(uloc_addr.sun_path, local_name);

    unlink(local_name);

    uloc_len = sizeof(uloc_addr);

    if (bind(usfd, (struct sockaddr *)&uloc_addr, sizeof(uloc_addr)) == -1)
    {
        perror("Error in binding");
        exit(0);
    }

    if (listen(usfd, 3) == -1)
    {
        perror("Error in listen");
        exit(0);
    }

    return usfd;
}

int uds_set_client(char foreign_name[], char local_name[])
{
    int usfd;

    struct sockaddr_un ufor_addr;

    int userv_len;

    struct sockaddr_un uloc_addr;

    int uloc_len;

    if ((usfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("Error in socket creation");
        exit(0);
    }

    unlink(local_name);

    uloc_addr.sun_family = AF_UNIX;
    strcpy(uloc_addr.sun_path, local_name);

    uloc_len = sizeof(uloc_addr);

    if (bind(usfd, (struct sockaddr *)&uloc_addr, sizeof(uloc_addr)) == -1)
    {
        perror("Error in binding");
        exit(0);
    }

    ufor_addr.sun_family = AF_UNIX;
    strcpy(ufor_addr.sun_path, foreign_name);

    userv_len = sizeof(ufor_addr);

    if (connect(usfd, (struct sockaddr *)&ufor_addr, userv_len) == -1)
    {
        perror("Error in accept");
        exit(0);
    }

    return usfd;
}

int connect_umpire()
{
    return uds_set_client(UMP, BOW_UMP);
}

int accept_batsman(int usfd)
{
    struct sockaddr_un bats_addr;
    int len = sizeof(bats_addr);
    int nusfd;
    if ((nusfd = accept(usfd, (SA *)&bats_addr, (socklen_t *)&len)) == -1)
    {
        perror("Cannot accept with batsman\n");
    }
    return nusfd;
}

void exit_handler(int sig)
{
    write(STDOUT_FILENO, "got signal from umpire to quit\n", 32);

    exit(EXIT_FAILURE);
}

void next_bowler(int sig)
{
    write(STDOUT_FILENO, "getting next bowler\n", 21);
}

void init_code()
{
    signal(SIGUSR1, exit_handler);
    signal(SIGUSR2, next_bowler);
    srand(time(0));
}

unsigned short /* this function generates header checksums */
csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
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

int get_pid_from_name(char *name)
{
    char command[200] = {0};
    strcpy(command, "pidof ");
    strcat(command, name);
    int fd = fileno(popen(command, "r"));
    char s[1000];
    read(fd, &s, 1000);
    int X = atoi(s);
    return X;
}

int main(int argc, char *argv[])
{
    init_code();
    int bow_usfd = uds_set_server(BOW);
    int umpire_usfd = connect_umpire();
    int bat_usfd = accept_batsman(bow_usfd);
    while (1)
    {
        int file_fd = recv_fd(umpire_usfd);
        int speed = rand() % 20;
        int spin = rand() % 20;
        printf("speed:%d spin:%d\n", speed, spin);
        long pointer = lseek(file_fd, 0, SEEK_CUR);
        write(file_fd, &speed, sizeof(speed));
        write(file_fd, &spin, sizeof(spin));
        lseek(file_fd, pointer, SEEK_SET);
        kill(get_pid_from_name("./umpire.out"), SIGUSR1);
        send_fd(bat_usfd, file_fd);
    }
}