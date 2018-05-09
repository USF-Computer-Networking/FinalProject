/**
 * common.h
 *
 * This file defines common functionality between our server and client.
 */
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h>

#ifndef _COMMON_H_
#define _COMMON_H_

#define DEFAULT_PORT 5555
#define MAX_PATH 1024

#endif

struct __attribute__((__packed__)) netfs_msg_header {
    uint64_t msg_len;
    uint16_t msg_type;
};

enum msg_types {
  MSG_OPEN = 0,
  MSG_READ = 1,
  MSG_READDIR = 2,
  MSG_GETATTR = 3
};

ssize_t write_len(int fd, void *buf, size_t length);
int read_len(int fd, void *buf, size_t length);
int connect_to(char *hostname, int port);

