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
#include <pthread.h>
#define __USE_BSD /* use bsd'ish ip header */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#define __FAVOR_BSD /* use bsd'ish tcp header */
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#define SA struct sockaddr
#define PROTO_SAN 202
char client_no = 0;
struct sockaddr_in getServAddr(char *servIP, int servPort)
{
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(servIP);
    serv_addr.sin_port = htons(servPort);
    return serv_addr;
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

int connectTcp(char *servIP, int servPort)
{
    int sFD;
    if ((sFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in socket creation\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv_addr = getServAddr(servIP, servPort);
    int serv_len = sizeof(serv_addr);
    if ((connect(sFD, (SA *)&serv_addr, serv_len)) < 0)
    {
        perror("Error in connect\n");
        exit(1);
    }
    return sFD;
}

bool dealWithServ(int sFD, char *buff)
{
    bzero(buff, sizeof(buff));
    int buff_len = 200;
    fgets(buff, buff_len, stdin);
    buff[strlen(buff) - 1] = '\0';
    printf("buff is:%s\n", buff);
    fflush(stdout);
    write(sFD, buff, 128);
    if (strcmp(buff, "exit") == 0)
        return true;
    bzero(buff, sizeof(buff));
    read(sFD, buff, 128);
    printf("Read from server '%s'\n", buff);
    return false;
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

struct message_struct
{
    long type;
    char semaphoreName[100];
    char usdPath[100];
};

sem_t *getSem(char *semName)
{
    unlink(semName);
    sem_t *currSem = sem_open(semName, O_CREAT, 0666, 0);
    return currSem;
}

int sendMsg(int mQId, char *semName, char *usdPath)
{
    struct message_struct currMsg;
    currMsg.type = 100;
    strcpy(currMsg.semaphoreName, semName);
    strcpy(currMsg.usdPath, usdPath);
    return msgsnd(mQId, &currMsg, 200, 0);
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

int connectConctOrntdUSClient(char *serverPath)
{
    int usfd;
    struct sockaddr_un userv_addr;
    int userv_len, ucli_len;
    if ((usfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in socket\n");
        exit(0);
    }
    bzero(&userv_addr, sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, serverPath);
    userv_len = sizeof(userv_addr);
    if (connect(usfd, (struct sockaddr *)&userv_addr, userv_len) == -1)
    {
        perror("Error in connect\n");
        exit(0);
    }
    return usfd;
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
        printf("Error in listen\n");
        exit(0);
    }
    return usfd;
}

char *recv_from_raw(int rsfd, char *recv_addr)
{
    char *buffer = (char *)malloc(1024 * sizeof(char));
    bzero(buffer, 1024);
    struct sockaddr_in for_addr;
    for_addr.sin_family = PF_INET;
    for_addr.sin_addr.s_addr = inet_addr(recv_addr);
    int len = sizeof(for_addr);
    int retVal = -1;
    if ((retVal = recvfrom(rsfd, buffer, 1024, 0, (SA *)&for_addr, (socklen_t *)&len)) == -1)
    {
        perror("Error in recv from fielder\n");
        return NULL;
    }
    return buffer;
}

void print_ip_header(struct ip *iph)
{
    // printf("header length:%d\n", iph->ip_hl);
    // printf("version:%d\n", iph->ip_v);
    // printf("TOS:%d\n", iph->ip_tos);
    // printf("IP Len:%d\n", iph->ip_len);
    // printf("id: %d\n", iph->ip_id);
    // printf("offest:%d\n", iph->ip_off);
    // printf("TTL:%d\n", iph->ip_ttl);
    // printf("Prot No:%d\n", iph->ip_p);
    // printf("Sum:%d\n", iph->ip_sum);
    // printf("Src addr:%s\n", inet_ntoa(iph->ip_src));
    // printf("Dest addr:%s\n", inet_ntoa(iph->ip_dst));
    // printf("TOS:%d\n", iph->ip_tos);
    char *message = (char *)iph + (iph->ip_hl << 2);
    printf("\nMessage:%s\n", message);
}

struct ip *create_ip_packet(char *message, int prot_no, char *src_addr, char *dest_addr)
{
    struct ip *iph = (struct ip *)malloc(1024 * sizeof(char));
    iph->ip_hl = 5;
    iph->ip_v = 4;
    iph->ip_tos = 0;
    iph->ip_len = sizeof(struct ip) + strlen(message); /* no payload */
    iph->ip_id = htonl(54321);                         /* the value doesn't matter here */
    iph->ip_off = 0;
    iph->ip_ttl = 255;
    iph->ip_p = prot_no;
    iph->ip_sum = 0;                          /* set it to 0 before computing the actual checksum later */
    iph->ip_src.s_addr = inet_addr(src_addr); /* SYN's can be blindly spoofed */
    iph->ip_dst.s_addr = inet_addr(dest_addr);

    iph->ip_sum = csum((unsigned short *)iph, iph->ip_len >> 1);
    char *payload = (char *)iph + (iph->ip_hl << 2);
    strcpy(payload, message);
    return iph;
}

int send_ip_packet(struct ip *iph, int sock)
{
    struct sockaddr_in for_addr;
    for_addr.sin_family = PF_INET;
    for_addr.sin_addr = iph->ip_dst;

    return sendto(sock, iph, iph->ip_len, 0, (SA *)&for_addr, sizeof(for_addr));
}

void *read_from_raw_socket(void *rsfd_ptr)
{
    int rsfd = *(int *)rsfd_ptr;
    printf("Rsfd:%d\n", rsfd);
    fflush(stdout);
    while (1)
    {
        char *buffer = recv_from_raw(rsfd, "0.0.0.0");
        print_ip_header((struct ip *)buffer);
    }
}

void send_message_from_raw(int rsfd, char *message)
{
    struct ip *iph = create_ip_packet(message, PROTO_SAN, "192.168.192.118", "127.0.0.1");
    send_ip_packet(iph, rsfd);
}
int get_raw_socket()
{
    int rsfd;
    if ((rsfd = socket(PF_INET, SOCK_RAW, PROTO_SAN)) == -1)
    {
        perror("error in creation of raw socket\n");
        exit(EXIT_FAILURE);
    }
    char *buffer = (char *)malloc(1024 * sizeof(char));
    int one = 1;
    const int *val = &one;
    if (setsockopt(rsfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        printf("\n\t Warning: I was not able to set HDRINCL!\n");
    return rsfd;
}

void create_thread_to_read(int rsfd)
{
    int *rsfd_ptr = (int *)malloc(sizeof(int));
    *rsfd_ptr = rsfd;
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, read_from_raw_socket, rsfd_ptr);
}

int main(int argc, char *argv[])
{
    int prot_no = 200;
    int rsfd = get_raw_socket();
    create_thread_to_read(rsfd);
    int servPort = 50500;
    char *servIP = "127.0.0.1";
    int clientNo = 1;

    printf("argc:%d\n", argc);
    char *myName = (char *)calloc(200, sizeof(char));
    printf("%s\n", argv[0]);
    switch (argc)
    {
    case 1:
        strcpy(myName, argv[0]);
        break;
    case 2:
        printf("argv[1]:%s\n", argv[1]);
        clientNo = atoi(argv[1]);
        strcpy(myName, argv[1]);
        break;
    case 3:
        clientNo = atoi(argv[1]);
        strcpy(myName, argv[1]);
        servPort = atoi(argv[2]);
        break;
    case 4:
        clientNo = atoi(argv[1]);
        strcpy(myName, argv[1]);
        servPort = atoi(argv[2]);
        servIP = (char *)calloc(200, sizeof(char));
        strcpy(servIP, argv[3]);
        break;
    }
    printf("Hi my name is %s\n", myName);
    char *rsfd_message = (char *)calloc(200, sizeof(char));
    sprintf(rsfd_message, "%s got the connection\n", myName);
    int sFD;
    int mQId;
    char *buff = (char *)calloc(200, sizeof(char));

    char *semName = (char *)calloc(200, sizeof(char));
    strcpy(semName, myName);
    strcat(semName, "sem");
    sem_t *currSem = getSem(semName);

    char *usdPath = (char *)calloc(200, sizeof(char));
    strcpy(usdPath, myName);
    strcat(usdPath, "usdpath");

    bool isPresent = getMQId("ess-mq", &mQId);
    if (!isPresent)
    {
        sFD = connectTcp(servIP, servPort);
    }
    else
    {
        int usfd = bindConctOrntdUSServer(usdPath);
        if (sendMsg(mQId, semName, usdPath) < 0)
        {
            perror("Error in sendMsg");
            printf("mQId:%d\n", mQId);
        }
        printf("sent a request\n");
        struct sockaddr_un ucli_addr;
        int ucli_len = sizeof(ucli_addr);
        int nusfd;
        if ((nusfd = accept(usfd, (SA *)&ucli_addr, &ucli_len)) < 0)
        {
            perror("Error in accept\n");
            exit(EXIT_FAILURE);
        }
        sFD = recv_fd(nusfd);
        close(usfd);
        unlink(usdPath);
    }
    printf("Connected \n");
    send_message_from_raw(rsfd, rsfd_message);
    struct message_struct nextMessage;
    bzero(&nextMessage, sizeof(nextMessage));
    bool clientPresent = false;
    while (1)
    {
        if (dealWithServ(sFD, buff))
        {
            break;
        }
        if (clientPresent || msgrcv(mQId, &nextMessage, 200, 100, IPC_NOWAIT) >= 0)
        {
            clientPresent = true;
            printf("Found a waiting client\n");
            printf("Do you want to send the connection to other client(y/n):");
            char response[128];
            fgets(response, 128, stdin);
            if (strcmp(response, "y\n") != 0)
            {
                continue;
            }
            clientPresent = false;
            printf("Usd path:%s\n", nextMessage.usdPath);
            int usfd = connectConctOrntdUSClient(nextMessage.usdPath);
            send_fd(usfd, sFD);
            printf("Sent the fd\n");
            fflush(stdout);
            printf("Enter a something to post a request to get the connection(exit for exiting):");
            char temp[100];
            fgets(temp, 100, stdin);
            if (strcmp(temp, "exit\n") == 0)
            {
                exit(0);
            }
            isPresent = getMQId("ess-mq", &mQId);
            if (!isPresent)
            {
                sFD = connectTcp(servIP, servPort);
            }
            else
            {
                int usfd = bindConctOrntdUSServer(usdPath);
                sendMsg(mQId, semName, usdPath);
                struct sockaddr_un ucli_addr;
                int ucli_len = sizeof(ucli_addr);
                int nusfd;
                if ((nusfd = accept(usfd, (SA *)&ucli_addr, &ucli_len)) < 0)
                {
                    perror("Error in accept\n");
                    exit(EXIT_FAILURE);
                }
                sFD = recv_fd(nusfd);
                close(usfd);
                unlink(usdPath);
            }
            printf("Got the connection\n");
            send_message_from_raw(rsfd, rsfd_message);
        }
    }
    if (msgrcv(mQId, &nextMessage, 200, 100, IPC_NOWAIT) >= 0)
    {
        int usfd = connectConctOrntdUSClient(nextMessage.usdPath);
        send_fd(usfd, sFD);
    }
    else
    {
        if (msgctl(mQId, IPC_RMID, NULL) == -1)
        {
            perror("msgctl");
            exit(1);
        }
    }
    close(sFD);
}