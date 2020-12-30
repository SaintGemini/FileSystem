/**************************************************************
* Class:  CSC-415-03 
* Name: Valeria Vallejo, Chung Hei Fong, Erik Chacon
* Student ID: 920594217  917595970  920768287
* Project: File System Project
*
* File: b_io.c
*
* Description: Interface of basic I/O functions
*
**************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include <string.h>

#include "freeSpaceAllocation.h"
#include "mfs.h"

#define BLOCKSIZE 512

// always points to head
//dataBlock *headPtr;

// temp ptr for seek manipulation
dataBlock *globalPtr;

int isInit = -1;
int reset = 0;

// -------------------------------------------------------------------------------------------
// for opening and creating files
int b_open(char *filename, int flags)
{
    int fd;

    for (dataBlock *temp = HEAD; temp->next != NULL; temp = temp->next)
    {
        if (strcmp(temp->fileName, filename) == 0)
        {
            fd = temp->blockID;

            // if write only
            if ((flags & O_WRONLY) == O_WRONLY)
            {
                temp->permission = 1;
            }

            // if truncate
            if ((flags & O_TRUNC) == O_TRUNC)
            {
                temp->permission = 3;
            }
            return fd;
        }
    }
    // if create
    if ((flags & O_CREAT) == O_CREAT)
    {

        fd = findFreeDataBlocks(1);

        for (dataBlock *temp = HEAD; temp->next != NULL; temp = temp->next)
        {
            // if found
            if (temp->blockID == fd)
            {
                // change name
                temp->fileName = filename;
                temp->textOffset = 0;
                // // if write only
                if ((flags & O_WRONLY) == O_WRONLY)
                {
                    temp->permission = 1;
                }
            }
        }
        initDirectoryEntry(filename, FT_REGFILE);
        // return block ID
        return fd;
    }
    printf("File doesn't exist.\n");
    return 0;
}

// -------------------------------------------------------------------------------------------

// blockID - where we start to read
// buffer - copy what we read to buffer
// count - how many bytes to read

int b_read(int blockID, char *buffer, int count)
{
    // go through link list to find correct block
    for (dataBlock *temp = HEAD; temp->next != NULL; temp = temp->next)
    {
        // if found right block
        if (temp->blockID == blockID)
        {
            // let global pointer have access to block
            globalPtr = temp;
        }
    }
    while (globalPtr->textOffset == blockSize)
    {
        int newBlockID = findNextBlock(globalPtr);

        while (globalPtr->blockID != newBlockID)
        {
            // helper function
            // find next block associated with file
            globalPtr = globalPtr->next;
        }
    }

    LBAread(globalPtr->textInFile, 1, globalPtr->blockID);

    int remain = strlen(globalPtr->textInFile) - globalPtr->textOffset;

    // if we have to read more than one block
    if (remain < count)
    {
        // remaining bytes to read
        // 112
        int remainingBytesinPrev = BLOCKSIZE - globalPtr->textOffset;

        // copy from textInFile to buffer
        memcpy(buffer, globalPtr->textInFile + globalPtr->textOffset, remainingBytesinPrev);
        if (globalPtr->blockID == findEndBlock(globalPtr))
        {
            return strlen(globalPtr->textInFile) - globalPtr->textOffset;
        }
        globalPtr->textOffset = blockSize;

        // recalculate remaining bytes to read
        int remainingBytes = count - remainingBytesinPrev;
        // move to next block
        int newBlockID = findNextBlock(globalPtr);
        while (globalPtr->blockID != newBlockID)
        {
            // helper function
            // find next block associated with file
            globalPtr = globalPtr->next;
        }
        globalPtr->textOffset = 0;
        LBAread(globalPtr->textInFile, 1, globalPtr->blockID);
        // copy from textInFile to buffer
        memcpy(buffer + remainingBytesinPrev, globalPtr->textInFile + globalPtr->textOffset, remainingBytes);
        globalPtr->textOffset = remainingBytes;

        // return how many bytes were read
        return count;
    }
    else
    {
        // copy from textInFile to buffer
        memcpy(buffer, globalPtr->textInFile + globalPtr->textOffset, count);
        globalPtr->textOffset = globalPtr->textOffset + count;

        // return how many bytes were read
        return count;
    }
    // error message if nothing was returned
    printf("Nothing was returned from b_read. Something went wrong.\n");
    return -1;
}

// -------------------------------------------------------------------------------------------

// count is how many bytes to write
// buffer is where we copy from
// blockID is where we want to copy to

// change data block bit map to a '1' after write
int b_write(int blockID, char *buffer, int count)
{
    // loop through blocks to find right one
    for (dataBlock *temp = HEAD; temp->next != NULL; temp = temp->next)
    {
        // if found right one
        if (temp->blockID == blockID)
        {
            // get access
            globalPtr = temp;
            break;
        }
    }
    // check to see if we can write
    if (globalPtr->permission == 1 || globalPtr->permission == 3)
    {
        // if we have to over write
        if (globalPtr->permission == 3)
        {   
            // get starting block
            dataBlock *temp = globalPtr;
            
            // compare size to see if it's over one block
            int numBlocksToFree = findEndBlock(temp);
            // free all text
            for (int i = 0; i < numBlocksToFree; i++)
            {
                memcpy(temp->textInFile, "", 1);

                temp->size = 0;
                temp->spaceRemain = blockSize;
                temp->textOffset = 0;

                if (numBlocksToFree > 1)
                {
                    temp = temp->next;
                }
            }
        }
        char *compFilename = globalPtr->fileName;
        // if block is full, go to next block that isn't
        if (globalPtr->size == blockSize)
        {
            int newBlockID = findEndBlock(globalPtr);
            while (globalPtr->blockID != newBlockID)
            {
                // helper function
                // find next block associated with file
                globalPtr = globalPtr->next;
            }
        }
        // size of what we have plus what they want
        int bufferSize = globalPtr->size + count;

        // 512 - bytes already in file
        globalPtr->spaceRemain = BLOCKSIZE - globalPtr->size;

        // if we go over one block
        if (bufferSize > BLOCKSIZE)
        {

            memcpy(globalPtr->textInFile + globalPtr->size, buffer, globalPtr->spaceRemain);




/*************** COMMMENT ********************/









            LBAwrite(globalPtr->textInFile, 1, globalPtr->blockID);
            flipBits(dataBitMap, globalPtr->blockID, 1);
            int remainingBuffer = count - globalPtr->spaceRemain;
            globalPtr->size = blockSize;
            globalPtr->spaceRemain = 0;
            // go to next block
            int newBlockID = findFreeDataBlocks(1);

            while (globalPtr->blockID != newBlockID)
            {
                // helper function
                // find next block associated with file
                globalPtr = globalPtr->next;
            }
            globalPtr->permission = 1;
            globalPtr->fileName = compFilename;
            memcpy(globalPtr->textInFile, buffer + remainingBuffer, count - remainingBuffer);

            globalPtr->size = count - remainingBuffer;
            globalPtr->spaceRemain = blockSize - globalPtr->size;
            globalPtr->fileName = compFilename;
            // return number of bytes written
            return count;
        }
        // if count < one block
        else
        {

            memcpy(globalPtr->textInFile + globalPtr->size, buffer, count);
            // update size
            globalPtr->size = globalPtr->size + count;
            globalPtr->spaceRemain = globalPtr->spaceRemain - count;
            if (count < 200)
            {
                LBAwrite(globalPtr->textInFile, 1, globalPtr->blockID);
            }
        } // end one block logic

        return count;
    }
    // we can't write
    else
    {
        return 0;
    }
}

// -------------------------------------------------------------------------------------------

// for repositioning text offset in file
int b_seek(int blockID, off_t offset, int whence)
{
    // search through linked list of data blocks
    for (dataBlock *temp = HEAD; temp->next != NULL; temp = temp->next)
    {
        // if found starting block
        if (temp->blockID == blockID)
        {
            // gain access
            globalPtr = temp;
            break;
        }
    }

    // check if offset is bigger than size of textInFile
    if (offset > globalPtr->size)
    {
        printf("Seek out of bounds.\n");
        return -1;
    }

    // what block to start seek at (will be int)
    int blockOffset = (globalPtr->textOffset + offset) / 512;

    // if SEEK_SET (10) set offset from beginning of file
    if (whence == 0)
    {
        // if file is more than one data block
        if (blockOffset > 0)
        {
            // find num of block to cycle through
            int numOfBlocks = offset / 512;

            // while not at right block
            while (numOfBlocks > 0)
            {
                // go to next block
                globalPtr->next;

                // decrease num of blocks to move through
                numOfBlocks--;
            }
            // remaining offset after cycling through # of 512 blocks
            int remainOffset = offset - (512 * blockOffset);
            // move ptr to remainOffset
            globalPtr->textOffset = remainOffset;
            // return offset relative from beginning
            return globalPtr->textOffset;
        }
        else
        {
            // if seek is in first block we find

            // move ptr to offset
            globalPtr->textOffset = offset;
            // return position relative to beginning
            return offset;
        }
    } // end if SEEK_SET

    // if SEEK_CUR, move offset from current text offset position
    if (whence == 1)
    {
        // check if offset is bigger than size of textInFile
        if (offset + globalPtr->textOffset > globalPtr->size)
        {
            printf("Seek out of bounds.\n");
            return -1;
        }

        // if file is more than one data block
        if (blockOffset > 0)
        {
            // find num of block to cycle through
            int numOfBlocks = (offset - globalPtr->textOffset) / 512;

            // while not at right block
            while (numOfBlocks > 0)
            {
                // go to next block
                globalPtr->next;

                // decrease num of blocks to move through
                numOfBlocks--;
            }
            // remaining offset after cycling through # of 512 blocks
            int remainOffset = offset - (512 * blockOffset) + globalPtr->textOffset;
            // move ptr to remainOffset
            globalPtr->textOffset = remainOffset;
            // return offset relative from beginning
            return globalPtr->textOffset;
        } // end if bigger than one block
        else
        {
            // if seek is in first block we find

            // move ptr to offset
            globalPtr->textOffset = offset + globalPtr->textOffset;
            // return position relative to beginning
            return globalPtr->textOffset;
        } // end if in one block
    }     // end if SEEK_CUR

    // if  SEEK_END
    if (whence == 2)
    {
        // set text offset to end of file + however many bytes they want
        globalPtr->textOffset = globalPtr->size + offset;
        // if over one block, give them the space and do nothing

        // if file is more than one data block
        if (blockOffset > 0)
        {
            // find num of block to cycle through
            int numOfBlocks = (globalPtr->textOffset) / 512;

            // while not at right block
            while (numOfBlocks > 0)
            {
                char *temp = globalPtr->fileName;
                // go to next block
                globalPtr->next;
                // set filename
                globalPtr->fileName = temp;

                // decrease num of blocks to move through
                numOfBlocks--;
            }

            // remaining offset after cycling through # of 512 blocks
            int remainOffset = offset - (512 * blockOffset);
            // move ptr to remainOffset
            globalPtr->textOffset = globalPtr->size - remainOffset;
            // return offset relative from beginning
            return globalPtr->textOffset;
        } // end if bigger than one block
        else
        {
            // if seek is in first block we find

            // move ptr to offset
            globalPtr->textOffset = globalPtr->size + offset;
            // return position relative to beginning
            return globalPtr->textOffset;
        } // end if in one block
    }

    // if nothing was returned at this point, something went wrong
    printf("Something went wrong, nothing returned from b_seek!\n");
    return -1;
}

// -------------------------------------------------------------------------------------------

void b_close(int fd)
{
    // reset globalPtr to head
    globalPtr = HEAD;
}





/************************************ COMMENT ***********************************/





int findEndBlock(dataBlock *check)
{

    dataBlock *temp = check;
    int blockID = temp->blockID;
    while (temp->next != NULL)
    {
        if (strcmp(temp->fileName, check->fileName) == 0)
        {
            blockID = temp->blockID;
        }
        temp = temp->next;
    }
    return blockID;
}

int findNextBlock(dataBlock *check)
{
    dataBlock *temp = check;
    int blockID = temp->blockID;
    while (temp->next != NULL)
    {
        temp = temp->next;
        if (strcmp(temp->fileName, check->fileName) == 0)
        {
            return temp->blockID;
        }
    }
    return blockID;
}