/**************************************************************
* Class:  CSC-415-03 
* Name: Valeria Vallejo, Chung Hei Fong, Erik Chacon
* Student ID: 920594217  917595970  920768287
* Project: File System Project
*
* File: freeSpaceAllocation.c
*
* Description: Free Space Management functions
*
**************************************************************/
#include "freeSpaceAllocation.h"
#include <fsLow.h>

int dataBitMap[NUM_DATA_BLOCKS];
int dataDirEnt[NUM_DIR_ENTR];
dataBlock *HEAD;

// init data block bit map
void initDataBitMap()
{
    // for every data block
    for (int i = 0; i < NUM_DATA_BLOCKS; i++)
    {
        // set to 0 (it is free)
        dataBitMap[i] = 0;
    }
}

// init directory entry bit map
void initDirBitMap(int dataBitMap[])
{
    // for number of directory entries in a directory
    for (int i = 0; i < NUM_DIR_ENTR; i++)
    {
        dataBitMap[i] = 0;
    }
}


// returns starting index in data block bit vector map
int findFreeDataBlocks(int numOfBlocks)
{
    // keep track of how many blocks in a row
    int counter = 0;

    // loop through bit vector map
    for (int i = 0; i < NUM_DATA_BLOCKS; i++)
    {
        // if found free spot
        if (dataBitMap[i] == 0)
        {
            // increase counter by one
            counter++;
        }
        // if not free
        else
        {
            // reset counter
            counter = 0;
        }

        // if only need one block
        if (counter == numOfBlocks && numOfBlocks == 1) {
            // return index of block
            return i;
        }
        // if found num of free blocks (in a row)
        else if (counter == numOfBlocks && numOfBlocks > 1)
        {

            // return starting index
            return i - counter;
        }
    }
    // if can't find num of blocks
    printf("No available space for this size of file.\n");
    return -1;
}

// switch 0 to 1 and 1 to 0
// show that unused blocks are now used
void flipBits(int bitMap[], int startingIndex, int numToFlip)
{
    // flipBits(dataBitMap, 0, 1);
    // for number of times we have to flip
    for (int i = 0; i < numToFlip; i++)
    {
        if( bitMap[startingIndex + i] == 1){
            bitMap[startingIndex + i] = 0;
        }
        else{
            bitMap[startingIndex + i] = 1;
        }
    }
}

//function for data block allocation
dataBlock *allocDataBlocks()
{
    // index tracker
    int i = 0;
    // malloc space for first data block
    HEAD = (dataBlock *)malloc(sizeof(dataBlock));
    // set block id
    HEAD->blockID = i;
    // set blocksize
    HEAD->spaceRemain = blockSize;
    // malloc space for text file
    HEAD->textInFile = (char *)malloc(512);
    // set text offset to 0
    HEAD->textOffset = 0;
    HEAD->fileName = (char *)malloc(256);
    HEAD->next = NULL;
    // increase index by one
    i++;
    // create another datablock that also points to the first block
    dataBlock *prev = HEAD;
    // while we have more blocks to create
    while (i < NUM_DATA_BLOCKS)
    {
        // malloc a third data block ptr
        dataBlock *db = (dataBlock *)malloc(sizeof(dataBlock) + blockSize);
        // set the previous data block's next block to db (a new block)
        prev->next = db;
        // set new block id
        db->blockID = i;
        // set space remaining in block
        db->spaceRemain = blockSize;
        // set text offset
        db->textOffset = 0;
        // malloc space for text
        db->textInFile = (char *)malloc(512);
        db->permission = 10;
        db->fileName = (char *)malloc(256);
        db->next = NULL;
        // set previous data block ptr to new data block (the one just created)
        prev = db;
        // increase index
        i++;
    }
    // return the first data block
    return HEAD;
}
int isNew(int blockID)
{
    if (dataBitMap[blockID] == 0)
    {
        // is new file
        return 0;
    }
    // not new file
    return -1;
}

// find next free index in a directory's DE array
int findNextFreeIndex(int dataBitMap[])
{
    for (int i = 0; i < NUM_DIR_ENTR; i++)
    {
        if (dataBitMap[i] == 0)
        {
            return i; // 0 means free, so return this one
        }
    }
}
