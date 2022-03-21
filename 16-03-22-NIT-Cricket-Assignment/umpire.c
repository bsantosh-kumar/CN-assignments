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
#define BOW_UMP "bowler_umpire.uds"
#define BAT_UMP "batsman_umpire.uds"
#define UMP "umpire.uds"
#define PROT_UMP 200
#define PROT_FIE 240
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

int accept_connection(int usfd, int *bowusFD, int *batusFD)
{
    struct sockaddr_un cli_addr;
    int len = sizeof(cli_addr);
    int currSFD;
    if ((currSFD = accept(usfd, (SA *)&cli_addr, (socklen_t *)&len)) == -1)
    {
        perror("Error in accept\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(cli_addr.sun_path, BOW_UMP) == 0)
    {
        *bowusFD = currSFD;
        return 0;
    }
    else if (strcmp(cli_addr.sun_path, BAT_UMP) == 0)
    {
        *batusFD = currSFD;
        return 1;
    }
    else
    {
        printf("got connection from some other path\n");
    }
}

void batsman_handler(int sig)
{
    write(STDOUT_FILENO, "Batsman responded\n", 19);
}

void bowler_handler(int sig)
{
    write(STDOUT_FILENO, "Bowler responded\n", 18);
}

void init_code()
{
    signal(SIGUSR1, bowler_handler);
    signal(SIGUSR2, batsman_handler);
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

struct ip *create_ip_packet(char *message, int prot_no, char *src_addr, char *dest_addr)
{
    struct ip *iph = (struct ip *)malloc(4096 * sizeof(char));
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
    return iph;
}

int send_ip_packet(struct ip *iph, int sock)
{
    struct sockaddr_in for_addr;
    for_addr.sin_family = PF_INET;
    for_addr.sin_addr = iph->ip_dst;

    return sendto(sock, iph, iph->ip_len, 0, (SA *)&for_addr, sizeof(for_addr));
}

char *recv_from_raw(int rsfd)
{
    char *buffer = (char *)malloc(4096 * sizeof(char));
    bzero(buffer, 4096);
    struct sockaddr_in for_addr;
    for_addr.sin_family = PF_INET;
    for_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int len = sizeof(for_addr);
    int retVal = -1;
    if ((retVal = recvfrom(rsfd, buffer, 4096, 0, (SA *)&for_addr, (socklen_t *)&len)) == -1)
    {
        perror("Error in recv from fielder\n");
        return NULL;
    }
    return buffer;
}

char *get_message_from_packet(char *buffer)
{
    struct ip *iph = (struct ip *)buffer;
    char *ans = buffer + (iph->ip_hl << 2);
    return ans;
}

int main(int argc, char *argv[])
{
    init_code();
    int u_rsfd = -1;
    if ((u_rsfd = socket(PF_INET, SOCK_RAW, PROT_UMP)) == -1)
    {
        perror("error in creation of raw socket\n");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    const int *val = &one;
    if (setsockopt(u_rsfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        printf("\n\t Warning: I was not able to set HDRINCL!\n");

    int umpire_usfd = uds_set_server(UMP);
    int bowler_usfd = -1, batsman_usfd = -1;
    //one time for batsman and one time for bowler
    accept_connection(umpire_usfd, &bowler_usfd, &batsman_usfd);
    accept_connection(umpire_usfd, &bowler_usfd, &batsman_usfd);
    sigset_t usr1_set, usr2_set;
    sigemptyset(&usr1_set);
    sigemptyset(&usr2_set);
    sigaddset(&usr1_set, SIGUSR1);
    sigaddset(&usr2_set, SIGUSR2);
    struct timespec timeout;
    timeout.tv_sec = 3;
    timeout.tv_nsec = 0;
    int score = 0;
    int file_fd = open("ball.txt", O_CREAT | O_RDWR | O_TRUNC, 0777);
    int no_of_batsman = 0;
    int no_of_balls = 0;
    while (1)
    {
        send_fd(bowler_usfd, file_fd);
        int retSignal = -1;
        if (sigwait(&usr1_set, &retSignal) == -1)
        {
            perror("error in sigwait\n");
        }
        else
        {
            printf("Bowler has bowled\n");
        }
        siginfo_t info;

        if (sigtimedwait(&usr2_set, &info, &timeout) == -1)
        {
            printf("Batsman did not respond in time\n");
            //if batsman did not respond in time
            //then terminating all programs
            if (no_of_batsman == 11)
            {
                int batsman_pid = get_pid_from_name("./batsman.out");
                int bowler_pid = get_pid_from_name("./bowler.out");
                kill(batsman_pid, SIGUSR1);
                kill(bowler_pid, SIGUSR1);
                break;
            }
            else
            {
                printf("Getting next batsman\n");
                kill(get_pid_from_name("./batsman.out"), SIGUSR2);
                no_of_batsman++;
            }
            continue;
        }
        else
        {
            int curr_runs;
            printf("Batsman %d responded in time\n", no_of_batsman);
            read(batsman_usfd, &curr_runs, sizeof(int));
            if (curr_runs % 4 == 0 || curr_runs % 6 == 0)
            {
                printf("4 or 6\n");
                score += curr_runs % 4 == 0 ? 4 : 6;
                printf("Score is %d\n", score);
            }
            else
            {
                char *buffer = recv_from_raw(u_rsfd);
                char *message = get_message_from_packet(buffer);
                if (strcmp(message, "out") != 0)
                    continue;
                printf("Ball got caught, batsman is out\n");
                if (no_of_batsman == 11)
                {
                    int batsman_pid = get_pid_from_name("./batsman.out");
                    int bowler_pid = get_pid_from_name("./bowler.out");
                    kill(batsman_pid, SIGUSR1);
                    kill(bowler_pid, SIGUSR1);
                    break;
                }
                else
                {
                    printf("Getting next batsman\n");
                    kill(get_pid_from_name("./batsman.out"), SIGUSR2);
                    no_of_batsman++;
                }
            }
        }
        file_fd = recv_fd(batsman_usfd);
        no_of_balls++;
        no_of_balls %= 6;
        if (no_of_balls == 0)
        {
            printf("Getting next bowler\n");
            kill(get_pid_from_name("./bowler.out"), SIGUSR2);
        }
    }
    printf("Score:%d\n", score);
}