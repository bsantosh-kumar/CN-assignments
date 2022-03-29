all: all-interfaces.out arp.out arp_request.out

%.out: %.c
	gcc $< -o $@ -lpthread

clean:
	rm -f *.out *.uds