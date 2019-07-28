#ifndef REGISTRYREADER_H
#define REGISTRYREADER_H

#include "stdtypes.h"

C_DECLARATIONS_BEGIN

typedef struct RegReaderImp *RegReader;

RegReader createRegReader(void);
void destroyRegReader(RegReader reader);
int initRegReader(RegReader reader, const char* key);
int initRegReaderEx(RegReader reader, FORMAT templateString, ...);

int rrReadString(RegReader reader, const char* valueName, char* outBuffer, int bufferSize);
int rrReadMultiString(RegReader reader, const char* valueName, char* outBuffer, int bufferSize);
int rrWriteString(RegReader reader, const char* valueName, const char* str);
int rrReadInt64(RegReader reader, const char* valueName, S64* value);
int rrReadInt(RegReader reader, const char* valueName, unsigned int* value);
int rrWriteInt64(RegReader reader, const char* valueName, S64 value);
int rrWriteInt(RegReader reader, const char* valueName, unsigned int value);
int rrFlush(RegReader reader);
int rrDelete(RegReader reader, const char* valueName);

int rrClose(RegReader reader);

int rrEnumStrings(RegReader reader, int index, char* outName, int* inOutNameLen, char* outValue, int* inOutValueLen);

// This writes a single value without doing any heap operations (used in crash reporting)
int registryWriteInt(const char *keyName, const char *valueName, unsigned int value);

C_DECLARATIONS_END
#endif
