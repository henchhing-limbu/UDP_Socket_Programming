#include "helper.h"

// function declarations
uint8_t asciiToDecimal(uint8_t amount[]);
void decimalToAscii(uint8_t amount, uint8_t *array);
void writeToType0(FILE* outputStream, uint8_t type, uint8_t amount, uint16_t array[]);
void writeToType1(FILE* outputStream, uint8_t type, uint8_t* amount, int count, uint8_t* numbers);
void type0ToType1(uint8_t* amountArray, uint16_t* numbers, FILE* outputStream, int amount);
void type1ToType0(FILE* outputStream, uint8_t amount, uint8_t* numbers, int count);
char convertFile(int format, FILE* sourcefile, FILE* destfile);

int main(int argc, char* argv[]) {
	int sockfd;												// socket description
	struct sockaddr_in servAddr;							// server address
	struct sockaddr_in clntAddr;							// client address
	unsigned int clntAddrLen;								// size of client address struct
	unsigned short port;									// port number
	long fileSize;											// size of received data
	unsigned long outputFileNameSize;						// output file name size
	int format;												// format number
	char buffer[MAX_LINE];									// buffer for received data
	unsigned int seed;										// random seed
	float lossProb;											// loss probability
	struct sigaction myAction;								// for setting signal handler
	char ack = 1;											// initialized positive ack		
	char clientDone;										// client done message
	char errorMessage;										// initializing error message
	tries = 0;
	// checking for number of arguments
	if (argc != 4) {
		printf("SERVER: Not enough argumnets. 2 argumnets are needed.\n");
		exit(EXIT_SUCCESS);
	}
	port = atoi(argv[1]);
	printf("Port number: %d\n", port);
	lossProb = atof(argv[2]);
	seed = atoi(argv[3]);
	
	// setting signal handler for alarm signal
	myAction.sa_handler = CatchAlarm;
	if (sigfillset(&myAction.sa_mask) < 0)
		DieWithError("sigfillset() failed");
	myAction.sa_flags = 0;
	if (sigaction(SIGALRM, &myAction, 0) < 0)
		DieWithError("sigaction() failed for SIGALRM");
	
	while(1) {
		// creating listening socket
		if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			printf("SERVER: Error creating the socket.\n");
			exit(EXIT_FAILURE);
		}
		// constructing server address structure
		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;							// internet address family
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 			// any incoming interface
		servAddr.sin_port = htons(port);						// port
		
		// binding socket to the local address
		if (bind(sockfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
			printf("SERVER: Error binding the socket.\n");
			exit(EXIT_FAILURE);
		}
		// set the size of the in-out parameter
		clntAddrLen = sizeof(clntAddr);
		
		// receiving file size from the client
		if ((recvfrom(sockfd, recvPacketBuf, sizeof(fileSize) + 1, 0, (struct sockaddr*) &clntAddr, &clntAddrLen)) < 0) {
			printf("SERVER: Error receiving the file size from the client.\n");
			exit(EXIT_FAILURE);
		}
		printf("SERVER: Received file size.\n");
		// extracting data fromt the received packet
		extractPacket(recvPacketBuf, &seqNum, &fileSize, sizeof(fileSize));
		// file to write to 
		FILE* fp = fopen("Received","wb");
		
		long bytesToReceive = fileSize;
		while (bytesToReceive > 0) {
			if (bytesToReceive > MAX_LINE) {
				sendAndWaitServ(sockfd, lossProb, seed, 
				&clntAddr, clntAddrLen, &ack, sizeof(ack),
				buffer, MAX_LINE);
			}
			else {
				sendAndWaitServ(sockfd, lossProb, seed, 
				&clntAddr, clntAddrLen, &ack, sizeof(ack),
				buffer, bytesToReceive);
			}
			fwrite(buffer, 1, bytesReceived - 1, fp);
			bytesToReceive -= (bytesReceived - 1);
		}
		fclose(fp);		
		// sending acknowledgment for the data received
		// receiving format number from the client
		printf("SERVER: Sent acknowledgment for the data received.\n");
		sendAndWaitServ(sockfd, lossProb, seed, &clntAddr, clntAddrLen, &ack, sizeof(ack),
		&format, sizeof(format));
		printf("SERVER: Received format number.\n");
		
		// sending acknowledgment for the format number
		// receiving output file size from the client
		printf("SERVER: Sent acknowledgment for the format number.\n");
		sendAndWaitServ(sockfd, lossProb, seed, &clntAddr, clntAddrLen, &ack, sizeof(ack),
		&outputFileNameSize, sizeof(outputFileNameSize));
		printf("SERVER: Received output file size.\n");
		
		// sending acknowledgment for the output file name size
		// receiving output file name from the client
		char outputFileName[outputFileNameSize];
		printf("SERVER: Sent acknowledgment for the output file name size.\n");
		sendAndWaitServ(sockfd, lossProb, seed, &clntAddr, clntAddrLen, &ack, sizeof(ack),
		outputFileName, outputFileNameSize);
		printf("SERVER: Received output file name .\n");
		
		// sending acknowledgment for the output file name 
		// receiving client done message from the client
		printf("SERVER: Sent acknowledgment for the output file name.\n");
		sendAndWaitServ(sockfd, lossProb, seed, &clntAddr, clntAddrLen, &ack, sizeof(ack),
		&clientDone, sizeof(clientDone));
		printf("SERVER: Received client done message from the server.");
		
		// opening the file where we want to write the translated data
		FILE* destFile = fopen(outputFileName, "wb");
		// opening the source file for the translation
		fp = fopen("Received", "rb");
		
		// converting the file 
		// errorMessage equals the value returned by the function
		printf("SERVER: Going to call converFile().\n");
		errorMessage = convertFile(format, fp, destFile);
		
		fclose(destFile);
		fclose(fp);
		
		if (errorMessage < 0)
			remove(outputFileName);
		remove("Received");

		// sending errorMessage to the client
		// seqNum = (seqNum + 1) % 2;
		makePacket(sendPacketBuf, &seqNum, &errorMessage, sizeof(errorMessage));
		for (int i = 0; i < MAX_TRIES; i++) {
			lossy_sendto(lossProb, seed, sockfd, sendPacketBuf, sizeof(errorMessage) + 1, 
			(struct sockaddr*) &clntAddr, clntAddrLen);
		}
		printf("SERVER: closing the socket.\n");
		close(sockfd);
		
	}
	return 0;
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
	uint8_t type = 1;
	fwrite(&type, 1, 1, outputStream);
	
	fwrite(amountArray, 1, 3, outputStream); 
	int count = 0;
	char comma = ',';
	for (int i = 0; i < amount; i++) {
		char buffer[6];
		uint16_t num = numbers[i];
		num = (num >> 8) | (num << 8);
		count = snprintf(buffer, 6, "%d", num);
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
			// printf("String value = %s\n", slicedNumbers);
			// converting char to decimal value
			uint16_t num = atoi(slicedNumbers);
			num = (num >> 8) | (num << 8);
			// printf("Number value = %d\n", num);
			// finally wrting the 2 bytes into the file
			fwrite(&num, 1, 2, outputStream);
			memset(slicedNumbers, 0, end-start);
			start = end + 1;
		}
	} 
}