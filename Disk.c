/**************************************************************
* Class:  CSC-415-03 
* Name: Valeria Vallejo, Chung Hei Fong, Erik Chacon
* Student ID: 920594217  917595970  920768287
* Project: File System Project
*
* File: Disk.c
*
* Description: Interface of basic Disk functions. Main hub of 
* starting our partition
*
**************************************************************/  


#include "Disk.h"
// will hold filename
char *filename;
// will hold total volume size
uint64_t volumeSize;
// will hold block size
uint64_t blockSize;
// ptr to volume control block
SuperBlock *VCB;

// file system init
void fs_init()
{
    // set up disk
    startPartitionSystem(filename, &volumeSize, &blockSize);
    // init Super Block
    // malloc 512 bytes for first block
    VCB = malloc(sizeof(SuperBlock) + (512 - sizeof(SuperBlock)) + 1);
    // set magic numbers
    VCB->magicNum = 0;
    // set total number of data blocks
    VCB->numDataBlocks = NUM_DATA_BLOCKS;
    // set block size
    VCB->blockSize = 512;
    // find first free block
    VCB->freeBlockIndex = findFreeDataBlocks(1);
    // set root directory index
    VCB->rootDirIndex = 0;
    // write volume control block to disk
    LBAwrite(VCB, 1, 0);
    // init the data block bit map
    initDataBitMap();
    // flip first bit
    flipBits(dataBitMap, 0, 1);
    // write bitmap into disk
    //  -init directory entry bit vector map
    // TODO put bit map into data blocks
    initDirBitMap(dataDirEnt);

    allocDataBlocks();
    // init root dir
    fs_mkdir("/");
    LBAwrite(ROOTPTR, 1, 1);
}
