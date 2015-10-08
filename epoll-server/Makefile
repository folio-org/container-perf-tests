CFLAGS=-Wall

all: epoll-server sync-server

epoll-server: epoll-server.o common.o
	$(CC) $? -o $@

sync-server: sync-server.o common.o
	$(CC) $? -o $@

clean:
	rm -f sync-server epoll-server *.o