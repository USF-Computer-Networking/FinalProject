# Final Project: Network File System

Implementaion of a Linux file system. File system operates over the network, allowing client machines to view the files on 
remote server. This is accomplished using TCP sockets as well as the FUSE 3 library.

To run the server:

./netfs_server <starting directory> <port number>

To run client:

./netfs_client --server=SERVER_NAME --port=PORT_NUMBER -f <path to mount point>

Now, a user can cd into the mount point. The contents will reflect that of the directory that was specified when starting up 
the server.

