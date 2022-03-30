all: NC.out P.out C.out

%.out: %.c
	gcc $< -o $@ -lpthread

clean:
	rm -f *.out *.uds