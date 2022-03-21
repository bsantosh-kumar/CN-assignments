
#define __USE_BSD       /* use bsd'ish ip header */
#include <sys/socket.h> /* these headers are for a Linux system, but */
#include <netinet/in.h> /* the names on other systems are easy to guess.. */
#include <netinet/ip.h>
#define __FAVOR_BSD /* use bsd'ish tcp header */
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define P 9013

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

int main(void)
{
    int s = socket(PF_INET, SOCK_RAW, IPPROTO_UDP); /* open raw socket */
    char datagram[4096];                            /* this buffer will contain ip header, tcp header,
			   and payload. we'll point an ip header structure
			   at its beginning, and a tcp header structure after
			   that to write the header values into it */

    struct ip *iph = (struct ip *)datagram;

    int one = 1;
    const int *val = &one;
    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
        printf("\n\t Warning: I was not able to set HDRINCL!\n");

    struct sockaddr_in sin;
    /* the sockaddr_in containing the dest. address is used
			   in sendto() to determine the datagrams path */

    sin.sin_family = AF_INET;
    sin.sin_port = htons(9012);
    /* you byte-order >1byte header values to network
			      byte order (not needed on big endian machines) */
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    printf("Addr:%s", inet_ntoa((struct in_addr)sin.sin_addr));
    memset(&datagram, 0, 4096); /* zero out the buffer */

    /* we'll now fill in the ip/tcp header values, see above for explanations */
    iph->ip_hl = 5;
    iph->ip_v = 4;
    iph->ip_tos = 0;
    iph->ip_len = sizeof(struct ip); /* no payload */
    iph->ip_id = htonl(54321);       /* the value doesn't matter here */
    iph->ip_off = 0;
    iph->ip_ttl = 255;
    iph->ip_p = 15;
    iph->ip_sum = 0;                           /* set it to 0 before computing the actual checksum later */
    iph->ip_src.s_addr = inet_addr("1.2.3.4"); /* SYN's can be blindly spoofed */
    iph->ip_dst.s_addr = inet_addr("127.0.0.1");

    iph->ip_sum = csum((unsigned short *)datagram, iph->ip_len >> 1);

    printf("Addr:%s", inet_ntoa((struct in_addr)sin.sin_addr));

    printf("\n\n\t Headers have ben received, as : ");
    print_ip_header(iph);
    printf("%s\n", inet_ntoa(iph->ip_dst));
    char *message = datagram + (iph->ip_hl << 2);
    strcpy(message, "Hi\n");
    // while (1)
    {
        if (sendto(s,                       /* our socket */
                   datagram,                /* the buffer containing headers and data */
                   sizeof(datagram),        /* total length of our datagram */
                   0,                       /* routing flags, normally always 0 */
                   (struct sockaddr *)&sin, /* socket addr, just like in */
                   sizeof(sin)) > 0)        /* a normal send() */
            printf("Success\n");
    }
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
    printf("%s\n", datagram + sizeof(struct ip));

    return 0;
}
