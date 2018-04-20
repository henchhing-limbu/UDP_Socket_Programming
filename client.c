#include "helper.h"

int main (int argc, char *argv[]) {
	int sockfd;									// socket descriptor
	struct sockaddr_in servAddr;				// echo server address
	struct sockaddr_in fromAddr;				// source address of echo
	unsigned short port;						// server port
	unsigned int fromSize; 						// in-out of address size for recvfrom
	char* ipAddress;							// IP address of server
	char* filePath;								// path to the input file
	char buffer[MAX_LINE];						// buffer for receiving data
	long fileSize;								// fileSize
	unsigned int seed;							// random seed
	int format;									// to format
	unsigned long outputFileNameSize;			// output file name size
	float lossProb;								// loss probability
	struct sigaction myAction;					// For setting signal handler
	char* outputFileName;						// output file name
	char ack;									// acknowledgment from server
	seqNum = 0;									// sequence no. initialized to 0
	char clientDone = 1;						// client is done message
	char errorMessage;							// error message from server
	tries = 0;									// int
	
	// checking for correct number of arguments
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
	printf("Format: %d\n", format);
	outputFileName = argv[5];
	printf("output file name: %s\n", outputFileName);
	lossProb = atof(argv[6]);
	printf("loss probability: %.2f\n", lossProb);
	seed = atoi(argv[7]);
	printf("seed number: %d\n", seed);
	
	// set signal handler for alarm signal
	myAction.sa_handler = CatchAlarm;
	
	// block everything in handler
	if (sigfillset(&myAction.sa_mask) < 0)				
		DieWithError("sigfillset() failed");
	myAction.sa_flags = 0;
	
	if (sigaction(SIGALRM, &myAction, 0) < 0)
		DieWithError("sigaction() failed for SIGALRM");
	
	// creating UDP socket
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("SERVER: Couldn't creat socket.\n");
		exit(EXIT_FAILURE);
	}
	
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
	
	// sending file size to the server
	printf("CLIENT: Sending file size to the server.\n");
	sendAndWaitClnt(sockfd, lossProb, seed, &servAddr, sizeof(servAddr), &fromAddr, sizeof(fromAddr), 
	&fileSize, sizeof(fileSize), &ack, sizeof(ack));
	printf("CLIENT: Finished sending file size to  the server.\n");
	
	// bytes to send == file size
	long bytesToSend = fileSize;
	// printf("Bytes to send = %lu\n", bytesToSend);
	
	printf("CLIENT: Sending file.\n");
	while (bytesToSend > 0) {
		if (bytesToSend > MAX_LINE) {
			fread(buffer, 1, MAX_LINE, fp);
			sendAndWaitClnt(sockfd, lossProb, seed, 
			&servAddr, sizeof(servAddr), &fromAddr, 
			sizeof(fromAddr), buffer, MAX_LINE, 
			&ack, sizeof(ack));
		}
		else {
			fread(buffer, 1, bytesToSend, fp);
			sendAndWaitClnt(sockfd, lossProb, seed, 
			&servAddr, sizeof(servAddr), &fromAddr, 
			sizeof(fromAddr), buffer, bytesToSend, 
			&ack, sizeof(ack));
		}
		bytesToSend -= (bytesSent - 1);
	}
	printf("CLIENT: Finished sending file data to the server.\n");
	
	// sending format number to the server
	printf("CLIENT: Sending format number.\n");
	sendAndWaitClnt(sockfd, lossProb, seed, &servAddr, sizeof(servAddr), 
	&fromAddr, sizeof(fromAddr), &format, sizeof(format), &ack, sizeof(ack));
	printf("CLIENT: Finished sending format number.\n");
	
	// sending output file size to the server
	printf("CLIENT: Sending file size.\n");
	outputFileNameSize = strlen(outputFileName);
	sendAndWaitClnt(sockfd, lossProb, seed, &servAddr, sizeof(servAddr), 
	&fromAddr, sizeof(fromAddr), &outputFileNameSize, sizeof(outputFileNameSize), &ack, sizeof(ack));
	printf("CLIENT: Finished sending file size");
	
	// sending output file name to the server
	printf("CLIENT: Sending output file name.\n");
	sendAndWaitClnt(sockfd, lossProb, seed, &servAddr, sizeof(servAddr), 
	&fromAddr, sizeof(fromAddr), outputFileName, outputFileNameSize, &ack, sizeof(ack));
	printf("CLIENT: Finished sending output file name.\n");
	
	// sending "client done" message to the server
	printf("CLIENT: Sending client done.\n");
	sendAndWaitClnt(sockfd, lossProb, seed, &servAddr, sizeof(servAddr), 
	&fromAddr, sizeof(fromAddr), &clientDone, sizeof(clientDone), &errorMessage, sizeof(errorMessage));
	printf("CLIENT: Finsihed sending client done.\n");
	
	if (errorMessage < 0)  {
		printf("Format error\n");
		exit(EXIT_FAILURE);
	}
	printf("Success.\n");
	
	return 0;
}