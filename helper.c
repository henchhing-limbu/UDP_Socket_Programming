#include "helper.h"

int tries;
long bytesReceived;					
long bytesSent;						
char sendPacketBuf[MAX_LINE + 1];
char recvPacketBuf[MAX_LINE + 1];
unsigned char seqNum;

void sendAndWaitClnt(int sock, float lossProb, int seed, const struct sockaddr_in* destAddr, socklen_t destAddrLen, const struct sockaddr_in* fromAddr, socklen_t fromAddrLen, void* restrict sendBuf, unsigned long bytesToSend, void *restrict recvBuf, unsigned long bytesToReceive) {
	char recvSeqNum;
	// changing the sequence number
	seqNum = (seqNum + 1) % 2;
	// calls makePacket functions that make packets to send to the server
	// returns length of the packet
	unsigned long packetLen = makePacket(sendPacketBuf, &seqNum, sendBuf, bytesToSend);

	// sending packet to the server
	if ((bytesSent = lossy_sendto(lossProb, seed, sock, sendPacketBuf, packetLen, (struct sockaddr *) destAddr, destAddrLen)) != packetLen)
		DieWithError("lossy_sendto() failed");
	// setting alarm
	alarm(TIMEOUT_SECS);
	tries = 0;
	
	// resetting every member of recvPacketBuf to -1
	memset (recvPacketBuf, -1, MAX_LINE + 1);
	
	// receiving pacekts from the client
	while (((bytesReceived = recvfrom(sock, recvPacketBuf, sizeof(char) + bytesToReceive, 0, (struct sockaddr *) fromAddr, &fromAddrLen)) < 0) || (recvPacketBuf[0] != seqNum)) {
		// printf("Expected Sequence Number = %d\n", seqNum);
		// printf("Received Sequence Number = %d\n", recvPacketBuf[0]);
		if (errno == EINTR)	{			
			if (tries < MAX_TRIES) {	
				printf("recvfrom() timed out, %d more tries\n", MAX_TRIES - tries);
				if ((bytesSent = lossy_sendto(lossProb, seed, sock, sendPacketBuf, packetLen, (struct sockaddr *) destAddr, destAddrLen)) != packetLen)
					DieWithError("lossy_sendto() failed");
				alarm(TIMEOUT_SECS);
			} 
			else
				DieWithError("No Response");
		} 
		else
			DieWithError("recvfrom() failed");
	}
	// received something
	// closing alarm
	alarm(0);
	// extracting data from the packet
	extractPacket(recvPacketBuf, &recvSeqNum, recvBuf, bytesToReceive);
}


void sendAndWaitServ(int sock, float lossProb, int seed, const struct sockaddr_in* destAddr, socklen_t destAddrLen, void* restrict sendBuf, unsigned long bytesToSend, void *restrict recvBuf, unsigned long bytesToReceive) {
	unsigned long packetLen = makePacket(sendPacketBuf, &seqNum, sendBuf, bytesToSend);
	// holds received sequence number from the client
	char recvSeqNum;
	// sending acknowledment to the client
	if ((bytesSent = lossy_sendto(lossProb, seed, sock, sendPacketBuf, packetLen, (struct sockaddr *) destAddr, destAddrLen)) != packetLen)
		DieWithError("lossy_sendto() failed");
	
	// changing the sequence number
	seqNum = (seqNum + 1) % 2;
	// setting alarm
	alarm(TIMEOUT_SECS);
	// initializing tries to 0
	tries = 0;
	
	memset (recvPacketBuf, -1, MAX_LINE + 1);
	
	// receiving packets from the client
	while (((bytesReceived = recvfrom(sock, recvPacketBuf, sizeof(char) + bytesToReceive, 0, (struct sockaddr *) destAddr, &destAddrLen)) < 0) || (recvPacketBuf[0] != seqNum)) {
		// printf("Expected Sequence Number = %d\n", seqNum);
		// printf("Received Sequnece Number = %d\n", recvPacketBuf[0]);
		if (errno == EINTR)	{			
			if (tries < MAX_TRIES) {
				printf("recvfrom() timed out, %d more tries \n", MAX_TRIES - tries);
				if ((bytesSent = lossy_sendto(lossProb, seed, sock, sendPacketBuf, packetLen, (struct sockaddr *) destAddr, destAddrLen)) != packetLen)
					DieWithError("lossy_sendto() failed");
				alarm(TIMEOUT_SECS);
			} else
				DieWithError("No Response");
		} else
			DieWithError("recvfrom() failed");
	}
	// closing alarm
	alarm(0);
	// extracting information from the packet
	extractPacket(recvPacketBuf, &recvSeqNum, recvBuf, bytesToReceive);
}

// makes packet to send between the two ends (client and server)
// returns the length of the packet sent
unsigned long makePacket(char* packetBuf, char* seqNum, void* restrict dataBuf, unsigned long dataSize) {
	memcpy(packetBuf, seqNum, sizeof(char));
	memcpy(packetBuf + sizeof(char), dataBuf, dataSize);
	return dataSize + 1;
}

// extracts the data from the packet (separates each data field
void extractPacket(char* packetBuf, char* seqNum, void* restrict dataBuf, unsigned long dataSize) {
	memcpy(seqNum, packetBuf, sizeof(char));
	memcpy(dataBuf, packetBuf + sizeof(char), dataSize);
}

// increases the tries by 1 
void CatchAlarm(int ignored)
{
	tries += 1;
}

// prints the error message and exits the program
void DieWithError(char* errorMessage) {
	perror(errorMessage);
	exit(1);
}