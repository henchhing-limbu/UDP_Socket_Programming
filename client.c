#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Global constatns
#define MAX_LINE	(1000)

// function declarations
void sendFile(FILE* fp, int sockfd, const (struct sockaddr_in) *pservaddr, socklen_t servlen);

int main(int argc, char* argv[]) {
	int sockfd;
	// socket structure
	struct sockaddr_in servaddr;

	if (argc > 8) {
		printf("CLIENT: More than necessary arguments.\n");
		exit(EXIT_SUCCESS);
	}
	else if (argc < 8) {
		printf("CLIENT: Not enough arguments.\n");
	}
	char* ipAddress = argv[1];
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
 
	// setting all the struct structures to null value
	bzero(&servaddr, sizeof(servaddr));
	
	// assigning values to relevant data members
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

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
	FILE* fp = fopen(filePath, "rb");
	if (fp == NULL) {
		printf("CLIENT: Failed to open the file.\n");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;	
}

// TODO: struct argument not sure if it's correct
void sendFile(FILE* fp, int sockfd, const (struct sockaddr_in) *pservaddr, socklen_t servlen) {
	int n;
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
			bytesSent = Sendto(sockfd, buffer, MAX_LINE, 0, pservaddr, servlen);
		}
		else {
			fread(buffer, 1, bytesToSend, fp);
			bytesSent = Sendto(sockfd, buffer, bytesToSend, 0, pservaddr, servlen);
		}
		bytesToSend -= bytesSent;
	}
	// sending filesize to the server
	Writeline(sockfd, &fileSize, sizeof(long)); 	
}
