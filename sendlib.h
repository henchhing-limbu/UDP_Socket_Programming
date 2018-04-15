#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

// function declaration
ssize_t lossy_sendto (float loss_ratio, unsigned int random_seed, int sockfd, const void * buf, size_t len,
                     const struct sockaddr *dest_addr, socklen_t addrlen);