all: proxy.out host-name.out
CFLAGS=-I/usr/include/openssl/
%.out: %.c
	gcc $< -o $@ -lpthread $(CFLAGS) -lcrypto -lssl

clean:
	rm -f *.out *.usd