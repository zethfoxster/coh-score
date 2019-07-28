#ifndef _AUTO_ENUMS_H_
#define _AUTO_ENUMS_H_

#include "stdio.h"
#include "stddef.h"
#include "stdtypes.h"
#include "metadictionary.h"

typedef struct
{
	int iIndex, iValue;
} AUTO_ENUM_INIT_VALUE_PAIR;

typedef struct
{
	char *pEnumName;
	char **ppEnumEntries;
	int iNumEntries;
	AUTO_ENUM_INIT_VALUE_PAIR *pInitValuePairs;
	void *pStashTable; //stashtable for converting strings to indices
} AUTO_ENUM_INFO;

typedef AUTO_ENUM_INFO* AutoEnumHandle;
#define AUTO_ENUM_HANDLE_INVALID (NULL)


char *AutoEnumStringFromIndex(AutoEnumHandle handle, intptr_t iIndex);

bool AutoEnumIndexFromString(intptr_t *pOutVal, AutoEnumHandle handle, const char *pString);

//string to pointer and pointer to string funcs for MetaDictionary (void* is just enum index)
bool AutoEnumDictionaryS2P(void **ppOutPointer, const char *pString, const void *pDictionaryData);
bool AutoEnumDictionaryP2S(char outString[METADICTIONARY_MAX_STRING_LENGTH], void *pInPointer, const void *pDictionaryData);


void AutoEnums_Init(AUTO_ENUM_INFO *pAutoEnums, int iNumAutoEnums);

#define AUTOENUMS_INIT(projName) { extern int gNumAutoEnums##projName; extern AUTO_ENUM_INFO gAllAutoEnums##projName[]; AutoEnums_Init(gAllAutoEnums##projName, gNumAutoEnums##projName); }



#endif
