//Author: Tobin deKorne
//Date: 4/16/18
//vmmgr emmulates a virtual memory manager in an OS
//This is done using paging with demand paging from a backing store
#include <stdio.h>
#include <stdlib.h> //for atoi()
#include <string.h> //for memcpy() to copy strings
#include <errno.h>	//for handling runtime errors

//Define some constant values having to do with system properties
#define PAGE_LENGTH 256 //The maximum length of each page (256 bytes)
#define VM_SIZE 256		//The maximum size of virtual memory
#define FRAME_SIZE 256	//The maximum size of a frame in physical memory
#define NUM_FRAMES 256	//The maximum number of frames this system supports
#define TLB_SIZE 16		//The maximum number of TLB entries

//Function prototypes declared here
void printTable(int arr[], size_t size);
int getPageNumber(int logicalAdd);
int getPageOffset(int logicalAdd);
int readBackingStore();
void translateAddress(int arr[]);
void initialize(int arr[], size_t size);

//
//Start of the main program
//
int main(int argc, char const *argv[]) {
	//Start be ensuring correct input from the user
	if (argc != 2) {
		printf("Usage: ./vmmgr <file>\n");
		return -1;
	}

	char *addFile = NULL;			//name of the file containing logical addresses
	char *backStoreFile = NULL;	//name of the backing store file
	char logicalAdd[8];		//will hold a logical address from file
	int logicalAddress, index = 0;
	FILE *addFD = NULL;			//file descriptor for logical address file

	//initialize arrays for ram, virtual memory, and the TLB
	int ram[FRAME_SIZE * NUM_FRAMES]; 	//set up ram to have 256 frames with 256 bytes each
	int pageTable[PAGE_LENGTH];
	int vm[VM_SIZE];					//set virtual memory size at 256 bytes
	int tlb[TLB_SIZE];					//set the TLB to have 16 slots

	//Use the initialize function to initialize each table with blank memory (-1 in each slot)
	initialize(ram, FRAME_SIZE * NUM_FRAMES);
	initialize(vm, VM_SIZE);
	initialize(tlb, TLB_SIZE);
	initialize(pageTable, PAGE_LENGTH);

	//printTable(vm, sizeof(vm)/sizeof(int));

	//Read file
	addFD = fopen(argv[1], "r");
	if (addFD == NULL)
		exit(EXIT_FAILURE);

	//This while loop reads in each line of logical addresses and processes them
	//using the functions written for dealing with a paging system
	while (fgets(logicalAdd, 7, addFD) != NULL) {
		logicalAddress = atoi(logicalAdd);
		vm[index] = logicalAddress;
		index++;
		printf("%i\n", logicalAddress);
	}

	//Translate addresses
	translateAddress(vm);

	//Process

	return EXIT_SUCCESS;
}

//This function will initialize an integer array with every slot set to -1 to denote uninitialized memory
void initialize(int arr[], size_t size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		arr[i] = -1;
	}
}

//This function returns the page number from a logical address
//params: logicalAdd an integer logical address
//return: int - the 8 bit representation of the page number
int getPageNumber(int logicalAdd) {
	return logicalAdd >> 8; //shift the bits right 8 times to place the page number in the correct location
}

void outputData(int logicalAdd, int physicalAdd, char signedByte, unsigned char page, unsigned char offset) {
	char *head_foot = "---------------------------\n";
	printf("%s\n", head_foot);
	printf("Logical Address: %i\n", logicalAdd);
	printf("Physical Address: %i\n", physicalAdd);
	printf("Signed Byte: %i\n", (int) signedByte);
	printf("page: %i\n", (int) page); //show the page number translation
	printf("offset: %i\n", (int) offset);	//show the page offset
	printf("%s\n", head_foot);
}

//This function returns the page offset from a logical address
//params: logicalAdd an integer logical address
//return: int - the 8 bit representation of the page offset
int getPageOffset(int logicalAdd) {
	return logicalAdd & 0xFF; //bit mask the logical address with hex FF to get the last bits of data
}

//This is the function that translates logical addresses to physical addresses
void translateAddress(int arr[]) {
	int i = 0; //indexing value
	unsigned char page, offset; //variables for holding the page number and page offset
	int frame, physicalAddress;
	while (arr[i] != -1) { //while we still have a valid memory address
		offset = getPageOffset(arr[i]);
		page = getPageNumber(arr[i]);
		frame = page * FRAME_SIZE;
		physicalAddress = frame + offset;
		//ADD PAGE FAULT FUNCTIONALITY LATER!!!
		outputData(arr[i], physicalAddress, (char) (arr[i] >> 16), page, offset);
		i++; //increment the index
	}//end while
}

//printTable prints the contents of any table as an integer array
void printTable(int arr[], size_t size) {
	int i = 0;
	for (i; i < size; i++) {
		printf("%i %i\n", i, arr[i]); //print the position in the table and the contents at that location
	}//end for
}
