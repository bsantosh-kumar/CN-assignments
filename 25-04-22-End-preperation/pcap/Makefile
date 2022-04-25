all: look-up-net.out basic-cap.out basic-pcap-loop.out basic-filter.out

%.out: %.c
	gcc $< -o $@ -lpthread -lpcap

clean:
	rm -f *.out *.usd