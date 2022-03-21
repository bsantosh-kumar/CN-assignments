all: sniffer.out

%.out: %.c
	gcc $< -o $@ -lpthread

clean:
	rm -f *.out *.uds