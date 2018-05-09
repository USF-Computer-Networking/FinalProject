/**
 * netfs_client.h
 *
 * Implementation of the netfs client file system. Based on the fuse 'hello'
 * example here: https://github.com/libfuse/libfuse/blob/master/example/hello.c
 */

#define FUSE_USE_VERSION 31

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse3/fuse.h>
#include <netdb.h> 
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"
#include "logging.h"
#define TEST_DATA "hello world!\n"


/* Command line options */
static struct options {
    int show_help;
    int port;
    char *server_name;
} options;

#define OPTION(t, p) { t, offsetof(struct options, p), 1 }

static const struct fuse_opt option_spec[] = {
    OPTION("-h", show_help),
    OPTION("--help", show_help),
    OPTION("--port=%d", port),
    OPTION("--server=%s", server_name),
    FUSE_OPT_END
};

static int netfs_getattr(
        const char *path, struct stat *stbuf, struct fuse_file_info *fi) {

    LOG("getattr: %s\n", path);

    /* Clear the stat buffer */
    memset(stbuf, 0, sizeof(struct stat));

    /* By default, we will return 0 from this function (success) */
    int res = 0;

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path+1, "test_file") == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(TEST_DATA);
    } else {
        res = -ENOENT;
    }

    return res;
}

static int netfs_readdir(
        const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
        struct fuse_file_info *fi, enum fuse_readdir_flags flags) {

    LOG("readdir: %s\n", path);

    /* By default, we will return 0 from this function (success) */
    int res = 0;
    
    struct netfs_msg_header req_header = { 0 };
    req_header.msg_type = MSG_READDIR;
    req_header.msg_len = strlen(path) + 1;

   
    int server_fd = connect_to(options.server_name, DEFAULT_PORT);
    if (server_fd < 0) {
    	perror("connect");       
    }
    write_len(server_fd, &req_header, sizeof(struct netfs_msg_header));
    write_len(server_fd, path, req_header.msg_len);
    uint16_t reply_len;
    char reply_path[MAX_PATH] = { 0 };
    do {
    	read_len(server_fd, &reply_len, sizeof(uint16_t));
	read_len(server_fd, reply_path, reply_len);
	filler(buf, reply_path, NULL, 0, 0);
	printf("-> %s\n", reply_path);
    } while (reply_len > 0);
    
    close(server_fd);
    /* We only support one directory: the root directory. */
    if (strcmp(path, "/") != 0) {
        return -ENOENT;
    } else {
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);
        /* Create our single test file */
        filler(buf, "test_file", NULL, 0, 0);
    }

    return res;
}

static int netfs_open(const char *path, struct fuse_file_info *fi) {

    LOG("open: %s\n", path);

    /* By default, we will return 0 from this function (success) */
    int res = 0;
    if (strcmp(path+1, "test_file") != 0) {
        return -ENOENT;
    }

    /* We only support opening the file in read-only mode */
    if ((fi->flags & O_ACCMODE) != O_RDONLY) {
        return -EACCES;
    }

    return res;
}

static int netfs_read(
        const char *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *fi) {

    LOG("read: %s\n", path);

    if(strcmp(path+1, "test_file") != 0) {
        /* We only support one file (test_file)...*/
        return -ENOENT;
    }

    size_t len;
    len = strlen(TEST_DATA);
    if (offset < len) {
        if (offset + size > len) {
            size = len - offset;
        }
        /* Note how the read request may not start at the beginning of the file.
         * We take the offset into account here: */
        memcpy(buf, TEST_DATA + offset, size);
    } else {
        size = 0;
    }

    return size;
}

/* This struct maps file system operations to our custom functions defined
 * above. */
static struct fuse_operations netfs_client_ops = {
    .getattr = netfs_getattr,
    .readdir = netfs_readdir,
    .open = netfs_open,
    .read = netfs_read,
};

static void show_help(char *argv[]) {
    printf("usage: %s [options] <mountpoint>\n\n", argv[0]);
    printf("File-system specific options:\n"
            "    --port=<n>          Port number to connect to\n"
            "                        (default: %d)"
            "\n", DEFAULT_PORT);
}

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    /* Set up default options: */
    options.port = DEFAULT_PORT;

    /* Parse options */
    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
        return 1;
    }

    if (options.show_help) {
        show_help(argv);
        assert(fuse_opt_add_arg(&args, "--help") == 0);
        args.argv[0] = (char*) "";
    }

    return fuse_main(args.argc, args.argv, &netfs_client_ops, NULL);
}
