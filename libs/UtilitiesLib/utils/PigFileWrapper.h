/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * Module Description:
 *   Wrapper for piglib functions to support fopen, fread, fseek, etc
 * 
 ***************************************************************************/
#pragma once
#ifndef _PIGFILEWRAPPER_H
#define _PIGFILEWRAPPER_H

#include "piglib.h"

void *pig_fopen(const char *name,const char *how);
void *pig_fopen_pfd(PigFileDescriptor *pfd,const char *how);
int pig_fclose(void *handle);
int pig_fseek(void *handle,long dist,int whence);
long pig_ftell(void *handle);
long pig_fread(void *handle, void *buf, long size);
char *pig_fgets(void *handle,char *buf,int len);
int pig_fgetc(void *handle);
#define pig_getc(handle) pig_fgetc(handle)
//long pig_fwrite(const void *buf,long size1,long size2,FileWrapper *fw)=NULL;
//void pig_fflush(FileWrapper *fw)=NULL;
//int pig_fprintf (FileWrapper *fw, const char *format, ...);
//int pig_fscanf(FileWrapper* fw, const char* format, ...);
//int pig_fputc(int c,FileWrapper* fw);

void *pig_lockRealPointer(void *handle);
void pig_unlockRealPointer(void *handle);

long pig_filelength(void *handle);

void initPigFileHandles(void);

void pig_freeOldBuffers(void); // Frees buffers on handles that haven't been accessed in a long time
void pig_freeBuffer(void *handle); // Frees a buffer, if there is one, on a zipped file handle

#endif