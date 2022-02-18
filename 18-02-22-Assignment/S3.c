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
#include <time.h>
#define SA struct sockaddr
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
    unlink(localPath);

    ulocal_len = sizeof(ulocal_addr);
    // if (bind(usfd, (struct sockaddr *)&ulocal_addr, ulocal_len) == -1)
    // {
    //     perror("local: bind");
    //     exit(0);
    // }

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
char *changeBuff(char *buff)
{
    int len = strlen(buff);
    for (int i = 0; i < len; i++)
    {
        if (buff[i] >= 'A' && buff[i] <= 'Z')
        {
            buff[i] = buff[i] - 'A' + 'a';
        }
    }
    return buff;
}

void *thread_func(void *arg)
{
    int nsfd = *(int *)arg;
    char *buff = (char *)calloc(200, sizeof(char));
    while (1)
    {
        bzero(buff, sizeof(buff));
        read(nsfd, buff, 128);
        if (strcmp(buff, "exit") == 0)
        {
            break;
        }
        changeBuff(buff);
        write(nsfd, buff, 128);
    }
    free(buff);
    free(arg);
}
void startServeClient(int usfd)
{
    int nsfd;
    if ((nsfd = recv_fd(usfd)) == 1)
    {
        perror("Error in recieiving\n");
        return;
    }

    pthread_t currThread;
    int *fdPtr = (int *)malloc(sizeof(int));
    *fdPtr = nsfd;
    pthread_create(&currThread, NULL, thread_func, (void *)fdPtr);
}

void gen_random(char *s, const int len)
{
    srand(time(0));
    static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i)
    {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    strcat(s, "USD-PATH");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        exit(EXIT_FAILURE);
    char *localUSDPath = (char *)calloc(200, sizeof(char));
    char *foreignUSDPath = (char *)calloc(200, sizeof(char));

    gen_random(localUSDPath, 7);
    strcpy(foreignUSDPath, argv[1]);

    int usfd = connectConctOrntdUSClient(localUSDPath, foreignUSDPath);
    char *buff = (char *)calloc(200, sizeof(char));
    while (1)
    {
        bzero(buff, sizeof(buff));
        fflush(stdout);
        read(usfd, buff, 128);
        if (buff == "exit")
        {
            printf("Exiting\n");
        }
        startServeClient(usfd);
    }
}