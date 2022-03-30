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
        return -1;
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

void store_new_client(int nc_sfd, char ***client_info, int *no_clients)
{
    int temp = (*no_clients);
    (*no_clients)++;
    // udp ip, udp port, tcp ip, tcp port
    for (int i = 0; i < 5; i++)
    {
        tcp_recieve(nc_sfd, client_info[temp][i]);
        printf("%s\n", client_info[temp][i]);
        fflush(stdout);
    }
}

void handle_client_udp(int udp_sfd, char ***client_info, int *all_clients)
{
    char *client_no_to_send = (char *)calloc(200, sizeof(char));
    char *foreign_ip = (char *)calloc(200, sizeof(char));
    char *foreign_port = (char *)calloc(200, sizeof(char));
    strcpy(foreign_ip, "0.0.0.0");
    strcpy(foreign_port, "0");
    udp_recv(udp_sfd, client_no_to_send, foreign_ip, foreign_port);
    printf("client wants to send to :%s\n", client_no_to_send);
    char *message_to_send = (char *)calloc(200, sizeof(char));
    udp_recv(udp_sfd, message_to_send, foreign_ip, foreign_port);
    printf("message to send :%s\n", message_to_send);
    for (int i = 0; i < (*all_clients); i++)
    {
        for (int j = 0; j < 5; j++)
        {
            if (strcmp(client_info[i][j], client_no_to_send) == 0)
            {
                udp_send(udp_sfd, message_to_send, client_info[i][0], client_info[i][1]);
            }
        }
    }

    return;
}
void handle_client_tcp(int tcp_sfd, struct pollfd *poll_fds, int *n_poll_fds)
{
    int nsfd = -1;
    struct sockaddr_in foreign_addr;
    bzero(&foreign_addr, sizeof(foreign_addr));
    socklen_t foreign_len = sizeof(foreign_addr);
    if ((nsfd = accept(tcp_sfd, (SA *)&foreign_addr, &foreign_len)) == -1)
    {
        perror("Error in accept");
        exit(0);
    }
    poll_fds[*n_poll_fds].fd = nsfd;
    poll_fds[*n_poll_fds].events = POLLIN;
    (*n_poll_fds)++;
    return;
}
void talk_with_client(int *nsfd_ptr, int udp_sfd, char ***client_info, int *all_clients)
{
    int nsfd = *nsfd_ptr;
    char *client_no_to_send = (char *)calloc(200, sizeof(char));
    bzero(client_no_to_send, sizeof(client_no_to_send));
    if (tcp_recieve(nsfd, client_no_to_send) == -1)
    {
        *nsfd_ptr = -1;
        printf("Client has exited\n");
        fflush(stdout);
    }
    printf("client no to send:%s\n", client_no_to_send);
    char *message_to_send = (char *)calloc(200, sizeof(char));
    if (tcp_recieve(nsfd, message_to_send) == -1)
    {
        *nsfd_ptr = -1;
        printf("Client has exited\n");
        fflush(stdout);
    }
    printf("message to send is:%s\n", message_to_send);
    for (int i = 0; i < (*all_clients); i++)
    {
        for (int j = 0; j < 5; j++)
        {
            if (strcmp(client_info[i][j], client_no_to_send) == 0)
            {
                udp_send(udp_sfd, message_to_send, client_info[i][0], client_info[i][1]);
            }
        }
    }

    fflush(stdout);
}
int main(int argc, char *argv[])
{
    // first argument is the NC IP to connect to
    // second argument is the NC port to connect to
    // third argument is the UDP IP to open
    // fourth argument is the UDP port to open
    // fifth argument is the TCP IP to open
    // sixth argument is the TCP port to open on
    if (argc != 8)
    {
        printf("Usage, %s <NC IP> <NC port> <UDP IP> <UDP port> <TCP IP> <TCP port> <package no>\n", argv[0]);
        exit(0);
    }
    char *ncIP = (char *)calloc(200, sizeof(char));
    char *udpIP = (char *)calloc(200, sizeof(char));
    char *tcpIP = (char *)calloc(200, sizeof(char));
    char *ncPORT = (char *)calloc(200, sizeof(char));
    char *tcpPORT = (char *)calloc(200, sizeof(char));
    char *udpPORT = (char *)calloc(200, sizeof(char));
    char *package_no = (char *)calloc(200, sizeof(char));
    for (int i = 2; i <= 6; i += 2)
    {
        if (atoi(argv[i]) == 0)
        {
            printf("%s is not a number\n", argv[i]);
            exit(0);
        }
    }
    bzero(ncIP, sizeof(ncIP));
    strcpy(ncIP, argv[1]);
    bzero(ncPORT, sizeof(ncPORT));
    strcpy(ncPORT, argv[2]);
    bzero(udpIP, sizeof(udpIP));
    strcpy(udpIP, argv[3]);
    bzero(udpPORT, sizeof(udpPORT));
    strcpy(udpPORT, argv[4]);
    bzero(tcpIP, sizeof(tcpIP));
    strcpy(tcpIP, argv[5]);
    bzero(tcpPORT, sizeof(tcpPORT));
    strcpy(tcpPORT, argv[6]);
    bzero(package_no, sizeof(package_no));
    strcpy(package_no, argv[7]);
    int udp_sfd = udp_server_init(udpIP, udpPORT);
    int tcp_sfd = tcp_server_init(tcpIP, tcpPORT);
    int nc_sfd = tcp_client_init(ncIP, ncPORT);
    // first of all every package sends the information of its
    // udp ip, udp port, tcp ip, tcp port to NC
    tcp_send(nc_sfd, udpIP);
    tcp_send(nc_sfd, udpPORT);
    tcp_send(nc_sfd, tcpIP);
    tcp_send(nc_sfd, tcpPORT);
    tcp_send(nc_sfd, package_no);
    struct pollfd poll_fds[100];
    int n_poll_fds = 0;
    poll_fds[n_poll_fds].fd = nc_sfd;
    poll_fds[n_poll_fds].events = POLLIN;
    n_poll_fds++;
    poll_fds[n_poll_fds].fd = udp_sfd;
    poll_fds[n_poll_fds].events = POLLIN;
    n_poll_fds++;
    poll_fds[n_poll_fds].fd = tcp_sfd;
    poll_fds[n_poll_fds].events = POLLIN;
    n_poll_fds++;
    poll_fds[n_poll_fds].fd = STDIN_FILENO;
    poll_fds[n_poll_fds].events = POLLIN;
    n_poll_fds++;

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

    while (1)
    {
        poll(poll_fds, n_poll_fds, -1);
        int temp_fds = n_poll_fds;
        for (int i = 0; i < temp_fds; i++)
        {
            if (poll_fds[i].revents == 0)
            {
                continue;
            }
            if (poll_fds[i].revents & POLLIN)
            {
                if (poll_fds[i].fd == nc_sfd)
                {
                    store_new_client(nc_sfd, client_info, &no_clients);
                }
                else if (poll_fds[i].fd == udp_sfd)
                {
                    handle_client_udp(udp_sfd, client_info, &no_clients);
                }
                else if (poll_fds[i].fd == tcp_sfd)
                {
                    handle_client_tcp(tcp_sfd, poll_fds, &n_poll_fds);
                }
                else if (poll_fds[i].fd == STDIN_FILENO)
                {
                    char *buffer = (char *)calloc(200, sizeof(char));
                    fgets(buffer, 200, stdin);
                    printf("Sending the buffer '%s' to NC\n", buffer);
                    tcp_send(nc_sfd, buffer);
                }
                else
                {
                    talk_with_client(&poll_fds[i].fd, udp_sfd, client_info, &no_clients);
                }
            }
        }
    }
}