
CFLAGS += -Wall -g -I/usr/include/fuse3 -lpthread -lfuse3 -D_FILE_OFFSET_BITS=64

LDFLAGS +=

all: netfs_client netfs_server

netfs_client: netfs_client.o common.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(client_flags) $^ -o $@
netfs_server: netfs_server.o common.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

common.o: common.c common.h logging.h

netfs_client.o: netfs_client.c netfs_server.c common.h logging.h
netfs_server.o: netfs_client.c netfs_server.c common.h logging.h



clean:
	rm -f netfs_client netfs_server

