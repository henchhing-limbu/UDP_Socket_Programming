#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Global constants
#define MAX_LINE		(1000)

// function declarations
void DieWithError(char *errorMessage);
int main (int argc, char *argv[]) {
	// socket descriptor
	int sockfd;
	struct sockaddr_in servAddr;				// echo server address
	struct sockaddr_in fromAddr;				// source address of echo
	unsigned short port;						// server port
	unsigned int fromSize; 						// in-out of address size for recvfrom
	char* ipAddress;							// IP address of server
	char* echoString;							// string to send to server
	char buffer[MAX_LINE];						// buffer for receiving data
	int echoStringLen;							// length of sent string
	int recvStringLen;							// length of received string
	unsigned long fileSize;						// fileSize
	
	// checking for correct number of arguemnts
	if (argc < 4) {
		printf("CLIENT: Not enough arguments.\n");
		exit(1);
	}
	else if (argc > 4) {
		printf("CLIENT: More than enough arguments.\n");
		exit(1);
	}
	ipAddress = argv[1];
	echoString = argv[2];
	printf("Input string: %s\n", echoString);
	// checking for the string length
	if ((echoStringLen = strlen(echoString)) > MAX_LINE) {
		printf("CLIENT: Echo word too long.\n");
		exit(EXIT_FAILURE);
	}
	port = atoi(argv[3]);
	printf("Port number: %d\n", port);
	
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
	
	FILE* fp = fopen("practice_project_test_file_1", "rb");
	if (fp == NULL) {
		printf("CLIENT: Failed to open the file.\n");
		exit(EXIT_FAILURE);
	}
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	rewind(fp);
	
	unsigned long bytesToSend = fileSize;
	unsigned long bytesSent;
	// sending filesize to the server
	printf("CLIENT: Filesize = %li\n", fileSize);
	if ((sendto(sockfd, &fileSize, sizeof(long), 0, (struct sockaddr*) &servAddr, sizeof(servAddr))) < 0) {
		printf("CLIENT: Error sending filesize to the server.\n");
		exit(EXIT_FAILURE);
	}
	printf("CLIENT: Sent file size to the server.\n");
	// sending data to the server
	while (bytesToSend > 0) {
		if (bytesToSend > MAX_LINE) {
			fread(buffer, 1, MAX_LINE, fp);
			bytesSent = sendto(sockfd, buffer, MAX_LINE, 0, (struct sockaddr*) &servAddr, sizeof(servAddr));
			if (bytesSent < 0) {
				printf("CLIENT: sending data to ther server failed.\n");
				exit(EXIT_FAILURE);
			}
		}
		else {
			fread(buffer, 1, bytesToSend, fp);
			bytesSent = sendto(sockfd, buffer, MAX_LINE, 0, (struct sockaddr*) &servAddr, sizeof(servAddr));
			if (bytesSent < 0) {
				printf("CLIENT: sending data to the server failed.\n");
				exit(EXIT_FAILURE);
			}
		}
		bytesToSend -= bytesSent;
		// printf("CLIENT: Bytes sent = %li\n", bytesSent);
	}
	printf("CLIENT: Sent file to the server.\n");
	// receiving a response
	fromSize = sizeof(fromAddr);
	// receive string from the server
	if ((recvStringLen = recvfrom(sockfd, buffer, MAX_LINE, 0, (struct sockaddr*) &fromAddr, &fromSize)) != echoStringLen ) {
		printf("CLIENT: recvfrom() failed.\n");
		// DieWithError("recvfrom() failed.");
		exit(EXIT_FAILURE);
	}
	if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
		printf("CLIENT: Received data from unknown source.\n");
		exit(EXIT_FAILURE);
	}
	// terminating the received data with null
	buffer[MAX_LINE] = '\0';
	printf("Received: %s\n", buffer);
	if (close(sockfd) < 0) {
		printf("CLIENT: Error closing the socket.\n");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}
