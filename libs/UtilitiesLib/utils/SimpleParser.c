#include "SimpleParser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "utils.h"

/********************************************************************
 * Begin string parsing utilties
 *
 *
 */

static char* buffer = NULL;
static char* bufferCursor;

static char* getNextToken(){
	if(!bufferCursor)
		return NULL;
	
	// Found an open quote?
	if('\"' == *bufferCursor){
		char* token;
		char* endQuote;

		// Save off the beginning of the token to be returned.
		token = bufferCursor + 1;

		// Find a matching end quote, then return the enclosed string.
		endQuote = strchr(token, '\"');
		if(endQuote){
			
			// Mark the end of the token.
			*endQuote = '\0';

			// Move bufferCursor to the next token so this function
			// may continue to process this string.
			bufferCursor = endQuote + 1;

			// Forward the cursor until it hits either null terminating character
			// or a space.
			while('\0' != *bufferCursor && ' ' == *bufferCursor)
				bufferCursor++;
		}

		// Here, return either the quoted string, or the unclosed string.
		return token;
	}

	return strsep(&bufferCursor, " ");
}

int isEmptyStr(const char* str){
	const char* s = str;

	if (!s)
		return 1;

	while(*s != '\0' && characterIsSpace(*s))
		s++;

	if(strlen(s) == 0)
		return 1;
	else
		return 0;
}

int isAlphaNumericStr(const char* str){
	int stringLength;
	int i;

	stringLength = (int)strlen(str);
	for(i = 0; i < stringLength; i++){
		if(!isalnum((unsigned char)str[i]))
			return 0;
	}

	return 1;
}

const char *removeLeadingWhiteSpaces(const char* str)
{
	while(characterIsSpace(*str))
		str++;
	return str;
}

void removeTrailingWhiteSpaces(char** str){
	// Place a cursor at the end of the string
	char* cursor = *str + strlen(*str) - 1;

	while(characterIsSpace(*cursor)){
		*cursor = '\0';
		--cursor;
	}
}

void beginParse(char* string){
	bufferCursor = buffer = string;
}

/* Function getInt()
 *	Attempts to extract an integer from the string being parsed.
 *	If the operation is successful, the converted value is
 *	stored into the integer pointed to by "output".
 *
 *	Parameters:
 *		output - address where the converted integer should be stored
 *
 *	Returns:
 *		1 - operation succeeded
 *		0 - operation failed
 */
int getInt(int* output){
	char* token = getNextToken();
	if(token){
		*output = atoi(token);
		return 1;
	}
	
	return 0;
}


int getFloat(float* output){
	char* token = getNextToken();
	if(token){
		*output = (float)atof(token);
		return 1;
	}
	
	return 0;
}

/* Function getString()
 *	Attempts to extract a string that does not contain a whitespace
 *	from the string being parsed.  If the operation is successful,
 *	the pointer to the string is stored in "output".  Otherwise,
 *	it wiil store NULL into "output".  Note that the resulting string
 *	pointer points to some location in the string being parsed.
 *
 *	Parameters:
 *		output - address where the extracted string (token) should be stored
 *
 *	Returns:
 *		1 - operation succeeded
 *		0 - operation failed
 */
int getString(char** output){
	char* token;
	
	do{
		token = getNextToken();
		if(!token){
			output = NULL;
			return 0;
		}
	}while(isEmptyStr(token));

	token = (char*)removeLeadingWhiteSpaces(token);
	removeTrailingWhiteSpaces(&token);

	*output = token;
	return 1;
}

void endParse(){
	bufferCursor = buffer = NULL;
}

/* 
 *
 *
 * End string parsing utilties
 ********************************************************************/