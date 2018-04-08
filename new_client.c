#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Global constant
#define MAX_LINE		(25)

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
	char buffer[MAX_LINE+1];					// buffer for receiving echoed string
	int echoStringLen;							// length of sent string
	int recvStringLen;							// length of received string
	
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
	
	// sending string to the server
	int x ;
	if ((x = sendto(sockfd, echoString, echoStringLen, 0, (struct sockaddr*) &servAddr, sizeof(servAddr))) != echoStringLen) {
		printf("echoString length: %d\n", echoStringLen);
		printf("sentBytes length: %d\n", x);
		printf("CLIENT: sendto sent different number of bytes than expected.\n");
		exit(EXIT_FAILURE);
	}
	printf("sent string length: %d\n", x);
	printf("echo String Length: %d\n", echoStringLen);
	// receiving a response
	fromSize = sizeof(fromAddr);
	// receive string from the server
	if ((recvStringLen = recvfrom(sockfd, buffer, MAX_LINE, 0, (struct sockaddr*) &fromAddr, &fromSize)) != echoStringLen ) {
		// printf("Sent string length: %d\n", echoStringLen);
		// printf("received string length: %d\n", recvStringLen);
		// printf("CLIENT: recvfrom() failed.\n");
		DieWithError("recvfrom() failed.");
		// exit(EXIT_FAILURE);
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