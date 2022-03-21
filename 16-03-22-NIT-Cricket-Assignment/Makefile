all: batsman.out bowler.out umpire.out fielder.out

%.out: %.c
	gcc $< -o $@ -lpthread

clean:
	rm -f *.out *.uds