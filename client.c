#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"

// Global constatns
#define MAX_LINE	(1000)
// TODO: change this
#define SERV_PORT	(5000)

// function declarations
// TODO: check for the pointer to pointer of sockaddr
void sendFile(FILE* fp, int sockfd, const struct sockaddr* pservaddr, socklen_t servlen);

int main(int argc, char* argv[]) {
	printf("Entered main.\n");
	int sockfd;
	// socket structure
	struct sockaddr_in servaddr;
	/*
	if (argc > 8) {
		printf("CLIENT: More than necessary arguments.\n");
		exit(EXIT_SUCCESS);
	}
	else if (argc < 8) {
		printf("CLIENT: Not enough arguments.\n");
	}
	*/
	char* ipAddress = argv[1];
	/*
	short int port = atoi(argv[2]);
	int filePathSize = strlen(argv[3]);
	char filePath[filePathSize];
	memcpy(filePath, argv[3], filePathSize);
	int format = atoi(argv[4]);
	if (format < 0 || format > 3) {
		printf("CLIENT: Incorrect format number.\nFormat number must be in the range 0-3.\n");
		exit(EXIT_SUCCESS);
	}
	int fileNameSize = strlen(argv[5]);
	char outFile[fileNameSize];
	memcpy(outFile, argv[5], fileNameSize);
	// loss probability
	float lossProbab = atof(argv[6]);
	// random see
	int randomSeed = atoi(argv[7]);
 	*/

	// setting all the struct structures to null value
	memset(&servaddr, 0, sizeof(servaddr));
	
	// assigning values to relevant data members
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);

	// set the remote IP address
	if (inet_aton(ipAddress, &servaddr.sin_addr) <= 0) {
		printf("CLIENT: Invalid remote IP address.\n");
		exit(EXIT_FAILURE);
	}
	
	// creating socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf("CLIENT: Error creating socket.\n");
		exit(EXIT_FAILURE);
	}
	
	// TODO: Unsure what is correct
	/*
	FILE* fp = fopen(filePath, "rb");
	if (fp == NULL) {
		printf("CLIENT: Failed to open the file.\n");
		exit(EXIT_FAILURE);
	}
	*/
	// sendFile(fp, sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	sendFile(stdin, sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	return EXIT_SUCCESS;	
}

// TODO: struct argument not sure if it's correct
void sendFile(FILE* fp, int sockfd, const struct sockaddr* pservaddr, socklen_t servlen) {
	printf("Entered sendFile.\n");
	int n;
	/*
	fseek(fp, 0, SEEK_END);
	unsigned long fileSize = ftell(fp);
	rewind(fp);
	// TODO: whether to make the buffersize equal to the filesize
	// TODO: or to make buffer size equal MAX_LINE
	char buffer[MAX_LINE];
	unsigned long bytesToSend = fileSize;
	unsigned long bytesSent;
	while (bytesToSend > 0) {
		if (bytesToSend > MAX_LINE) {
			fread(buffer, 1, MAX_LINE, fp);
			bytesSent = sendto(sockfd, buffer, MAX_LINE, 0, pservaddr, servlen);
		}
		else {
			fread(buffer, 1, bytesToSend, fp);
			bytesSent = sendto(sockfd, buffer, bytesToSend, 0, pservaddr, servlen);
		}
		bytesToSend -= bytesSent;
	}
	// sending filesize to the server
	Writeline(sockfd, &fileSize, sizeof(long));
	*/
	char sendline[MAX_LINE], recvline[MAX_LINE+1];
	while (fgets(sendline, MAX_LINE, fp) != NULL) {
		sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		n = recvfrom(sockfd, recvline, MAX_LINE, 0, NULL, NULL);
		recvline[n] = 0;
		fputs(recvline, stdout);
	}
}
