CFLAGS=-Wall

all: epoll-server sync-server

epoll-server: epoll-server.o
	$(CC) $< -o $@

sync-server: sync-server.o
	$(CC) $< -o $@

clean:
	rm -f sync-server epoll-server *.o
