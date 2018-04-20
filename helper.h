#include <stdlib.h>
#include <stdio.h>	
#include <string.h>	
#include <errno.h>					
#include <signal.h> 				
#include <unistd.h>					
#include "sendlib.h"				

// GLOBAL CONSTANTS
#define MAX_LINE 1000	
#define TIMEOUT_SECS 1
#define MAX_TRIES 500			

// extern variables
extern long bytesReceived;								// bytes received by recevfrom() of server; used in receiving file data			
extern long bytesSent;									// bytes send by lossy_sendto() of client; used in sending file data
extern int tries;										// keeps track of number of tries
extern unsigned char seqNum;							// sequence number 
extern char sendPacketBuf[MAX_LINE + 1];				// contains data to be sent 
extern char recvPacketBuf[MAX_LINE + 1];				// receives data in here

// function declarations
void DieWithError(char* errorMessage);
unsigned long makePacket(char* packetBuf, char* seqNum, void* restrict dataBuf, unsigned long dataSize);
void extractPacket(char* packetBuf, char* seqNum, void* restrict dataBuf, unsigned long dataSize);
void sendAndWaitClnt(int sock, float lossProb, int seed, const struct sockaddr_in* destAddr, socklen_t destAddrLen, const struct sockaddr_in* fromAddr, socklen_t fromAddrLen, void* restrict sendBuf, unsigned long bytesToSend, void *restrict recvBuf, unsigned long bytesToReceive);
void sendAndWaitServ(int sock, float lossProb, int seed, const struct sockaddr_in* destAddr, socklen_t destAddrLen, void* restrict sendBuf, unsigned long bytesToSend, void *restrict recvBuf, unsigned long bytesToReceive);
void CatchAlarm(int ignored);