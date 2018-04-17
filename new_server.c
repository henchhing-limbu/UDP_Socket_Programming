#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "helper.h"

// Global constants
#define MAX_LINE		(1000)

// function declarations
uint8_t asciiToDecimal(uint8_t amount[]);
void decimalToAscii(uint8_t amount, uint8_t *array);
void writeToType0(FILE* outputStream, uint8_t type, uint8_t amount, uint16_t array[]);
void writeToType1(FILE* outputStream, uint8_t type, uint8_t* amount, int count, uint8_t* numbers);
void type0ToType1(uint8_t* amountArray, uint16_t* numbers, FILE* outputStream, int amount);
void type1ToType0(FILE* outputStream, uint8_t amount, uint8_t* numbers, int count);
char convertFile(int format, FILE* sourcefile, FILE* destfile);

int main(int argc, char* argv[]) {
	int sockfd;		
	struct sockaddr_in servAddr;							// server address
	struct sockaddr_in clntAddr;							// client address
	// unsigned int clntAddrLen;								// length of incoming message
	unsigned short port;									// port number
	unsigned long fileSize;									// size of received data
	unsigned long format;
	char buffer[MAX_LINE];									// buffer for received data
	char packet[MAX_LINE + 1];								// packet coming from the client.
	unsigned int seed;										// random seed
	float lossProb;											// loss probability
	struct sigaction myAction;								// for setting signal handler
	// char seqNum;											// sequence number
	
	// checking for number of arguments
	if (argc != 4) {
		printf("SERVER: Not enough argumnets. 2 argumnets are needed.\n");
		exit(EXIT_SUCCESS);
	}
	// port number
	port = atoi(argv[1]);
	printf("Port number: %d\n", port);
	// seed number
	seed = atoi(argv[2]);
	// loss Probability
	lossProb = atof(argv[3]);
	
	// creating socket
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("SERVER: Error creating the socket.\n");
		exit(EXIT_FAILURE);
	}
	
	// setting signal handler for alarm signal
	myAction.sa_handler = CatchAlarm;
	
	if (sigfillset(&myAction.sa_mask) < 0)
		DieWithError("sigfillset() failed");
	myAction.sa_flags = 0;
	
	if (sigaction(SIGALRM, &myAction, 0) < 0)
		DieWithError("sigaction() failed for SIGALRM");
	
	// constructing server address structure
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
	
	unsigned int clntAddrLen = sizeof(clntAddr);
	unsigned int servAddrLen = sizeof(servAddr);
	
	while(1) {
		// set the size of the in-out parameter
		clntAddrLen = sizeof(clntAddr);
		
		// initializing each member of buffer to be 0
		memset(buffer, 0, MAX_LINE);
		
		// receiving file size from the client
		if ((recvfrom(sockfd, packet, sizeof(fileSize) + 1, 0, (struct sockaddr*) &clntAddr, &clntAddrLen)) < 0) {
			printf("SERVER: Error receiving the file size from the client.\n");
			exit(EXIT_FAILURE);
		}
		printf("SERVER: Received file size.\n");
		// sequence number
		// ack[0] = packet[0];
		// acknowledgment
		// acknowledgment; contains acknowledgment number and sequence number
		char ack[2];
		ack[1] = 1;		
		
		unsigned long bytesToReceive;
		unsigned long bytesReceived;
		// getting bytesToReceive fromt the packet
		memcpy(&bytesToReceive, packet + 1, sizeof(fileSize));
				
		// file to write to 
		FILE* fp = fopen("Received","wb");
		
		while (bytesToReceive > 0) {
			printf("SERVER: Bytes To Receive = %li\n", bytesToReceive);
			// printf("SERVER: Receiving data from the client.\n");
			if (bytesToReceive > MAX_LINE) {
				// sending acknowledgment for the received file size
				bytesReceived = sendAndWaitServ(lossProb, seed, sockfd, packet, MAX_LINE + 1, ack, (struct sockaddr*) &clntAddr, clntAddrLen, (struct sockaddr*) &servAddr, servAddrLen);
				printf("SERVER: Received data packet fromt the client.\n");
		
			}
			else {
				// printf("This is the last packet.\n");
				bytesReceived = sendAndWaitServ(lossProb, seed, sockfd, packet, bytesToReceive + 1, ack, (struct sockaddr*) &clntAddr, clntAddrLen, (struct sockaddr*) &servAddr, servAddrLen);
				printf("SERVER: Received data packet from the client.\n");
			}
			printf("SERVER: Bytes received = %li\n", bytesReceived);
			// TODO: change the addition of sequence number in every packet
			fwrite(packet + 1, 1, bytesReceived - 1, fp);
			bytesToReceive += 1;
			bytesToReceive -= bytesReceived;
		}
		printf("SERVER: Received file.\n");
		
		fclose(fp);
		
		// sending acknowledgment for the data received
		// receiving the file foramt here
		sendAndWaitServ(lossProb, seed, sockfd, packet, sizeof(long) + 1, ack, (struct sockaddr*) &clntAddr, clntAddrLen, (struct sockaddr*) &servAddr, servAddrLen);
		
		memcpy(&format, packet + 1, sizeof(long));
		printf("SERVER: Received format = %li\n", format);
		
		// sending acknowledgment to the server
		// receiving output filename size from the server
		unsigned long outputFileNameSize;
		sendAndWaitServ(lossProb, seed, sockfd, packet, sizeof(long) + 1, ack, (struct sockaddr*) &clntAddr, clntAddrLen, (struct sockaddr*) &servAddr, servAddrLen);

		memcpy(&outputFileNameSize, packet + 1, sizeof(long));
		printf("SERVER: Received output file name size = %li\n", outputFileNameSize);

		// sending acknowledgment for the output file name size to the client
		// receiving output file name from the client.
		char outputFileName[outputFileNameSize];
		sendAndWaitServ(lossProb, seed, sockfd, packet, outputFileNameSize + 1, ack, (struct sockaddr*) &clntAddr, clntAddrLen, (struct sockaddr*) &servAddr, servAddrLen);
		memcpy(outputFileName, packet + 1, outputFileNameSize);
		printf("SERVER: Received output file name = %s\n", outputFileName);
		
		// TODO: send with loss probability
		// sending acknowledgement for the output file name to the client
		// Send the ack for the output file name and with for client done ack USE SEND AND WAIT
		sendAndWaitServ(lossProb, seed, sockfd, packet, sizeof(long) + 1, ack, (struct sockaddr*) &clntAddr, clntAddrLen, (struct sockaddr*) &servAddr, servAddrLen);
		printf("SERVER: Received client done acknowledgement from the client.\n");
		
		FILE* destfile = fopen(outputFileName, "wb");
		fp = fopen("Received","rb");
		
		char errorMessage = convertFile(format, fp, destfile);
		ack[1] = errorMessage;
		
		fclose(destfile);
		fclose(fp);
		
		if (errorMessage < 0)
			remove(outputFileName);
		remove("Recevied");
		
		// sending acknowledgment to the client
		for (int i = 0; i < MAX_TRIES; i++) {
			lossy_sendto(lossProb, seed, sockfd, ack, ACK_SIZE, (struct sockaddr*) &clntAddr, clntAddrLen);
		}
		/*
		if (sendto(sockfd, &errorMessage, sizeof(int), 0, (struct sockaddr*) &clntAddr, sizeof(clntAddr)) < 0) {
			printf("SERVER: Error sending data back to the client.\n");
			exit(EXIT_FAILURE);
		}
		*/
	}
	//return EXIT_SUCCESS;
}

char convertFile(int format, FILE* sourcefile, FILE* outputStream) {
	printf("Entered convertFile\n");
	// moving the pointer to the end of the file
	fseek(sourcefile, 0, SEEK_END);
	// gives the offset
	long fileSize = ftell(sourcefile);
	// moving the pointer back to the start of the file
	fseek(sourcefile, 0, SEEK_SET);
	// gives the offset which must be 0
	long temp = ftell(sourcefile);
	
	// printing filesize and temp
	// printf("Filesize: %lu\n", fileSize);
	// printf("Temp: %lu\n", temp);
	
	// loop until the end of the file
	while (temp < fileSize) {
		// firstByte either 0 or 1
		uint8_t type = fgetc(sourcefile);
		printf("Type %d", type);
		printf("\t");
		
		// Type 0
		if (type == 0) {
			uint8_t amount = fgetc(sourcefile);
			uint8_t amountArray[3];
			// calls the function to convert the decimal amount 
			// to ASCII characters stored in an array
			decimalToAscii(amount, amountArray);
			
			// printing the amount
			printf("Amount: ");
			for (int i = 0; i < 3; i++) {
				printf("%c", amountArray[i]);
			}
			printf("\t");
			
			// array to store the numbers
			uint16_t numbers[amount];
			// reading the file and storing the numbers in the array
			fread(numbers, 2, amount, sourcefile);
			// changing the Endianess
			uint16_t x, y;
			
			// print the numbers
			for (int i = 0; i < amount; i++) {
				// sll by 8
				x = numbers[i] << 8;
				// srl by 8
				y = numbers[i] >> 8;
				// 
				uint16_t z = x | y;
				printf("%d", z);
			
				// printf("%d", numbers[i]);
				if(i != amount-1)
					printf(",");
			}
			printf("\n");
			if (format == 0 || format == 2) 
				writeToType0(outputStream, type, amount, numbers);
			else if (format == 1 || format == 3) 
				type0ToType1(amountArray, numbers, outputStream, amount);
		}
		
		// Type 1
		else if (type == 1) {
			uint8_t amount[3];
			// getting the value of amount
			fread(amount, 1, 3, sourcefile);
			uint8_t amountInDecimal = asciiToDecimal(amount);
			uint8_t num = amountInDecimal;
			
			// print the amount 
			printf("Amount: ");
			for (int i = 0; i < 3; i++) {
				printf("%c", amount[i]);
			}
			printf("\t");
			// get the position of 1 or 0
			// to calculate the offset for fseek
			int count = 0;
			uint8_t charValue;
			long currOffset = ftell(sourcefile);
			
			// break until the end of unit is not reached
			while (1) {
				charValue = fgetc(sourcefile);
				// if amountInDecimal is below 2
				if (amountInDecimal < 2) {
					if (charValue == 0 || charValue == 1) {
						break;
					}
				}
				// if it reaches the end of the file
				// increments count and breaks
				if (ftell(sourcefile) == fileSize) {
						count++;
						break;
				}
				// if comma, subtract 1 from amountInDecimal
				if (charValue == ',') {
					// printf("Found , ");
					amountInDecimal -= 1;
				}
				count++;
			}
			
			// array to store the numbers
			uint8_t numbers[count];
			// changing the position of the pointer
			fseek(sourcefile, currOffset, SEEK_SET);
			// stores the numbers in the array
			fread(numbers, 1, count, sourcefile);
			
			// prints the numbers
			for (int i = 0; i < count; i++) {
				printf("%c", numbers[i]);
			}
			printf("\n");
			if (format == 0 || format == 1)
				writeToType1(outputStream, type, amount, count, numbers);
			else if (format == 2 || format == 3)
				type1ToType0(outputStream, num, numbers, count);
		}
		else {
			printf("Error.\n");
			return -1;
		}
		temp = ftell(sourcefile);
	}
	return 0;
}

// helper function to conver ascii to decimal numbers
uint8_t asciiToDecimal(uint8_t amount[]) {
	uint8_t temp = atoi(amount);;
	return temp;
}

// helper function to convert decimal to ascii characters
void decimalToAscii(uint8_t amount, uint8_t *array) {
	int remainder;
	int i = 2;
	// population the array with corresponding ascii characters
	while(i >= 0) {
		remainder = amount % 10;
		amount = amount / 10;
		array[i] = remainder + '0';
		i -= 1;
	}
}

// writes the type 0 unit to  an output file
void writeToType0(FILE* outputStream, uint8_t type, uint8_t amount, uint16_t* array) {
	int unitLength = amount * 2 + 1 + 1;
	
	// array to store all the data of the unit
	uint8_t unitData[unitLength];
	// copying the first byte into unitData
	memcpy(unitData, &type, 1);
	// copying the second byte (amount) to unitData
	memcpy(unitData + 1, &amount, 1);
	// copyting the numbers array to unitData
	memcpy(unitData + 2, array, amount * 2);
	
	// printf("%c\n", unitData[unitLength-1]);
	
	
	// writing to a *outStream
	fwrite(unitData, 1, unitLength, outputStream);
}

// writes the type 1 unit to an output file 
void writeToType1(FILE* outputStream, uint8_t type, uint8_t* amount, int count, uint8_t* numbers) {
	int unitLength = count + 1+ 3;
	uint8_t unitData[unitLength];
	memcpy(unitData, &type, 1);
	memcpy(unitData + 1, amount, 3);
	memcpy(unitData + 4, numbers, count);
	fwrite(unitData, 1, unitLength, outputStream);
}

// translates to type1 format
void type0ToType1(uint8_t* amountArray, uint16_t* numbers, FILE* outputStream, int amount) {
	// change the first byte
	// TODO: Assume for now these values are passed
	uint8_t type = 1;
	fwrite(&type, 1, 1, outputStream);
	
	fwrite(amountArray, 1, 3, outputStream); 
	int count = 0;
	char comma = ',';
	for (int i = 0; i < amount; i++) {
		char buffer[5];
		int num = numbers[i];
		count = snprintf(buffer, 5, "%d", num);
		// depending on the value of count add the char from buffer into file
		fwrite(buffer, 1, count, outputStream);
		if (i != amount-1) 
			fwrite(&comma, 1, 1, outputStream);
	}
}

void type1ToType0(FILE* outputStream, uint8_t amount, uint8_t* numbers, int count) {
	// changing the first byte
	uint8_t type = 0;
	fwrite(&type, 1, 1, outputStream);
	
	// writing the decimal amount to the file
	// Convert to a byte/ maybe more than a byte
	fwrite(&amount, 1, 1, outputStream);
	int start = 0;
	int end = 0;
	int x = 0;
	// writing the decimal values to the file
	for (int i = 0; i < count; i++ ) {
		// if char is ','
		if ((numbers[i] == ',') || (i == count - 1)) {
			end = i;
			if (i == count - 1)
				end++;
			// creating a sliced array			
			// contains the chars to convert to 2 byte numbers
			uint8_t slicedNumbers[end-start];
			for (int j = start; j < end; j++) {
				slicedNumbers[x] = numbers[j];
				x++;
			}
			x = 0;
			// converting char to decimal value
			uint16_t num = atoi(slicedNumbers);
			// finally wrting the 2 bytes into the file
			fwrite(&num, 1, 2, outputStream);
			start = end + 1;
		}
	} 
}