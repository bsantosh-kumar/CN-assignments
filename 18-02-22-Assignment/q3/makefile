all: P1.out P2.out P3.out P4.out client.out server.out echo.out

%.out: %.c
	gcc $< -o $@ -lpthread

clean:
	rm -f *.out