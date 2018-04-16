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
// TODO: Try 
// extern int seqNum;

// function declarations
void DieWithError(char* errorMessage);
void CatchAlarm(int ignored);
int sendAndWaitClnt(float lossProb, unsigned int seed, int sockfd, void* restrict buffer, size_t bufferSize,
 char* ack, const struct sockaddr* fromAddr, socklen_t fromAddrLen, const struct sockaddr* servAddr, socklen_t servAddrLen);

 int sendAndWaitServ(float lossProb, unsigned int seed, int sockfd, void* restrict buffer, size_t bufferSize, char* ack, 
const struct sockaddr* fromAddr, socklen_t fromAddrLen, const struct sockaddr* servAddr, socklen_t servAddrLen);
void makePacket(char* packet, char seqNum, char* dataBuffer, unsigned long dataSize);
void extractPacket(char* packet, char* seqNum, char* dataBuffer, unsigned long dataSize);