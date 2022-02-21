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
#define SA struct sockaddr
#define UDP_PORT 50000
#define ALL_PORTS 65336

#define N 100
char *USD_PATH = "F-USD-PATH";

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
        perror("Error in listen\n");
        exit(0);
    }

    if (usfd != -1)
        printf("USD bind successful\n");

    return usfd;
}

int bindUDP()
{
    int sfd = -1;
    if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("Error in udp socket creation\n");
        exit(0);
    }
    struct sockaddr_in localAddr;
    bzero(&localAddr, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(UDP_PORT);
    localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if ((bind(sfd, (struct sockaddr *)&localAddr, sizeof(localAddr))) == -1)
    {
        printf("Error in udp bind\n");
        printf("error:%s errorno:%d\n", strerror(errno), errno);

        exit(EXIT_FAILURE);
    }
    if (sfd != -1)
        printf("UDP Bind successful\n");
    return sfd;
}

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

char *recieveUdp(int udpSFD)
{
    char *buff = (char *)calloc(200, sizeof(char));
    bzero(buff, sizeof(buff));
    struct sockaddr_in sendSockAdd;
    int sockAddLen = sizeof(sendSockAdd);
    if ((recvfrom(udpSFD, buff, 128, 0, (struct sockaddr *)&sendSockAdd, (socklen_t *)&sockAddLen)) == -1)
    {
        printf("error in recieve udp\n");
    }
    return buff;
}

void getPortAndPath(char *buff, int *portPtr, char *path)
{
    char *portStart = strtok(buff, "(,)");
    char *portEnd = strtok(NULL, "(,)");
    *portPtr = atoi(portStart);
    strtok(NULL, "(,)");
    strcpy(path, portEnd);
}

int acceptUSD(int usdSFD)
{
    int nusfd = -1;

    struct sockaddr_un ucli_addr;
    bzero(&ucli_addr, sizeof(ucli_addr));

    int ucli_len = sizeof(ucli_addr);
    if ((nusfd = accept(usdSFD, (SA *)&ucli_addr, (socklen_t *)&ucli_len)) == -1)
    {
        perror("Error in usdSFD accept\n");
        return -1;
    }
    return nusfd;
}

int createService(int udpSFD, int usdSFD, int *sfdPtr, int *nusfdPtr)
{
    char *buff = recieveUdp(udpSFD);

    int servicePort;
    char *path = (char *)calloc(200, sizeof(char));
    getPortAndPath(buff, &servicePort, path);

    int sfd = -1;
    int nusfd = -1;
    sfd = bindAndListenTcp(servicePort);
    if (fork() > 0)
    {
        nusfd = acceptUSD(usdSFD);
    }
    else
    {
        char usd_path[200];
        bzero(usd_path, sizeof(usd_path));
        strcpy(usd_path, USD_PATH);
        char *args[] = {path, usd_path, NULL};
        execvp(args[0], args);
        exit(0);
    }
    *sfdPtr = sfd;
    *nusfdPtr = nusfd;
    if (sfd != -1 && nusfd != -1)
    {
        printf("Successfully created a service at port no:%d\n", servicePort);
        fflush(stdout);
    }
    return servicePort;
}

void acceptClientAndSendFD(int tcpSFD, int nusfd)
{
    int nsfd;
    struct sockaddr_in tcli_addr;
    int tcli_len = sizeof(tcli_addr);
    if ((nsfd = accept(tcpSFD, (SA *)&tcli_addr, (socklen_t *)&tcli_len)) == -1)
    {
        perror("Error in accept\n");
        return;
    }
    char *buff = (char *)calloc(200, sizeof(char));
    bzero(buff, sizeof(buff));
    strcpy(buff, "sending fd");
    send(nusfd, buff, 128, 0);

    if (send_fd(nusfd, nsfd) < 0)
    {
        perror("Error in send fd\n");
        return;
    }
    printf("Successfully accepted a client\n");
    fflush(stdout);
}

struct sfd_info_t
{
    int portNo;
    int sfd;
};
typedef struct sfd_info_t sfd_info;

void handleNewService(int udpSFD, int usdSFD, int allUSDs[], sfd_info allTSFDs[], struct pollfd pollFDs[], int *pollSize, int *tsfdSize)
{
    int sfd;
    int nusfd;
    int servicePort = createService(udpSFD, usdSFD, &sfd, &nusfd);

    allUSDs[servicePort] = nusfd;

    allTSFDs[*tsfdSize].sfd = sfd;
    allTSFDs[*tsfdSize].portNo = servicePort;

    pollFDs[*pollSize].fd = sfd;
    pollFDs[*pollSize].events = POLLIN;

    (*pollSize)++;
    (*tsfdSize)++;
}

void handleClientRequest(sfd_info tcpInfoSFD, int allUSDs[])
{
    acceptClientAndSendFD(tcpInfoSFD.sfd, allUSDs[tcpInfoSFD.portNo]);
}

int main(int argc, char *argv[])
{
    int udpSFD = bindUDP();
    int usFD = bindConctOrntdUSServer(USD_PATH);

    fflush(stdout);

    struct pollfd pollFDs[N];
    sfd_info allTSFDs[N];
    int allUSDs[ALL_PORTS];

    bzero(pollFDs, sizeof(pollFDs));
    bzero(allTSFDs, sizeof(allTSFDs));
    bzero(allUSDs, sizeof(allUSDs));

    for (int i = 0; i < N; i++)
        pollFDs[i].fd = -1;

    int sizePoll = 0;
    int sizeTsfd = 0;

    pollFDs[sizePoll].fd = udpSFD;
    pollFDs[sizePoll++].events = POLLIN;
    while (1)
    {
        poll(pollFDs, sizePoll, -1);
        int currSize = sizePoll;
        for (int i = 0; i < currSize; i++)
        {
            fflush(stdout);
            if (pollFDs[i].revents == 0)
                continue;
            if (pollFDs[i].fd == -1)
                continue;
            if (!(pollFDs[i].revents | POLLIN))
            {
                fflush(stdout);
                if (pollFDs[i].revents | POLLHUP | POLLERR)
                {
                    pollFDs[i].fd = -1;
                    if (i != 0)
                    {
                        char *buff = (char *)calloc(200, sizeof(char));
                        strcpy(buff, "exit");
                        send(allUSDs[allTSFDs[i - 1].portNo], buff, 128, 0);
                        //free(buff);
                    }
                }
                continue;
            }
            if (i == 0)
            {
                handleNewService(udpSFD, usFD, allUSDs, allTSFDs, pollFDs, &sizePoll, &sizeTsfd);
                continue;
            }
            handleClientRequest(allTSFDs[i - 1], allUSDs);
            fflush(stdout);
        }
    }
}