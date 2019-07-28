/* File SimpleParser.h
 *	Contains simple string parsing utilities.
 *
 *
 */

#ifndef SIMPLEPARSER_H
#define SIMPLEPARSER_H

typedef const char *constCharPtr;

/********************************************************************
 * Begin string parsing utilties
 */

static INLINEDBG int characterIsSpace(unsigned char c){return isspace(c);}

void beginParse(char* string);
int getInt(int* output);
int getFloat(float* output);
int getString(char** output);
void endParse();
const char *removeLeadingWhiteSpaces(const char *str);
void removeTrailingWhiteSpaces(char** str);
int isEmptyStr(const char* str);
int isAlphaNumericStr(const char* str);
/* 
 * End string parsing utilties
 ********************************************************************/

#endif