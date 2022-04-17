all: all-interfaces.out arp.out arp_request.out rahul-arp.out

%.out: %.c
	gcc $< -o $@ -lpthread

clean:
	rm -f *.out *.uds