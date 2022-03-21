
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

#define P 9013

int main(void)
{

    int s = socket(PF_INET, SOCK_RAW, 15);

    int n = 0;

    int one = 1;
    const int *val = &one;
    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        printf("Warning: Cannot set HDRINCL!\n");

    char datagram[4096];

    struct ip *iph = (struct ip *)datagram;

    struct sockaddr_in sin, sin2;
    socklen_t len = sizeof(sin2);

    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(P);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");

    // bind(s, (struct sockaddr *)&sin, sizeof(sin));

    if ((n = recvfrom(s, datagram, 4096, 0, (struct sockaddr *)&sin, &len)) < 0)
        printf("error\n");

    printf("n=%d\n", n);
    printf("\n\n\t Headers have ben received, as : ");
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
    char *message = datagram + sizeof(struct ip);
    printf("message:%s\n", message);
    return 0;
}
