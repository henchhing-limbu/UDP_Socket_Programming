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
#define DEFAULT_PORT		(5000)

// function declarations
void dg_server(int sockfd, struct sockaddr* pclientaddr, socklen_t clientlen);

int main(int argc, char* argv[]) {
	int sockfd;
	struct sockaddr_in servaddr, clientaddr;

	// creating the socket
	printf("SERVER: Creating socket.\n");
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf("SERVER: Error creating socket.\n");
		exit(EXIT_FAILURE);
	}
	
	// intializing every struct structure to 0
	// printf("SERVER: Initializing every structure to 0.\n");
	memset(&servaddr, 0, sizeof(servaddr));
	// filling up relevant members
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(DEFAULT_PORT);
	
	// bind the socket
	printf("SERVER: binding the socket.\n");
	if (bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
		printf("SERVER: Error binding socket.\n");
		exit(EXIT_FAILURE);
	}
	printf("SERVER: Calling dg_server.\n");
	dg_server(sockfd, (struct sockaddr*) &clientaddr, sizeof(clientaddr));
}

void dg_server(int sockfd, struct sockaddr* pclientaddr, socklen_t clientlen) {
	printf("SERVER: Entered dg_server.\n");
	int n;
	socklen_t len;
	char mesg[MAX_LINE];
	for ( ; ; ) {
		len = clientlen;
		n = recvfrom(sockfd, mesg, MAX_LINE, 0, pclientaddr, &len);
		if (n < 0) {
			printf("SERVER: Error receiving data from the client.\n");
			exit(EXIT_FAILURE);
		}
		printf("Received data from client.\n");
		if (sendto(sockfd, mesg, n, 0, pclientaddr, len) < 0) {
			printf("SERVER: Error sending data to the client.\n");
			exit(EXIT_FAILURE);
		}
		printf("Sent data to client.\n");	
	}
}

