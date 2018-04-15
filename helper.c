#include "helper.h"
int tries = 0;

unsigned long timeout_loop(float lossProb, unsigned int seed, int sockfd, const void* buffer, size_t bufferSize, int ack, 
const struct sockaddr* fromAddr, socklen_t fromAddrLen, const struct sockaddr* servAddr, socklen_t servAddrLen) {
	unsigned long bytesSent;
	int x;
	// sending filesize to the server
	if ((bytesSent = lossy_sendto(lossProb, seed, sockfd, buffer, bufferSize, servAddr, servAddrLen)) < 0) { 	
		printf("CLIENT: Error sending filesize to the server.\n");
		exit(EXIT_FAILURE);
	}
	// printf("CLIENT: Sent file size = %li\n", buffer);
	alarm(TIMEOUT_SECS);
	while ((x = recvfrom(sockfd, &ack, sizeof(int), 0, (struct sockaddr*) fromAddr, &fromAddrLen)) < 0) {
		// alarm went off
		// printf("Entered here: %d\n", x);
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
	// resetting tries
	tries = 0;
	return bytesSent;
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
