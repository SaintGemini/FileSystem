/**************************************************************
* Class:  CSC-415-03 
* Name: Valeria Vallejo, Chung Hei Fong, Erik Chacon
* Student ID: 920594217  917595970  920768287
* Project: File System Project
*
* File: b_io.h
*
* Description: Interface of basic I/O functions
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H

// b_seek parameter values
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#include <fcntl.h>

// function prototype for b_io.c
int b_open (char * filename, int flags);
int b_read (int fd, char* buffer, int count);
int b_write (int fd, char * buffer, int count);
int b_seek (int fd, off_t offset, int whence);
void b_close (int fd);


#endif