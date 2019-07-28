#ifndef STRINGUTIL_H
#define STRINGUTIL_H

int narrowToWideStrCopy(const char* narrowString, unsigned short* wideString);
int narrowToWideStrCopyChars(const char* narrowString, unsigned short* wideString, int charsToCopy);
int UTF8ToWideStrConvert(const char* str, unsigned short* outBuffer, int outBufferMaxLength);
unsigned short* UTF8ToWideStrConvertStatic(const char *str);
unsigned short UTF8ToWideCharConvert(const char* str);
bool utf8StrValid(char const * str);
int WideToUTF8StrConvert(const unsigned short* str, char* outBuffer, int outBufferMaxLength);
char* WideToUTF8StrTempConvert(const unsigned short* str, int* newStrLength);
char* WideToUTF8CharConvert(unsigned short character);

int UTF8GetCharacterLength(const char* str);
char* UTF8GetNextCharacter(const char* str);
char* UTF8GetCharacter(const char* str, int index);
int UTF8GetLength(const char* str);
void UTF8RemoveChars(char* str, int beginIndex, int endIndex);

int UTF16GetCharacterLength(const unsigned short* str);
int UTF16GetLength(const unsigned short* str);

char *strnchr(char *pString, int iLength, char c);
void Str_trimTrailingWhitespace(char *str);

// hex string conversions
unsigned char* hexStrToBinStr(char* hexString, int strLength);
char* binStrToHexStr(const unsigned char* binArray, int arrayLength);

#endif
