all: proxy.out

%.out: %.c
	gcc $< -o $@ -lpthread

clean:
	rm -f *.out *.usd