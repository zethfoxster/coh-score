#ifndef _METADICTIONARY_H_
#define _METADICTIONARY_H_

//A MetaDictionary is a collection of dictionaries. A "dictionary", in this context, is something which converts pointers (which
//may actually be ints) to strings, and back. A metadictionary contains multiple dictionaries, each referred to by a name.

#define METADICTIONARY_MAX_STRING_LENGTH 256


//callback function which each dictionary uses to convert a string to a pointer
//returns true if the lookup succeeded
//
//the string in pInString is converted into a void*, and the output value is put into ppOutPointer. 
//
//pDictionaryData is a userData field which points to or identifies the dictionary.
typedef bool (*DictionaryS2P)(void **ppOutPointer, const char *pInString, const void *pDictionaryData);

//callback function which each dictionary uses to convert a pointer to a string
//returns true if the lookup succeeded
//
//the pointer in pInPointer is converted to a string and put into outString
//
//pDictionaryData is a userData field which points to or identifies the dictionary.
typedef bool (*DictionaryP2S)(char outString[METADICTIONARY_MAX_STRING_LENGTH], void *pInPointer, const void *pDictionaryData);


//this global function finds the dictionary with name pDictionaryName and uses it to convert the given string to a pointer
bool MetaDictionaryStringToPointer(void **ppOutPointer, const char *pInString, const char *pDictionaryName);

//this global function finds the named dictionary and uses it to convert the given pointer to a string
bool MetaDictionaryPointerToString(char outString[METADICTIONARY_MAX_STRING_LENGTH], void *pInPointer, const char *pDictionaryName);

//this function registers a new dictionary with the metadictionary
void MetaDictionaryAddDictionary(const char *pDictionaryName, void *pDictionaryData, DictionaryS2P S2PFunc, DictionaryP2S P2SFunc);

#endif


