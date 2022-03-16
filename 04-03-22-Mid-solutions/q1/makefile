all: D1.out D2.out D3.out B1.out B2.out V.out

%.out: %.c
	gcc $< -o $@ -lpthread

clean:
	rm -f *.out *.usd