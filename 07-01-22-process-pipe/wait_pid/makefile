CC=gcc
CFLAGS=-I.
p1: p1.c p2 p3
	$(CC) p1.c -o p1 $(CFLAGS)
p2: p2.c
	$(CC) p2.c -o p2 $(CFLAGS)
p3: p3.c
	$(CC) p3.c -o p3 $(CFLAGS)
clean:
	rm -f p1 p2 p3
