#define __USE_BSD /* use bsd'ish ip header */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#define __FAVOR_BSD /* use bsd'ish tcp header */
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define P 9013

void print_ip_header(struct ip *iph)
{
    printf("header length:%d\n \
            version:%d\n\
            TOS:%d\n \
            IP Len:%d\n \
            id: %d\n \
            offest:%d\n \
            TTL:%d\n \
            Prot No:%d\n \
            Sum:%d\n \
            scr addr:%s\n \
            dest addr:%s\n",
           iph->ip_hl,
           iph->ip_v,
           iph->ip_tos,
           iph->ip_len,
           iph->ip_id,
           iph->ip_off,
           iph->ip_ttl,
           iph->ip_p,
           iph->ip_sum,
           inet_ntoa(iph->ip_src),
           inet_ntoa(iph->ip_dst));
}

int main()
{

    int rsfd = -1;
    struct sockaddr_in sin;
    int addrlen = sizeof(sin);

    char msg[4096] = {0};

    if ((rsfd = socket(AF_INET, SOCK_RAW, 200)) == -1)
    {
        perror("Error in socket");
        exit(0);
    }
    int one = 1;
    const int *val = &one;
    if (setsockopt(rsfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        printf("\n\t Warning: I was not able to set HDRINCL!\n");

    // printf("rsfd:%d\n",rsfd);
    // scanf("%d",&rsfd);

    sin.sin_family = PF_INET;

    if (inet_pton(PF_INET, "127.0.0.1", &sin.sin_addr) <= 0)
    {
        perror("Error in network address");
        exit(0);
    }
    // if (bind (rsfd, (struct sockaddr *)&sin, sizeof(sin)) == -1){
    // 	perror ("Error in bind\n");
    // }
    sin.sin_port = htons(6969);
    if (recvfrom(rsfd, msg, 4096, 0, (struct sockaddr *)&sin, (socklen_t *)&addrlen) < 0)
        perror("Error in recieve from\n");
    printf("Message : %s\n", msg);
    struct ip *iph = (struct ip *)msg;
    print_ip_header(iph);
    char *payload = msg + sizeof(struct ip);
    printf("%s\n", payload);
    return 0;
}