all: pcap-init.out packet-send.out packet-recieve.out

%.out: %.c
	gcc $< -o $@ -lpcap

clean:
	rm -f *.out *.uds log*