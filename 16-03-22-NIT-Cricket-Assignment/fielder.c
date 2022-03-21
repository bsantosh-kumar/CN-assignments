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
#include <signal.h>
#include <sys/un.h>
#include <time.h>
#define SA struct sockaddr
#define BOW_UMP "bowler_umpire.uds"
#define BAT_UMP "batsman_umpire.uds"
#define UMP "umpire.uds"
#define BOW "bowler.uds"
#define BAT_BOW "batsman_bowler.uds"
#define PROT_UMP 200
#define PROT_FIE 240
#define PROT_BAT 169

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
    char *point = (char *)iph + (iph->ip_hl << 2);
    strcpy(point, message);
    return iph;
}

int send_ip_packet(struct ip *iph, int sock)
{
    struct sockaddr_in for_addr;
    for_addr.sin_family = PF_INET;
    for_addr.sin_addr = iph->ip_dst;

    return sendto(sock, iph, iph->ip_len, 0, (SA *)&for_addr, sizeof(for_addr));
}

char *recv_from_raw(int rsfd, char *src_addr)
{
    char *buffer = (char *)malloc(4096 * sizeof(char));
    bzero(buffer, 4096);
    struct sockaddr_in for_addr;
    for_addr.sin_family = PF_INET;
    for_addr.sin_addr.s_addr = inet_addr(src_addr);
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
    if (argc != 2)
    {
        printf("Format is sudo ./fielder.out <number of fielder>\n");
        exit(EXIT_FAILURE);
    }
    int fielder_no = atoi(argv[1]);
    if (fielder_no == 0)
    {
        printf("Fielder number should be an integer\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Fielder number is:%d\n", fielder_no);
    }
    int left[4] = {0, 16, 26, 36};
    int right[4] = {15, 25, 35, 40};
    int f_rsfd = -1;
    if ((f_rsfd = socket(PF_INET, SOCK_RAW, PROT_FIE)) == -1)
    {
        perror("error in creation of raw socket\n");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    const int *val = &one;
    if (setsockopt(f_rsfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        printf("\n\t Warning: I was not able to set HDRINCL!\n");

    while (1)
    {
        struct ip *iph = (struct ip *)recv_from_raw(f_rsfd, "1.2.3.4");
        char *message = get_message_from_packet((char *)iph);
        int curr_runs = atoi(message);
        printf("curr runs:%d\n", curr_runs);
        if (curr_runs % 4 == 0 || curr_runs % 6 == 0)
            continue;
        else if (left[fielder_no - 1] <= curr_runs && right[fielder_no - 1] >= curr_runs)
        {
            iph = create_ip_packet("out", PROT_UMP, "127.0.0.1", "127.0.0.1");
            send_ip_packet(iph, f_rsfd);
            printf("Sent ip packet to umpire\n");
        }
    }
}
