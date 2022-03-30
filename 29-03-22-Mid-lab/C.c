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
#include <pthread.h>
#include <time.h>
#include <sys/poll.h>
#define SA struct sockaddr
int csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}
int tcp_server_init(char *local_ip, char *port)
{
    int sfd;
    struct sockaddr_in serv_addr;
    printf("tcp init local ip :%s local port:%s\n", local_ip, port);
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Unable to create socket\n");
        return -1;
    }
    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        perror("Error in setsockopt");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    int port_no = atoi(port);
    printf("port no:%d\n", port_no);

    serv_addr.sin_port = htons(port_no);

    if (inet_pton(AF_INET, local_ip, &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid network address or address family\n");
        return 0;
    }

    if (bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Error in bind\n");
        return -1;
    }
    if (listen(sfd, 3) < 0)
    {
        perror("Error in listen\n");
    }
    printf("tcp sfd:%d\n", sfd);
    return sfd;
}
int tcp_client_init(char *foreign_ip, char *port)
{
    int sfd;
    struct sockaddr_in serv_addr;
    int serv_addrlen = sizeof(serv_addr);

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error in socket");
        exit(0);
    }
    printf("sfd:%d\n", sfd);
    serv_addr.sin_family = AF_INET;
    int port_no = atoi(port);
    printf("port no:%d\n", port_no);
    serv_addr.sin_port = htons(port_no);
    printf("foreign_ip:'%s' foreign_port:'%s'\n", foreign_ip, port);
    serv_addr.sin_addr.s_addr = inet_addr(foreign_ip);

    if ((connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) == -1)
    {
        perror("Error in connect tcp client init");
        exit(0);
    }

    if (getsockname(sfd, (struct sockaddr *)&serv_addr, (socklen_t *)&serv_addrlen) == -1)
    {
        perror("Error in getsockname");
        exit(0);
    }

    printf("Local IP Addresss : %s\n", inet_ntoa(serv_addr.sin_addr));
    printf("Local Port : %d\n\n", ntohs(serv_addr.sin_port));

    if (getpeername(sfd, (struct sockaddr *)&serv_addr, (socklen_t *)&serv_addrlen) == -1)
    {
        perror("Error in getpeername");
        exit(0);
    }

    printf("Foreign IP Addresss : %s\n", inet_ntoa(serv_addr.sin_addr));
    printf("Foreign Port : %d\n\n", ntohs(serv_addr.sin_port));
    fflush(stdout);
    return sfd;
}

int udp_server_init(char *local_ip, char *port)
{
    int sfd;
    struct sockaddr_in serv_addr;

    if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("Unable to create socket\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    if (inet_pton(AF_INET, local_ip, &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid network address or address family\n");
        return 0;
    }

    if (bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Error in bind\n");
        return -1;
    }

    return sfd;
}

int udp_client_init(char *local_ip, char *port)
{
    int sfd;

    if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Error in socket");
        exit(0);
    }
    struct sockaddr_in local_addr;
    bzero(&local_addr, sizeof(local_addr));
    (local_addr).sin_family = AF_INET;
    (local_addr).sin_port = htons(atoi(port));
    if (inet_pton(AF_INET, local_ip, &(local_addr).sin_addr) <= 0)
    {
        perror("Error in network address udp client init");
        exit(0);
    }

    return sfd;
}

int tcp_send(int tcp_sfd, char *message)
{
    if ((write(tcp_sfd, message, 128)) <= 0)
    {
        perror("Write cannot be performed properly\n");
        exit(0);
    }
    else
        return strlen(message);
}

int tcp_recieve(int tcp_sfd, char *buff)
{
    if ((read(tcp_sfd, buff, 128)) <= 0)
    {
        perror("Read cannot be performed\n");
        exit(0);
    }
    return strlen(buff);
}

int udp_send(int udp_sfd, char *message, char *foreign_ip, char *foreign_port)
{
    struct sockaddr_in foreign_addr;
    bzero(&foreign_addr, sizeof(foreign_addr));
    foreign_addr.sin_family = AF_INET;
    foreign_addr.sin_port = htons(atoi(foreign_port));
    if (inet_pton(AF_INET, foreign_ip, &(foreign_addr).sin_addr) <= 0)
    {
        perror("Error in network address udp send");
        exit(0);
    }
    if ((sendto(udp_sfd, message, 128, 0, (SA *)&foreign_addr, sizeof(foreign_addr))) == 0)
    {
        perror("error in send udp\n");
        exit(0);
    }
    return strlen(message);
}
int udp_recv(int udp_sfd, char *buffer, char *foreign_ip, char *foreign_port)
{
    struct sockaddr_in foreign_addr;
    bzero(&foreign_addr, sizeof(foreign_addr));
    foreign_addr.sin_family = AF_INET;
    foreign_addr.sin_port = htons(atoi(foreign_port));
    if (inet_pton(AF_INET, foreign_ip, &(foreign_addr).sin_addr) <= 0)
    {
        perror("Error in network address udp recv");
        exit(0);
    }
    socklen_t foreign_len = sizeof(foreign_addr);
    if ((recvfrom(udp_sfd, buffer, 128, 0, (SA *)&foreign_addr, &foreign_len)) == 0)
    {
        perror("error in send udp\n");
        exit(0);
    }
    return strlen(buffer);
}
void handle_live(int nc_sfd, int udp_sfd, char *client_no)
{
    char *tcp_package_ip = calloc(200, sizeof(char));
    char *(tcp_package_port) = calloc(200, sizeof(char));
    tcp_recieve(nc_sfd, tcp_package_ip);
    tcp_recieve(nc_sfd, tcp_package_port);
    printf("package_ip:%s package_port:%s\n", tcp_package_ip, tcp_package_port);
    fflush(stdout);
    int pack_sfd = tcp_client_init(tcp_package_ip, tcp_package_port);
    struct pollfd poll_fds[10];
    poll_fds[0].fd = pack_sfd;
    poll_fds[0].events = POLLIN;
    poll_fds[1].fd = udp_sfd;
    poll_fds[1].events = POLLIN;
    poll_fds[2].fd = STDIN_FILENO;
    poll_fds[2].events = POLLIN;
    while (1)
    {
        printf("Enter some the client no to send to(-1) to break : ");
        fflush(stdout);
        poll(poll_fds, 3, -1);
        for (int i = 0; i < 3; i++)
        {
            if (poll_fds[i].revents == 0)
            {
                continue;
            }
            if (poll_fds[i].revents & POLLIN)
            {
                if (i == 0)
                {
                }
                if (i == 1)
                {
                    char *buffer = calloc(200, sizeof(char));
                    udp_recv(udp_sfd, buffer, "0.0.0.0", "0");
                    if (strlen(buffer) == 0)
                    {
                        printf("No buffer at all exiting\n");
                        exit(0);
                    }
                    printf("recieved in udp live '%s'\n", buffer);
                    fflush(stdout);
                }
                if (i == 2)
                {
                    char *client_no_to_send = (char *)calloc(200, sizeof(char));
                    scanf("%s", client_no_to_send);
                    printf("client no to send %s\n", client_no_to_send);
                    if (strcmp(client_no_to_send, "-1") == 0)
                    {
                        exit(0);
                    }
                    tcp_send(pack_sfd, client_no_to_send);
                    char *message_to_send = (char *)calloc(200, sizeof(char));
                    printf("Enter the message to send to the other client:\n");
                    scanf("%s", message_to_send);
                    tcp_send(pack_sfd, message_to_send);
                }
            }
        }
    }
}
void handle_paid(int nc_sfd, int udp_sfd, char *client_no)
{
    char *udp_package_ip = calloc(200, sizeof(char));
    char *(udp_package_port) = calloc(200, sizeof(char));
    tcp_recieve(nc_sfd, udp_package_ip);
    tcp_recieve(nc_sfd, udp_package_port);
    printf("package_ip:%s package_port:%s\n", udp_package_ip, udp_package_port);
    struct pollfd poll_fds[10];

    poll_fds[0].fd = udp_sfd;
    poll_fds[0].events = POLLIN;
    poll_fds[1].fd = STDIN_FILENO;
    poll_fds[1].events = POLLIN;
    while (1)
    {
        printf("Enter some the client no to send to(-1) to break : ");
        fflush(stdout);
        poll(poll_fds, 2, -1);
        for (int i = 0; i < 2; i++)
        {
            if (poll_fds[i].revents & POLLIN)
            {
                if (i == 0)
                {

                    char *buffer = calloc(200, sizeof(char));
                    bzero(buffer, sizeof(buffer));
                    udp_recv(udp_sfd, buffer, "0.0.0.0", "0");
                    printf("recieved in udp paid '%s'\n", buffer);
                    fflush(stdout);
                }
                if (i == 1)
                {
                    char *client_no_to_send = (char *)calloc(200, sizeof(char));
                    scanf("%s", client_no_to_send);
                    if (strcmp(client_no_to_send, "-1") == 0)
                    {
                        exit(0);
                    }
                    udp_send(udp_sfd, client_no_to_send, udp_package_ip, udp_package_port);
                    char *message_to_send = (char *)calloc(200, sizeof(char));
                    printf("Enter the message to send to the other client:\n");
                    scanf("%s", message_to_send);
                    udp_send(udp_sfd, message_to_send, udp_package_ip, udp_package_port);
                }
            }
        }
    }

    return;
}
void print_ip_header(struct ip *iph)
{
    printf("header length:%d\n", iph->ip_hl);
    printf("version:%d\n", iph->ip_v);
    printf("TOS:%d\n", iph->ip_tos);
    printf("IP Len:%d\n", iph->ip_len);
    printf("id: %d\n", iph->ip_id);
    printf("offest:%d\n", iph->ip_off);
    printf("TTL:%d\n", iph->ip_ttl);
    printf("Prot No:%d\n", iph->ip_p);
    printf("Sum:%d\n", iph->ip_sum);
    printf("Src addr:%s\n", inet_ntoa(iph->ip_src));
    printf("Dest addr:%s\n", inet_ntoa(iph->ip_dst));
    printf("TOS:%d\n", iph->ip_tos);
    char *message = (char *)iph + (iph->ip_hl << 2);
    printf("Message:%s\n", message);
}

char *recv_from_raw(int rsfd, char *recv_addr)
{
    char *buffer = (char *)malloc(4096 * sizeof(char));
    bzero(buffer, 4096);
    struct sockaddr_in for_addr;
    for_addr.sin_family = PF_INET;
    for_addr.sin_addr.s_addr = inet_addr(recv_addr);
    int len = sizeof(for_addr);
    int retVal = -1;
    if ((retVal = recvfrom(rsfd, buffer, 4096, 0, (SA *)&for_addr, (socklen_t *)&len)) == -1)
    {
        perror("Error in recv from fielder\n");
        return NULL;
    }
    return buffer;
}

void *raw_thread()
{
    int u_rsfd = -1;
    if ((u_rsfd = socket(PF_INET, SOCK_RAW, 200)) == -1)
    {
        perror("error in creation of raw socket\n");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    const int *val = &one;
    if (setsockopt(u_rsfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        printf("\n\t Warning: I was not able to set HDRINCL!\n");
    printf("In raw thread\n");
    while (1)
    {
        char *buffer = recv_from_raw(u_rsfd, "0.0.0.0");
        printf("Recieved from raw\n");
        fflush(stdout);
        print_ip_header((struct ip *)buffer);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 8)
    {
        printf("Usage: %s <NC ip> <NC port> <udp ip> <udp port> <tcp ip> <tcp port> <client no>\n", argv[0]);
        exit(0);
    }
    pthread_t raw_thread_id;
    pthread_create(&raw_thread_id, NULL, raw_thread, NULL);
    char *ncIP = (char *)calloc(200, sizeof(char));
    char *udpIP = (char *)calloc(200, sizeof(char));
    char *tcpIP = (char *)calloc(200, sizeof(char));
    char *ncPORT = (char *)calloc(200, sizeof(char));
    char *tcpPORT = (char *)calloc(200, sizeof(char));
    char *udpPORT = (char *)calloc(200, sizeof(char));
    char *client_no = (char *)calloc(200, sizeof(char));

    bzero(ncIP, sizeof(ncIP));
    bzero(udpIP, sizeof(udpIP));
    bzero(tcpIP, sizeof(tcpIP));
    bzero(ncPORT, sizeof(ncPORT));
    bzero(tcpPORT, sizeof(tcpPORT));
    bzero(udpPORT, sizeof(udpPORT));

    strcpy(ncIP, argv[1]);
    strcpy(ncPORT, argv[2]);
    strcpy(udpIP, argv[3]);
    strcpy(udpPORT, argv[4]);
    strcpy(tcpIP, argv[5]);
    strcpy(tcpPORT, argv[6]);
    strcpy(client_no, argv[7]);

    int udp_sfd = udp_server_init(udpIP, udpPORT);
    int nc_sfd = tcp_client_init(ncIP, ncPORT);

    printf("Enter the package to connect to:");
    int connect_to_package = -1;
    scanf("%d", &connect_to_package);
    char *message = (char *)calloc(200, sizeof(char));
    sprintf(message, "%d", connect_to_package);
    tcp_send(nc_sfd, message);
    bzero(message, sizeof(message));
    char *functions[] = {"live", "paid"};
    srand(time(0));
    sprintf(message, "%s", functions[rand() % 2]);
    // strcpy(message, "paid");
    tcp_send(nc_sfd, message);
    tcp_send(nc_sfd, udpIP);
    tcp_send(nc_sfd, udpPORT);
    tcp_send(nc_sfd, tcpIP);
    tcp_send(nc_sfd, tcpPORT);
    tcp_send(nc_sfd, client_no);
    if (strcmp(message, "live") == 0)
    {
        handle_live(nc_sfd, udp_sfd, client_no);
    }
    else
    {
        handle_paid(nc_sfd, udp_sfd, client_no);
    }
}