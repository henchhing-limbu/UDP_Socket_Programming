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
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf("SERVER: Error creating socket.\n");
		exit(EXIT_FAILURE);
	}
	
	// intializing every struct structure to 0
	memset(&servaddr, 0, sizeof(servaddr));
	// filling up relevant members
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(DEFAULT_PORT);
	
	// bind the socket
	if (bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
		printf("SERVER: Error binding socket.\n");
		exit(EXIT_FAILURE);
	}
	dg_server(sockfd, (struct sockaddr*) &clientaddr, sizeof(clientaddr));	
	return 0;
}

void dg_server(int sockfd, struct sockaddr* pclientaddr, socklen_t clientlen) {
	int n;
	socklen_t len;
	char mesg[MAX_LINE];
	for ( ; ; ) {
		len = clientlen;
		n = recvfrom(sockfd, mesg, MAX_LINE, 0, pclientaddr, &len);
		sendto(sockfd, mesg, n, 0, pclientaddr, len);	
	}
}

