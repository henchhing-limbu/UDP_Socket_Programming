#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>								// for sigaction()
#include <errno.h>
#include "sendlib.h"

// Global constants
#define TIMEOUT_SECS	 (3)
#define MAX_TRIES		 (5)

extern int tries;

// function declarations
// function declarations
void DieWithError(char* errorMessage);
void CatchAlarm(int ignored);
// void timeout_loop(int sockfd, const void* buffer, size_t bufferSize, const struct sockaddr* fromAddr, socklen_t fromAddrLen, const struct sockaddr* servAddr, socklen_t servAddrLen);
unsigned long timeout_loop(float lossProb, unsigned int seed, int sockfd, const void* buffer, size_t bufferSize,
 int ack, const struct sockaddr* fromAddr, socklen_t fromAddrLen, const struct sockaddr* servAddr, socklen_t servAddrLen);