/**************************************************************
* Class:  CSC-415-03 
* Name: Valeria Vallejo, Chung Hei Fong, Erik Chacon
* Student ID: 920594217  917595970  920768287
* Project: File System Project
*
* File: Disk.h
*
* Description: Interface of basic Disk function prototypes
*
**************************************************************/  


#ifndef _DISK_H
#define _DISK_H

#include <stdio.h>
#include <stdlib.h>
#include "mfs.h"
#include "freeSpaceAllocation.h"
#include <fsLow.h>

// global variables
extern char *filename;
extern uint64_t volumeSize;
extern uint64_t blockSize;
SuperBlock *VCB;

// will init our file systemm
void fs_init();

#endif
