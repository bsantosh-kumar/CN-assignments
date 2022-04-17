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
    u_char packet[] = "\x02\x03\x04\x02\x03\x04\xf1\xf2\xf1\xf2\xf1\xf2\x86\x00";
    int len = sizeof(packet) - 1;
    handle = pcap_open_live("eth0", 65535, 1, 1, errbuf);

    if (!handle)
    {
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
        // sleep(1);
    }
}