/*
    Packet sniffer using libpcap library
*/
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    char errbuf[100];
    pcap_t *handle = NULL;
    u_char packet[] = "\xdc\xfb\x48\x0c\x6f\xd9\xf1\xf2\xf1\xf2\xf1\xf2\x86\x00";
    int len = sizeof(packet) - 1;
    printf("Came here\n");
    fflush(stdout);
    handle = pcap_open_live("eth0", 65535, 1, 1, errbuf);
    fflush(stdout);
    if (!handle)
    {
        printf("here too\n");
        exit(0);
    }
    int count = 0;
    while (1)
    {
        if (pcap_sendpacket(handle, packet, len) < 0)
        {
            printf("pcap_send error\n");
        }
        count++;
        printf("sent %d\n", count);
        sleep(1);
    }
}