#pragma once

#include "stdio.h"

extern bool gbLastFWCloseActuallyWrote;

typedef struct FileWrapper FileWrapper;
#define FILE FileWrapper

FileWrapper *fw_fopen(const char *pFilename, const char *pMode);
void fw_fclose(FileWrapper *pFile);
int fw_fseek(FileWrapper *pFile, long iBytes, int iWhere);
int fw_fprintf(FileWrapper *pFile, const char *pFmt, ...);
char fw_getc(FileWrapper *pFile);
void fw_putc(char c, FileWrapper *pFile);
int fw_ftell(FileWrapper *pFile);
int fw_fread(void *pBuf, int iSize, int iCount, FileWrapper *pFile);

#define fopen(fname, mode) fw_fopen(fname, mode)
#define fclose(fname) fw_fclose(fname)
#define fseek(fname, bytes, where) fw_fseek(fname, bytes, where)
#define fprintf(pFile, pFmt, ...) fw_fprintf(pFile, pFmt, __VA_ARGS__)
#define getc(pFile) fw_getc(pFile)
#define putc(c, pFile) fw_putc(c, pFile)
#define ftell(pFile) fw_ftell(pFile)
#define fread(pBuf, iSize, iCount, pFile) fw_fread(pBuf, iSize, iCount, pFile)