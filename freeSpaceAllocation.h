/**************************************************************
* Class:  CSC-415-03 
* Name: Valeria Vallejo, Chung Hei Fong, Erik Chacon
* Student ID: 920594217  917595970  920768287
* Project: File System Project
*
* File: freeSpaceAllocation.h
*
* Description: header file for free space allocation
*
**************************************************************/
#ifndef FREESPACEALLOCATION_H
#define FREESPACEALLOCATION_H
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>

#include "mfs.h"

// global variables that will be
// initialized outside of h file
extern char *filename;
extern uint64_t volumeSize;
extern uint64_t blockSize;
// define global variables
#define NUM_DATA_BLOCKS 19531
#define NUM_DIR_ENTR 50
// struct for volume control block
typedef struct SuperBlock
{
    // holds magic numbers
    int magicNum;
    // number of data blocks in file system
    int numDataBlocks;
    // size of blocks
    int blockSize;
    // where is next available free data block
    int freeBlockIndex;
    // where is root directory (starting block number)
    int rootDirIndex;

} SuperBlock;
// global variable for volume control block
extern SuperBlock *VCB;

// struct for data block
typedef struct dataBlock
{
    // holds file name
    char *fileName;
    // starting block index
    int blockID;
    // maybe delete, maybe not
    int size;
    // space remaining (in bytes) of text in file
    int spaceRemain;
    // current position in file
    int textOffset;
    // text in the file
    char *textInFile;
    // permissions
    int permission;
    // ptr to next block
    struct dataBlock *next;
} dataBlock;

// to init data block bitmap
void initDataBitMap();
void initDirBitMap(int dataBitMap[]);
int findFreeDataBlocks(int numOfBlocks);
dataBlock *allocDataBlocks();
int isNew(int blockID);
int findNextFreeIndex(int dataBitMap[]);
void flipBits(int bitMap[], int startingIndex, int numToFlip);


//array of directory entries
struct DirectoryEntry *dirEntrArr[NUM_DIR_ENTR]; //first 2 elements are . and .. (self and parent)

// data block bitmap
extern int dataBitMap[NUM_DATA_BLOCKS];

// directory entry bitmap
extern int dataDirEnt[NUM_DIR_ENTR];

// head ptr to first data block
extern dataBlock *HEAD;

#endif
