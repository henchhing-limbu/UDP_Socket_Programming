#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>								// for sigaction()
// #include <errno.h>
// #include "sendlib.h"
#include "helper.h"


// Global constants
#define MAX_LINE		 (1000)

// function declarations

int main (int argc, char *argv[]) {
	// socket descriptor
	int sockfd;
	struct sockaddr_in servAddr;				// echo server address
	struct sockaddr_in fromAddr;				// source address of echo
	unsigned short port;						// server port
	unsigned int fromSize; 						// in-out of address size for recvfrom
	char* ipAddress;							// IP address of server
	char* filePath;								// path to the input file
	char buffer[MAX_LINE];						// buffer for receiving data
	// int echoStringLen;						// length of sent string
	// int recvStringLen;						// length of received string
	unsigned long fileSize;						// fileSize
	unsigned int seed;							// random seed
	unsigned long format;						// to format
	unsigned long outputFileNameSize;			// output file name size
	float lossProb;								// loss probability
	struct sigaction myAction;					// For setting signal handler
	char numBuffer[2];							// sending numbers to the server
	char* outputFileName;						// output file name
	char packet[MAX_LINE+1];
	
	// checking for correct number of arguemnts
	if (argc < 8) {
		printf("CLIENT: Not enough arguments.\n");
		exit(EXIT_FAILURE);
	}
	else if (argc > 8) {
		printf("CLIENT: More than enough arguments.\n");
		exit(EXIT_FAILURE);
	}
	ipAddress = argv[1];
	port = atoi(argv[2]);
	printf("Port number: %d\n", port);
	filePath = argv[3];
	printf("File Path: %s\n", filePath);
	format = atoi(argv[4]);
	if (format > 3 || format < 0) {
		printf("CLIENT: Incorrect format number. Format number range is 0 to 3.\n");
		exit(EXIT_FAILURE);
	}
	printf("Format: %li\n", format);
	outputFileName = argv[5];
	printf("output file name: %s\n", outputFileName);
	lossProb = atof(argv[6]);
	printf("loss probability: %.2f\n", lossProb);
	seed = atoi(argv[7]);
	printf("seed number: %d\n", seed);
	
	// creating UDP socket
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("SERVER: Couldn't creat socket.\n");
		exit(EXIT_FAILURE);
	}
	
	// set signal handler for alarm signal
	myAction.sa_handler = CatchAlarm;
	
	if (sigfillset(&myAction.sa_mask) < 0)				// block everything in handler
		DieWithError("sigfillset() failed");
	myAction.sa_flags = 0;
	
	if (sigaction(SIGALRM, &myAction, 0) < 0)
		DieWithError("sigaction() failed for SIGALRM");
	
	// constructing the server address structure
	// filling the server address structure with 0
	memset(&servAddr, 0, sizeof(servAddr));
	// filling the relevant data members
	servAddr.sin_family = AF_INET;						// internet addr family
	servAddr.sin_addr.s_addr = inet_addr(ipAddress);	// server ip address
	servAddr.sin_port = htons(port);					// server port
	
	FILE* fp = fopen(filePath, "rb");
	if (fp == NULL) {
		printf("CLIENT: Failed to open the file.\n");
		exit(EXIT_FAILURE);
	}
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	printf("File size = %lu\n", fileSize);
	rewind(fp);
	
	unsigned long bytesToSend = fileSize;
	printf("Bytes to send = %lu\n", bytesToSend);
	unsigned long bytesSent;
	// size of clnt address
	fromSize = sizeof(fromAddr);
	
	// sequence number
	char seqNum = 0;
	// declaring ack and initializing it
	char ack[2] = {seqNum, 0};
	
	// creating file size buffer
	int x = sizeof(long);
	char fileSizeBuffer[x];
	memcpy(fileSizeBuffer, &fileSize, x);
	
	// makes file size packet
	makePacket(packet, seqNum, fileSizeBuffer, x);
	
	// sending file size packet to the server
	printf("CLIENT: Sending filesize to the server.\n");
	sendAndWaitClnt(lossProb, seed, sockfd, packet, x + 1, ack, (struct sockaddr*) &fromAddr, fromSize, 
			(struct sockaddr*) &servAddr, sizeof(servAddr), &seqNum); 
	printf("CLIENT: Received acknowledgment from the server.\n");
	
	// sending data to the server
	while (bytesToSend > 0) {
		printf("CLIENT: Sending file data to the server.\n");
		if (bytesToSend > MAX_LINE) {
			fread(buffer, 1, MAX_LINE, fp);
			// TODO: change
			makePacket(packet, seqNum, buffer, MAX_LINE);
			// sending packet to the server
			printf("CLIENT: Sending data to the server.\n");
			bytesSent = sendAndWaitClnt(lossProb, seed, sockfd, packet, MAX_LINE + 1, ack, (struct sockaddr*) &fromAddr,
			fromSize,(struct sockaddr*) &servAddr, sizeof(servAddr), &seqNum);
			printf("CLIENT: Received acknowledgment for the bytes sent.\n");
		}
		else {
			// creating packet
			fread(buffer, 1, bytesToSend, fp);
			// TODO: change
			makePacket(packet, seqNum, buffer, bytesToSend);
			// sending packet to the server
			printf("CLIENT: Sending data to the server.\nThis is the last data packet.\n");
			bytesSent = sendAndWaitClnt(lossProb, seed, sockfd, packet, bytesToSend + 1, ack, (struct sockaddr*) &fromAddr, fromSize,
			(struct sockaddr*) &servAddr, sizeof(servAddr), &seqNum);
			printf("CLIENT: Received acknowledgment for the bytes sent.\n");
		}
		bytesToSend += 1;
		bytesToSend -= bytesSent;
		printf("Bytes to send = %lu \n", bytesToSend);
	}
	printf("CLIENT: Sent file to the server.\n");
	
	// creating packet for format number
	char formatBuffer[x];
	memcpy(formatBuffer, &format, x);
	makePacket(packet, seqNum, formatBuffer, x);
	// makePacket(packet, seqNum, &format, sizeof(format));
	
	// sending format to the server
	printf("CLIENT: Sending format to the server.\n");
	sendAndWaitClnt(lossProb, seed, sockfd, packet, x + 1, ack, (struct sockaddr*) &fromAddr, fromSize, (struct sockaddr*) &servAddr,
	sizeof(servAddr), &seqNum);
	printf("CLIENT: Acknowledgment for format number received.\n");
	
	outputFileNameSize = strlen(outputFileName);
	
	// creating packet for output file name size
	char outFileNameSizeBuf[x];
	memcpy(outFileNameSizeBuf, &outputFileNameSize, x);
	makePacket(packet, seqNum, outFileNameSizeBuf, x);
	
	// sending output file name size to the server
	printf("CLIENT: Sending output file name size to the server.\n");
	sendAndWaitClnt(lossProb, seed, sockfd, packet, x + 1, ack, (struct sockaddr*) &fromAddr, fromSize, (struct sockaddr*) &servAddr,
	sizeof(servAddr), &seqNum);
	printf("CLIENT: Received acknowledgment for output file name size.\n");
	// printf("CLIENT: size sent = %d\n", outputFileNameSize);
	
	// creating packet
	// adding sequence number
	char outFileName[outputFileNameSize];
	memcpy(outFileName, outputFileName, outputFileNameSize);
	makePacket(packet, seqNum, outFileName, outputFileNameSize);
	
	// sending output file name to the server
	printf("CLIENT: Sending output file name to the server.\n");
	sendAndWaitClnt(lossProb, seed, sockfd, packet, outputFileNameSize + 1, ack, (struct sockaddr*) &fromAddr, fromSize, 
	(struct sockaddr*) &servAddr, sizeof(servAddr), &seqNum);
	printf("CLIENT: Received acknowledgment for output file name.\n");
	
	// Send the client done ack to the server and wait for the Server Response 
	// Get rid of the solo recv from below this 
	// Use the send and wait function
	
	unsigned long clntDone = 1;
	char clntDoneBuf[x];
	memcpy(clntDoneBuf, &clntDone, x);
	makePacket(packet, seqNum, clntDoneBuf, x);
	// Sending client done acknowledgment to the server
	// Getting the confirmation(error) message from the server
	sendAndWaitClnt(lossProb, seed, sockfd, packet, x + 1, ack, (struct sockaddr*) &fromAddr, fromSize, (struct sockaddr*) &servAddr,
	sizeof(servAddr), &seqNum);
	printf("CLIENT: Received error message from the server.\n");
	
	/*
	printf("CLIENT: Sending client done acknowledgment to the server.\n");
	sendAndWaitClnt(lossProb, seed, sockfd, packet, 
	// fromSize = sizeof(fromAddr);
	if (recvfrom(sockfd, &errorMessage, sizeof(int), 0, (struct sockaddr*) &fromAddr, &fromSize) < 0) {
		printf("CLIENT: Error receiving errorMessage from the server.\n");
		exit(EXIT_FAILURE);
	}
	*/
	if (ack[1] < 0)  {
		printf("Format error\n");
		exit(EXIT_FAILURE);
	}
	printf("Success.\n");
	
	if (close(sockfd) < 0) {
		printf("CLIENT: Error closing the socket.\n");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
	