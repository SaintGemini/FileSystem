/**************************************************************
* Class:  CSC-415-03 
* Name: Valeria Vallejo, Chung Hei Fong, Erik Chacon
* Student ID: 920594217  917595970  920768287
* Project: File System Project
*
* File: mfs.h
*
* Description: Header file for Directory Management. Contains
* all function prototypes and structs used to create and 
* maintain the directories in our file system.
*
**************************************************************/
#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "b_io.h"

#include <dirent.h>

#define FT_REGFILE DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

// define global vars 
#define NUM_BLOCKS 20
#define NUM_EXTENTS 4
#define NUM_DIR_ENTR 50
#define MAX_DATA_BLOCKS 20


extern char* currentWorkingDirectory;

// keeps track of which data blocks belong to a file (and how many)
typedef struct FileFD
{
	int fileBlockIDs[MAX_DATA_BLOCKS];
	int totalDataBlocks;
	char filename[256];
} FileFD;

// holds metadata of files and directories
typedef struct DirectoryEntry
{
	int deID; // directory entry id (index # that pertains to place in parent's DE array)
	int blockID; //index in LBA (used as param for children for their .. DE)
	int numBlocks;	// number of blocks that belong to the DE
	char filename[256];	// char[] for name of file or directory (default size of 255 + 1 extra)
	unsigned char fileType;	//type of file: FT_REGFILE or FT_DIRECTORY (defined in mfs.h)
	int fileSize; //size of file in bytes (only includes what's been written to)
	int allDirsIndex; //for directories, index in the allDirs bitmaps
} DirectoryEntry;

typedef struct fs_diriteminfo
{
	unsigned short d_reclen; /* length of this record */
	unsigned char fileType; // directory or reg file
	char d_name[256]; /* filename max filename is 255 characters */
} fs_diriteminfo;

typedef struct fdDir
{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	unsigned short d_reclen;		 /*length of this record */
	unsigned short dirEntryPosition; /*which directory entry position, like file pos */
	uint64_t directoryStartLocation; /* Starting LBA of directory */
	struct DirectoryEntry deArray[NUM_DIR_ENTR];		 // pointer to the array (also helps with cwd)
	int dirBitMap[NUM_DIR_ENTR];
	char *dirName; // name of directory
	int deArrayIndex; // keeps track of the last DE we read in readdir
	int highestArrayIndex; //need this for readdir so we make sure to go through all DEs, even
						   // if we have some deleted ones in between
} fdDir;

// fs functions:
int fs_mkdir(const char *pathname);
int fs_rmdir(const char *pathname);
fdDir *fs_opendir(const char *name);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

char *fs_getcwd(char *buf, size_t size);
int fs_setcwd(char *buf);	   //linux chdir
int fs_isFile(char *path);	   //return 1 if file, 0 otherwise
int fs_isDir(char *path);	   //return 1 if directory, 0 otherwise
int fs_delete(char *filename); //removes a file

struct fs_stat
{
	off_t st_size;		  /* total size, in bytes */
	blksize_t st_blksize; /* blocksize for file system I/O */
	blkcnt_t st_blocks;	  /* number of 512B blocks allocated */
	time_t st_accesstime; /* time of last access */
	time_t st_modtime;	  /* time of last modification */
	time_t st_createtime; /* time of last status change */

	/* add additional attributes here for your file system */
};

// defined in DirectoryManagement.c
extern DirectoryEntry *ROOTPTR;

// helper functions from DirectoryManagement.c
int initDirectoryEntry(char *pathname, unsigned char fileType);
int fs_stat(const char *path, struct fs_stat *buf);
void initAllDirsBitMap(int bitmap[]);
int getNextAllDirs();
void initFdDir(fdDir *fd, char *dirname);
char* truncateCWD();
FileFD findFileDataBlocks(char* filename);

int fs_mvFile(char* fromName, char* toName);
int fs_mvDir(char* fromName, char* toName);

#endif