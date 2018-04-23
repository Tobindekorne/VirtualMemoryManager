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
#define FRAME_SIZE 256	//The maximum size of a frame in physical memory
#define NUM_FRAMES 256	//The maximum number of frames this system supports
#define TLB_SIZE 16		//The maximum number of TLB entries

extern int errno;

//Function prototypes declared here
void readPageFromBackingStore(char page[], FILE * FD);
void initializeTLB(int arr[][2], size_t size);
void printTable(int arr[], size_t size);
void initialize(int arr[], size_t size);
void bin2(char * arr);
void bin(unsigned n);
int translateAddress(int virtualAddress);
int getPageNumber(int logicalAdd);
int getPageOffset(int logicalAdd);
int readBackingStore();

//
//Start of the main program
//
int main(int argc, char const *argv[]) {
	//Start be ensuring correct input from the user
	if (argc != 2) {
		printf("Usage: ./vmmgr <file>\n");
		return -1;
	}

	char *addFile = NULL;		    //name of the file containing logical addresses
	char *backStoreFile = NULL;	   //name of the backing store file
	char logicalAdd[8];		       //will hold a logical address from file
	char page[256];                //used to hold a 256 byte page loaded from disk (BACKING_STORE.bin file)
	int logicalAddress;            //used to hold a logical address from the address file
    int index = 0;                  //to keep track of the current index into memory
	FILE *addFD = NULL;			//file descriptor for logical address file
	FILE *backFD = NULL;		//file descriptor for the backing store file

	//initialize arrays for ram, virtual memory, and the TLB
	int ram[FRAME_SIZE * NUM_FRAMES]; 	//set up ram to have 256 frames with 256 bytes each
    //set page table size at 256 bytes - one for each frame in RAM
	int pageTable[PAGE_LENGTH];
    //set the TLB to have 16 entries and 2 slots for each entry
    // one slot for the page number and one for the frame number
	int tlb[TLB_SIZE][2];

	// initialize each table with blank memory (-1 in each slot)
	initialize(ram, FRAME_SIZE * NUM_FRAMES);
	initializeTLB(tlb, TLB_SIZE);
	initialize(pageTable, PAGE_LENGTH);

	//printTable(vm, sizeof(vm)/sizeof(int));

	//Read files
	//Open address file for reading in logical addresses
	addFD = fopen(argv[1], "r");
	if (addFD == NULL) {
		fprintf(stderr, "Error reading logical address file: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//open the backing store file to read in data from disk
	backFD = fopen("BACKING_STORE.bin", "r");
	if (backFD == NULL) {
		fprintf(stderr, "Error reading BACKING_STORE.bin: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//This while loop reads in each line of logical addresses and processes them
	//using the functions written for dealing with a paging system
	while (fgets(logicalAdd, 7, addFD) != NULL) {
		logicalAddress = atoi(logicalAdd);
		pageTable[index] = logicalAddress;
		index++;
	}

    readPageFromBackingStore(page, backFD);

	//Translate addresses

	return EXIT_SUCCESS;
}

/*
 * Function:  readPageFromBackingStore
 * --------------------
 * Reads a page from secondary memory (BACKING_STORE.bin) into a page array
 *
 *  page: the char array for a page
 *  FD: the file descriptor for seconday memory
 *
 */
void readPageFromBackingStore(char page[], FILE * FD) {
    fread(page, PAGE_LENGTH, 1, FD);
}

/*
 * Function:  initialize
 * --------------------
 * initialize an integer array with every
 * entry set to -1 to denote uninitialized memory
 *
 *  arr: the int array to be initialized
 *  size: the size of the array
 *
 */
void initialize(int arr[], size_t size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		arr[i] = -1;
	}
}

/*
 * Function:  initializeTLB
 * --------------------
 * initialize the TLB array with every
 * entry set to -1 to denote uninitialized memory
 *
 *  arr: the 2 dimensional int array for the TLB to be initialized
 *  size: the size of the array
 *
 */
void initializeTLB(int arr[][2], size_t size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		arr[i][0] = -1;
		arr[i][1] = -1;
	}
}

/*
 * Function:  getPageNumber
 * --------------------
 * returns the page number from a logical address
 *
 *  logicalAdd: the logical address to get the offset from as an int
 *
 *  returns: the 8 bit representation of the page number
 */
int getPageNumber(int logicalAdd) {
	return logicalAdd >> 8; //shift the bits right 8 times to place the page number in the correct location
}


/*
 * Function:  outputData
 * --------------------
 * outputs the logical address being translated, the physical address it
 * maps to, the signed byte for that value, the page number,
 * and the page offset.
 *
 *  logicalAdd: the logical address as an int
 *  physicalAdd: the mapped physical address as an int
 *  signedByte: the signed byte value there
 *  page: the page that goes with the logical address
 *  offset: the offset into the frame for the physical address
 *
 *  returns: the 8 bit representation of the page number
 */
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

/*
 * Function: getPageOffset
 * --------------------
 * computes the page offset of a virtual address:
 *
 *  logicalAdd: the logical address to get the offset from as an int
 *
 *  returns: the 8 bit representation of the page offset
 */
int getPageOffset(int logicalAdd) {
	return logicalAdd & 0xFF; //bit mask the logical address with hex FF to get the last bits of data
}

/*
 * Function:  translateAddress
 * --------------------
 * translates a virtual (logical) address to a physical address using:
 *          [page # as first 8 bits][page offset as second 8 bits]
 *  virtualAddress: the virtual (logical) address to be translated
 *
 *  returns: the frame number of the address
 *           returns -1 if there is a page fault
 */
int translateAddress(int virtualAddress) {
	unsigned char page, offset; //variables for holding the page number and page offset
	int frame, physicalAddress;
	offset = getPageOffset(virtualAddress);
	page = getPageNumber(virtualAddress);
	frame = page * FRAME_SIZE;
	physicalAddress = frame + offset;
	outputData(virtualAddress, physicalAddress, (char) (virtualAddress >> 16), page, offset);
    return physicalAddress;
}

/*
 * Function:  printTable
 * --------------------
 * outputs all values held in a the page table
 *
 *  arr: the integer array of page table entries
 *  size: the size of the page table array
 *
 */
void printTable(int arr[], size_t size) {
	int i = 0;
	for (i; i < size; i++) {
		printf("%i %i\n", i, arr[i]); //print the position in the table and the contents at that location
	}//end for
}

/*
 * Function:  bin
 * --------------------
 * outputs all binary value of an unsigned value n
 *
 *  n: an unsigned vlaue
 */
void bin(unsigned n)
{
    unsigned i;
    for (i = 1 << 31; i > 0; i = i / 2)
        (n & i) ? printf("1"): printf("0");
	printf("\n");
}

/*
 * Function:  bin2
 * --------------------
 * outputs all binary values
 * from an array of characters as one byte per line
 *
 *  arr: the character array
 */
void bin2(char * arr) {
	int j, i;
	for (j = 0; j < 256; j++) {
		for (i = 0; i < 8; i++) {
			printf("%d", !!((arr[j] << i) & 0x80));
		}
		printf("\n");
	}
}


//DEV NOTES:
// while(!feof(backFD)) {
//This will read in a single page from secondary memory
    // fread(page, 256, 1, backFD);
    // unsigned n = atoi(page);
    // bin2(page);
// }
