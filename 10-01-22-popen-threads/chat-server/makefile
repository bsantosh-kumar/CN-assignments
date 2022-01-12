CC=gcc
LP=-lpthread
all: server client
server: server.c
	$(CC) server.c -o server $(LP)
client: client.c
	$(CC) client.c -o client $(LP)
clean:
	rm -f server client send recieve