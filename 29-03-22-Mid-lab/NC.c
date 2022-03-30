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
#include <sys/poll.h>
#define SA struct sockaddr
#define RESERVED 2
int u_rsfd;
int csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
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
    char *buffer = (char *)iph + (iph->ip_hl << 2);
    strcpy(buffer, message);
    return iph;
}

int send_ip_packet(struct ip *iph, int sock)
{
    struct sockaddr_in for_addr;
    for_addr.sin_family = PF_INET;
    for_addr.sin_addr = iph->ip_dst;

    return sendto(sock, iph, iph->ip_len, 0, (SA *)&for_addr, sizeof(for_addr));
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

    serv_addr.sin_family = AF_INET;
    int port_no = atoi(port);
    printf("port no:%d\n", port_no);
    serv_addr.sin_port = htons(port_no);

    if (inet_pton(AF_INET, foreign_ip, &serv_addr.sin_addr) <= 0)
    {
        perror("Error in network address");
        exit(0);
    }

    if (connect(sfd, (struct sockaddr *)&serv_addr, sizeof serv_addr) == -1)
    {
        perror("Error in connect");
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
        perror("Error in network address");
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
        perror("Error in network address");
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
        perror("Error in network address");
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

void accept_package(int pack_sfd, char ***pack_info, int *pack_no, struct pollfd *poll_fds, int *n_pfds)
{
    int temp_no = *pack_no;
    int nsfd;
    struct sockaddr_in foreign_addr;
    bzero(&foreign_addr, sizeof(foreign_addr));
    socklen_t foreign_len = sizeof(foreign_addr);
    if ((nsfd = accept(pack_sfd, (struct sockaddr *)&foreign_addr, &foreign_len)) == -1)
    {
        printf("Error in accept\n");
        exit(0);
    }
    printf("Accepted the package\n");
    for (int i = 0; i < 5; i++)
    {

        tcp_recieve(nsfd, pack_info[temp_no][i]);
        printf("%s\n", pack_info[temp_no][i]);
        fflush(stdout);
    }
    printf("recieved the data\n");
    temp_no++;
    (*pack_no) = temp_no;
    temp_no = (*n_pfds);
    poll_fds[temp_no].fd = nsfd;
    poll_fds[temp_no].events = POLLIN;
    temp_no++;
    (*n_pfds) = temp_no;
}

void accept_client(int client_sfd, char ***client_info, int *n_clients, char ***pack_info, int n_pack, struct pollfd *poll_fds, int no_poll_fd)
{
    int temp_no = *n_clients;
    int nsfd;
    struct sockaddr_in foreign_addr;
    bzero(&foreign_addr, sizeof(foreign_addr));
    socklen_t foreign_len = sizeof(foreign_addr);
    if ((nsfd = accept(client_sfd, (struct sockaddr *)&foreign_addr, &foreign_len)) == -1)
    {
        printf("Error in accept\n");
        exit(0);
    }
    printf("Accepted the client\n");
    fflush(stdout);
    char *buffer = (char *)malloc(sizeof(char) * 200);
    tcp_recieve(nsfd, buffer);
    printf("client was to connect to package no:%s\n", buffer);
    int pack_no = atoi(buffer);
    int target_index = -1;

    printf("n_pack:%d\n", n_pack);
    for (int i = 0; i < n_pack; i++)
    {
        printf("pack_info:%s\n", pack_info[i][4]);
        if (pack_no == atoi(pack_info[i][4]))
        {
            target_index = i;
            break;
        }
    }
    printf("target_index:%d\n", target_index);
    if (target_index == -1)
    {
        printf("Invalid package no\n");
        close(nsfd);
        return;
    }
    int temp_client_no = *n_clients;
    (*n_clients)++;
    char *option = (char *)malloc(sizeof(char) * 200);
    tcp_recieve(nsfd, option);
    for (int i = 0; i < 5; i++)
    {
        tcp_recieve(nsfd, client_info[temp_client_no][i]);
    }
    printf("recieved the data\n");
    for (int i = 2; i < no_poll_fd; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            tcp_send(poll_fds[i].fd, client_info[temp_client_no][j]);
        }
    }
    if (strcmp(option, "paid") == 0)
    {
        printf("Client is asking for paid\n");
        tcp_send(nsfd, pack_info[target_index][0]);
        tcp_send(nsfd, pack_info[target_index][1]);
    }
    else
    {
        printf("Client is asking for live\n");
        tcp_send(nsfd, pack_info[target_index][2]);
        tcp_send(nsfd, pack_info[target_index][3]);
    }
}

int deal_with_free(int pack_nsfd, char ***client_info)
{
    char *buffer = (char *)calloc(200, sizeof(char));
    if ((read(pack_nsfd, buffer, 128)) == 0)
    {
        return 0;
    }
    printf("package has sent '%s'\n", buffer);
    fflush(stdout);
    struct ip *ip_header = create_ip_packet(buffer, 200, "127.0.0.1", "127.0.0.1");
    send_ip_packet(ip_header, u_rsfd);
    return strlen(buffer);
}

int main(int argc, char *argv[])
{
    // first argument is the NC IP to connect to
    // second argument is the NC port to connect to
    // third argument is the UDP IP to open
    // fourth argument is the UDP port to open
    // fifth argument is the TCP IP to open
    // sixth argument is the TCP port to open on
    if (argc != 6)
    {
        printf("Usage, %s <Connect P IP> <Connect P Port> <Connect Client IP> <Connect Client Port> <RAW SOCKET HPL>\n", argv[0]);
        exit(0);
    }
    u_rsfd = -1;
    if ((u_rsfd = socket(PF_INET, SOCK_RAW, 100)) == -1)
    {
        perror("error in creation of raw socket\n");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    const int *val = &one;
    if (setsockopt(u_rsfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        printf("\n\t Warning: I was not able to set HDRINCL!\n");

    char *connect_p_ip = (char *)calloc(200, sizeof(char));
    char *connect_client_ip = (char *)calloc(200, sizeof(char));
    char *connect_p_port = (char *)calloc(200, sizeof(char));
    char *connect_client_port = (char *)calloc(200, sizeof(char));
    char *raw_hpl = (char *)calloc(200, sizeof(char));
    for (int i = 2; i <= 4; i += 2)
    {
        if (atoi(argv[i]) == 0)
        {
            printf("%s is not a number\n", argv[i]);
            exit(0);
        }
    }

    strcpy(connect_p_ip, argv[1]);
    strcpy(connect_p_port, argv[2]);
    strcpy(connect_client_ip, argv[3]);
    strcpy(connect_client_port, argv[4]);
    strcpy(raw_hpl, argv[5]);

    // 0 for udp IP
    // 1 for udp PORT
    // 2 for tcp IP
    // 3 for tcp PORT
    char ***package_info = (char ***)calloc(100, sizeof(char **));
    for (int i = 0; i < 100; i++)
    {
        package_info[i] = (char **)calloc(5, sizeof(char *));
        for (int j = 0; j < 5; j++)
        {
            package_info[i][j] = (char *)calloc(200, sizeof(char));
            bzero(package_info[i][j], 200);
        }
    }
    int no_packs = 0;
    // char client_info[100][200];
    char ***client_info = (char ***)calloc(100, sizeof(char **));
    for (int i = 0; i < 100; i++)
    {
        client_info[i] = (char **)calloc(5, sizeof(char *));
        for (int j = 0; j < 5; j++)
        {
            client_info[i][j] = (char *)calloc(200, sizeof(char));
        }
    }
    int no_clients = 0;
    int pack_sfd = tcp_server_init(connect_p_ip, connect_p_port);
    int client_sfd = tcp_server_init(connect_client_ip, connect_client_port);

    struct pollfd poll_fds[100];
    poll_fds[0].events = POLLIN;
    poll_fds[0].fd = pack_sfd;
    poll_fds[1].events = POLLIN;
    poll_fds[1].fd = client_sfd;
    int n_poll_fds = 2;
    for (int i = 0; i < n_poll_fds; i++)
    {
        printf("%d %d\n", i, poll_fds[i].fd);
    }
    while (1)
    {
        printf("Waiting for connections\n");
        fflush(stdout);
        printf("no of poll fds=%d\n", n_poll_fds);
        poll(poll_fds, n_poll_fds, -1);
        printf("Came out of here\n");
        fflush(stdout);
        int temp = n_poll_fds;
        for (int i = 0; i < temp; i++)
        {
            if (poll_fds[i].revents == 0)
            {
                continue;
            }
            if (poll_fds[i].revents == POLLIN)
            {
                if (i == 0)
                {
                    printf("Accepting package\n");
                    accept_package(poll_fds[i].fd, package_info, &no_packs, poll_fds, &n_poll_fds);
                }
                else if (i == 1)
                {
                    printf("Accepting client\n");
                    accept_client(poll_fds[i].fd, client_info, &no_clients, package_info, no_packs, poll_fds, n_poll_fds);
                }
                else
                {
                    int ret_val = deal_with_free(poll_fds[i].fd, client_info);
                    if (ret_val == 0)
                    {
                        poll_fds[i].fd = -1;
                    }
                }
            }
        }
    }
}