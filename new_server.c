#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "helper.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Global constants
#define MAX_LINE		(1000)

int main(int argc, char* argv[]) {
	int sockfd;												// socket
	struct sockaddr_in servAddr;							// server address
	struct sockaddr_in clntAddr;							// client address
	unsigned int clntAddrLen;								// length of incoming message
	char buffer[MAX_LINE];									// buffer for data
	unsigned short port;									// port number
	unsigned long recvSize;									// size of received data
	
	// checking for number of arguments
	if (argc != 2) {
		printf("SERVER: Not enough argumnets. 2 argumnets are needed.\n");
		exit(EXIT_SUCCESS);
	}
	
	port = atoi(argv[1]);
	printf("Port number: %d\n", port);
	// creating socket
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("SERVER: Error creating the socket.\n");
		exit(EXIT_FAILURE);
	}
	
	// initializing struct members with 0
	memset(&servAddr, 0, sizeof(servAddr));
	// filling up the relevant struct structure
	servAddr.sin_family = AF_INET;							// internet address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 			// any incoming interface
	servAddr.sin_port = htons(port);						// port
	
	// binding socket to the local address
	if (bind(sockfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
		printf("SERVER: Error binding the socket.\n");
		exit(EXIT_FAILURE);
	}
	
	while(1) {
		// set the size of the in-out parameter
		clntAddrLen = sizeof(clntAddr);
		
		memset(buffer, 0, MAX_LINE);
		
		// receiving file size from the client
		if ((recvfrom(sockfd, &recvSize, sizeof(long), 0, (struct sockaddr*) &clntAddr, &clntAddrLen)) < 0) {
			printf("SERVER: Error receiving the file size from the client.\n");
			exit(EXIT_FAILURE);
		}
		printf("SERVER: Received file size = %li\n", recvSize);
		
		unsigned long bytesToReceive = recvSize;
		unsigned long bytesReceived;
		
		// file to write to 
		FILE* fp = fopen("Received","wb");
		
		while (bytesToReceive > 0) {
			if (bytesToReceive > MAX_LINE) {
				if ((bytesReceived = recvfrom(sockfd, buffer, MAX_LINE, 0, (struct sockaddr*) &clntAddr, &clntAddrLen)) < 0) {
					printf("SERVER: Error receiving data from the client.\n");
					exit(EXIT_FAILURE);
				}
			}
			else {
				if ((bytesReceived = recvfrom(sockfd, buffer, bytesToReceive, 0, (struct sockaddr*) &clntAddr, &clntAddrLen)) < 0) {
					printf("SERVER: Error receiving data from the client.\n");
					exit(EXIT_FAILURE);
				}
			}
			// printf("SERVER: Bytes received = %li\n", bytesReceived);
			fwrite(buffer, 1, bytesReceived, fp);
			bytesToReceive -= bytesReceived;
		}
		printf("SERVER: Received file.\n");
		
		fclose(fp);
		// sending received data back to the client
		/*
		if (sendto(sockfd, buffer, recvSize, 0, (struct sockaddr*) &clntAddr, sizeof(clntAddr)) != recvSize) {
			printf("SERVER: Error sending data back to the client.\n");
			exit(EXIT_FAILURE);
		}
		*/
	}
	//return EXIT_SUCCESS;
}