#include "helper.h"
int tries = 0;
// TODO: Try
// int seqNum = 0;

int sendAndWaitClnt(float lossProb, unsigned int seed, int sockfd, void *restrict buffer, size_t bufferSize, char* ack, 
const struct sockaddr* fromAddr, socklen_t fromAddrLen, const struct sockaddr* servAddr, socklen_t servAddrLen, char* seqNum) {
	int bytesSent;
	// sending filesize to the server
	if ((bytesSent = lossy_sendto(lossProb, seed, sockfd, buffer, bufferSize, servAddr, servAddrLen)) < 0) { 	
		printf("CLIENT: Error sending filesize to the server.\n");
		exit(EXIT_FAILURE);
	}
	// printf("CLIENT: Sent file size = %li\n", buffer);
	alarm(TIMEOUT_SECS);
	// while (((recvfrom(sockfd, &ack, sizeof(int), 0, (struct sockaddr *) fromAddr, &fromAddrLen)) < 0)) || (ack[0] !=  {
		//(recvfrom(sockfd, packet, sizeof(fileSize) + 1, 0, (struct sockaddr*) &clntAddr, &clntAddrLen)) < 0)
	int bytesReceived;
	while ((bytesReceived = (recvfrom(sockfd, ack, 2, 0, (struct sockaddr *) fromAddr, &fromAddrLen)) < 0) /* || (ack[0] == *seqNum) */ ){
		// alarm went off
		printf("CLIENT: Bytes received = %d\n", bytesReceived);
		// printf("Received Acknowledgement: %d\n", ack);
		if (errno == EINTR) {
			if (tries < MAX_TRIES) {
				printf("timed out, %d more tries...\n", MAX_TRIES - tries);
				if ((bytesSent = lossy_sendto(lossProb, seed, sockfd, buffer, bufferSize, servAddr, servAddrLen)) < 0) {
					DieWithError("sendto() failed");
				}
				alarm(TIMEOUT_SECS);
			}
			else
				DieWithError("No Response");
		}
		else
			DieWithError("recvfrom() failed");
	}
	alarm(0);
	printf("CLIENT: Received something\n");
	// resetting tries
	tries = 0;
	return bytesSent;
	/*
	// TODO: change
	if (ack[0] != *seqNum) {
		*seqNum = ack[0];
		return bytesSent;
	}
	else
		return sendAndWaitClnt(lossProb, seed, sockfd, buffer, bufferSize, ack, fromAddr, fromAddrLen, servAddr, servAddrLen, seqNum);
	*/
}

int sendAndWaitServ(float lossProb, unsigned int seed, int sockfd, void* restrict buffer, size_t bufferSize, char* ack, 
const struct sockaddr* fromAddr, socklen_t fromAddrLen, const struct sockaddr* servAddr, socklen_t servAddrLen) {
	// sequence number
	if (((char*)(buffer))[0] == 0)
		ack[0] = 1;
	else
		ack[0] = 0;
	
	//ack[0] = ((char*)(buffer))[0];
	
	
	int bytesReceived;
	// sending packet to the client
	if (lossy_sendto(lossProb, seed, sockfd, ack, 2, fromAddr, fromAddrLen) < 0) { 	
		printf("SERVER: Error sending filesize to the server.\n");
		exit(EXIT_FAILURE);
	}
	printf("SERVER: Sent acknowledgment to the client.\n");
	
	// printf("CLIENT: Sent file size = %li\n", buffer);
	alarm(TIMEOUT_SECS);
	while (((bytesReceived = recvfrom(sockfd, buffer, bufferSize, 0, (struct sockaddr*) fromAddr, &fromAddrLen)) < 0) /* || (ack[0] != ((char*)(buffer))[0]) */)  {
		if (errno == EINTR) {
			if (tries < MAX_TRIES) {
				printf("timed out, %d more tries...\n", MAX_TRIES - tries);
				if (lossy_sendto(lossProb, seed, sockfd, ack, 2, fromAddr, fromAddrLen) < 0) {
					DieWithError("sendto() failed");
				}
				printf("SERVER: Sent acknowledgment to the client.\n");
				alarm(TIMEOUT_SECS);
			}
			else
				DieWithError("No Response");
		}
		else
			DieWithError("recvfrom() failed");
	}
	alarm(0);
	// resetting tries
	tries = 0;
	return bytesReceived;
}

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void CatchAlarm(int ignored)
{
	tries += 1;
}

// makes packet
void makePacket(char* packet, char seqNum, char* dataBuffer, unsigned long dataSize) {
	memcpy(packet, &seqNum, 1);
	memcpy(packet + 1, dataBuffer, dataSize);
}

// extract packet
void extractPacket(char* packet, char* seqNum, char* dataBuffer, unsigned long dataSize) {
	memcpy(seqNum, packet, 1);
	memcpy(dataBuffer, packet + 1,  dataSize);
}