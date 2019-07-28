#include "wininclude.h"
#include "StringUtil.h"
#include "error.h"
#include "assert.h"
#include "stdtypes.h"


#ifndef IS_SURROGATE_PAIR // in case windows headers are not recent
	#define HIGH_SURROGATE_START  0xd800
	#define HIGH_SURROGATE_END    0xdbff
	#define LOW_SURROGATE_START   0xdc00
	#define LOW_SURROGATE_END     0xdfff
	#define IS_HIGH_SURROGATE(wch) (((wch) >= HIGH_SURROGATE_START) && ((wch) <= HIGH_SURROGATE_END))
	#define IS_LOW_SURROGATE(wch)  (((wch) >= LOW_SURROGATE_START) && ((wch) <= LOW_SURROGATE_END))
	#define IS_SURROGATE_PAIR(hs, ls) (IS_HIGH_SURROGATE(hs) && IS_LOW_SURROGATE(ls)) 
#endif

#define TempBufferCharacterLength 4 * 1024
// A "narrow" string is a 8-bit per character, null terminated string.
// A "wide" string is a 16-bit per character, null terminated string.

/* Function narrowToWideStrCopy()
 *	This function takes a narrow string and copies it into the given wide string
 *	buffer.  It is assumed that the wide string buffer is large enough to hold
 *	the string.
 */
int narrowToWideStrCopy(const char* narrowString, unsigned short* wideString){
	const char* srcCursor = narrowString;
	unsigned short* dstCursor = wideString;
	int strLength = 0;

	while(*srcCursor){
		*(dstCursor++) = *(srcCursor++);
		strLength++;
	}
	wideString[strLength] = '\0';

	return strLength;
}


/* Function narrowToWideStrCopyChars
 *	This function takes a narrow string and copies the specified number of
 *	characters into the given wide string buffer.  It is assumed that the
 *	wide string buffer is large enough to hold the string.
 */
int narrowToWideStrCopyChars(const char* narrowString, unsigned short* wideString, int charsToCopy){
	const char* srcCursor = narrowString;
	unsigned short* dstCursor = wideString;
	int strLength = 0;

	while(*srcCursor && charsToCopy){
		*(dstCursor++) = *(srcCursor++);
		strLength++;
		charsToCopy--;
	}
	wideString[strLength] = '\0';

	return strLength;
}


/* Function UTF8ToWideStrConvert()
 *	This function will return the given UTF-8 string
 */
int UTF8ToWideStrConvert(const char* str, unsigned short* outBuffer, int outBufferMaxLength){
	int result;
	int bufferSize;
	int strSize;

	// If either the outBuffer or the out buffer length is 0,
	// the user is asking how long the string will be after conversion.
	if(!outBuffer || !outBufferMaxLength){
		outBuffer = NULL;
		bufferSize = 0;

		if('\0' == *str)
			return 0;
	}
	else{
		bufferSize = outBufferMaxLength;

		// If the given string is an emtpy string, pass back an emtpy string also.
		if('\0' == *str){
			outBuffer[0] = '\0';
			return 0;
		}
	}
	
	strSize = (int)strlen(str);//(bufferSize ? min(strlen(str), bufferSize) : strlen(str));
	result = MultiByteToWideChar(CP_UTF8, 0, str, strSize, outBuffer, bufferSize);

	if(!result)
		printWinErr("MultiByteToWideChar", __FILE__, __LINE__, GetLastError());

	if(outBuffer)
		outBuffer[result] = 0;
	// Do not count the null terminating character as part of the string.
	return result;
}

unsigned short *UTF8ToWideStrConvertStatic(const char *str)
{
    THREADSAFE_STATIC unsigned short *s_usBuffer = NULL;
	THREADSAFE_STATIC int s_iLen = 512;
	int len;

	THREADSAFE_STATIC_MARK(s_usBuffer);

	if(!s_usBuffer)
	{
		s_usBuffer = (unsigned short *)malloc(s_iLen*sizeof(unsigned short));
	}

	s_usBuffer[0]='\0';

	if(!str || (len = UTF8ToWideStrConvert(str, NULL, 0))==0)
	{
		return s_usBuffer;
	}
	// Add space for a terminator
	len++;

	if(len > s_iLen)
	{
		s_iLen = len+128;
		s_usBuffer = realloc(s_usBuffer, s_iLen*sizeof(unsigned short));
	}

	UTF8ToWideStrConvert(str, s_usBuffer, s_iLen);

	return s_usBuffer;
}


unsigned short UTF8ToWideCharConvert(const char* str){
	int result;
	unsigned short outputBuffer = 0;

	if( str && *str )
	{
		result = MultiByteToWideChar(CP_UTF8, 0, str, UTF8GetCharacterLength(str), &outputBuffer, 1);
		if(!result)
			printWinErr("MultiByteToWideChar", __FILE__, __LINE__, GetLastError());
	}
	
	return outputBuffer;
}

bool utf8StrValid(char const * str)
{
	int cont = 0;
	if(str)
	{

		for(;*str;++str)
		{
			unsigned char c = *str;
			switch ( cont )
			{
			case 0:
				if(c < 0x80)		// 7-bit ASCII
					cont = 0;
				else if(c < 0xC0)	// continuation char, invalid
					return false;
				else if(c < 0xE0)	// two leading bits
					cont = 1;
				else if(c < 0xF0)	// three leading bits
					cont = 2;
				else
					cont = 3;
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				if((c&0xC0)!=0x80) // need 10 for leading bits
					return false;
				cont--;
				break;
			default:
				verify(0 && "invalid switch value.");
			};
		}
	}
	return cont == 0; // NULL is a valid utf8 str...
}

int WideToUTF8StrConvert(const unsigned short* str, char* outBuffer, int outBufferMaxLength){
	int result;
	int bufferSize;

	// If either the outBuffer or the out buffer length is 0,
	// the user is asking how long the string will be after conversion.
	if(!outBuffer || !outBufferMaxLength){
		outBuffer = NULL;
		bufferSize = 0;

		if('\0' == *str)
			return 0;
	}
	else{
		bufferSize = outBufferMaxLength;

		// If the given string is an emtpy string, pass back an emtpy string also.
		if('\0' == *str){
			outBuffer[0] = '\0';
			return 0;
		}
	}

	result = WideCharToMultiByte(CP_UTF8, 0, str, (int)wcslen(str), outBuffer, bufferSize, NULL, NULL);

	if(!result)
		printWinErr("WideCharToMultiByte", __FILE__, __LINE__, GetLastError());
	
	if(outBuffer)
		outBuffer[result] = 0;
	return result;
}

char* WideToUTF8StrTempConvert(const unsigned short* str, int* newStrLength){
	int strLength;
	#define bufferSize TempBufferCharacterLength
	static char outputBuffer[bufferSize];

	strLength = WideToUTF8StrConvert(str, outputBuffer, bufferSize);
	if(newStrLength)
		*newStrLength = strLength;
	return outputBuffer;
	#undef bufferSize
}

char* WideToUTF8CharConvert(unsigned short character){
	// Max UTF8 character length should be 6 bytes, add one for null terminating character.
	static char outputBuffer[7];
	unsigned short unicodeString[2];

	unicodeString[0] = character;
	unicodeString[1] = '\0';

	WideToUTF8StrConvert(unicodeString, outputBuffer, 7);
	return outputBuffer;
}


/* Function UTF8GetCharacterLength()
 *	Given the pointer to the first byte in a UTF-8 encoded character string, return the number of bytes 
 *	used to represent the first unicode character.
 *
 *	Character length encoding chart:
 *		0xxxxxxx	- 1 byte
 *		110xxxxx	- 2 bytes
 *		1110xxxx	- 3 bytes
 *		11110xxx	- 4 bytes
 *		111110xx	- 5 bytes
 *		1111110x	- 6 bytes
 *
 *	Returns:
 *		0 - if the first byte in the string is not the first byte of a UTF-8 character.
 *		number between 1 and 6 - if the first byte in the string is the first byte of a UTF-8 character.
 */
int UTF8GetCharacterLength(const char* inputStr){
	const unsigned char* str = inputStr;

	// If the character looks like 10xxxxxx, the character is not the first byte of
	// a UTF-8 encoded character.
	if((*str & ~((1 << 6) - 1)) == 0x80){
		return 1;
	}

	// If the character looks like 0xxxxxxx, the character is 1 byte long.
	if((*str & ~((1 << 7) - 1)) == 0x00)
		return 1;

	// If the character looks like 110xxxxx, the character is 2 bytes long.
	if((*str & ~((1 << 5) - 1)) == 0xC0)
		return 2;

	// If the character looks like 1110xxxx, the character is 3 bytes long.
	if((*str & ~((1 << 4) - 1)) == 0xE0)
		return 3;

	// If the character looks like 11110xxx, the character is 4 bytes long.
	if((*str & ~((1 << 3) - 1)) == 0xF0)
		return 4;

	// If the character looks like 111110xx, the character is 5 bytes long.
	if((*str & ~((1 << 2) - 1)) == 0xF8)
		return 5;

	// If the character looks like 1111110x, the character is 6 bytes long.
	if((*str & ~((1 << 1) - 1)) == 0xFC)
		return 6;

	// Should never happen.
//	assert(0);
	return 1;
}

/* Function UTF8GetNextCharacter()
 *	Given a character pointer to the first byte of a UTF8 encoded string, return
 *	the pointer to the first byte of the next character.
 *
 *	Returns:
 *		NULL string - cannot move to the next character because str is already at the end of a string.
 *		char* - a valid pointer that points at str[1].
 */
char* UTF8GetNextCharacter(const char* str){
	assert(str);

	if('\0' == *str)
		return (char *)str;
	
	return (char *)str + UTF8GetCharacterLength(str);
}

/* Function UTF8GetCharacter()
 *	Given a character pointer to the first byte of a UTF8 encoded string and a character index,
 *	return the pointer to the first byte of the indexed character.
 *
 *	Returns:
 *		NULL - the index specifies beyond the end of the UTF8 encoded string.
 *		char* - a valid pointer that points at str[indexed character].
 */
char* UTF8GetCharacter(const char* str, int index){
	int i;

	for(i = 0; i < index; i++){
		// Have we gone past the end of the string?
		if(!*str)
			return NULL;
		str += UTF8GetCharacterLength(str);
	}

	return (char *)str;
}

int UTF8GetLength(const char* str){
	int characterCount = 0;
	int i = 0;
	int byteLength = strlen(str);
	static int debugLength = 16;

	while(str[i]){
		i += UTF8GetCharacterLength(str + i);
		characterCount++;

		//	this assert means that the string doesn't have a properly encoded string
		//	an example catch made is that the character × (not x) was in menumessages.ms
		//	"ToHitToolTip",			"ToHit is an important part of your actual chance to hit a target.  Hit Chance = Clamp(Player Accuracy × Power Accuracy × Clamp( ToHit – Target Defense )). The clamp keeps numbers between 5% and 95%."
		//	which tripped this assert
		//	Temporary: thinking of a way to trap this before it gets here, or how to pass this up properly.
		devassertmsg(i <= byteLength, "last %d chars: %s", debugLength, binStrToHexStr(&str[max(0, byteLength - debugLength)], min(byteLength, debugLength) + 1));
	}

	return characterCount;
}

/* Function UTF8RemoveChars()
 *	Given a string and two character indicies, this function cuts out everything in between the two indices, exclusive.
 */
void UTF8RemoveChars(char* str, int beginIndex, int endIndex){
	int characterCount = 0;
	char* beginTruncatePosition = NULL;
	char* endTruncatePosition = NULL;
	int copySize;

	// shortcut out
	if( endIndex - beginIndex <= 0 )
	{
		return;
	}

	// Find the character marked by the begin index.
	beginTruncatePosition = str;

	while(*beginTruncatePosition){
		if(beginIndex <= characterCount)
			break;
		
		characterCount++;
		beginTruncatePosition += UTF8GetCharacterLength(beginTruncatePosition);
	}

	// Find the character before the one marked by the end index.
	endTruncatePosition = beginTruncatePosition;
	while(*endTruncatePosition){
		if(endIndex <= characterCount)
			break;
		
		characterCount++;
		endTruncatePosition += UTF8GetCharacterLength(endTruncatePosition);
	}

	copySize = (int)strlen(endTruncatePosition) + 1;
	memmove(beginTruncatePosition, endTruncatePosition, copySize);
}

/** 
* UTF16GetCharacterLength
*
* Given the pointer to the first two words in a UTF-16 encoded character string,
* return the number of words used to represent the first unicode character.
*
* @retval 1 Character is not a surrogate pair.
* @retval 2 Character is a surrogate pair.
*/
int UTF16GetCharacterLength(const unsigned short* str) {
	return IS_SURROGATE_PAIR(str[0], str[1]) ? 2 : 1;
}

int UTF16GetLength(const unsigned short* str) {
	int characterCount = 0;
	int i = 0;
	int wordLength = wcslen(str);

	while (str[i]) {
		i += UTF16GetCharacterLength(str + i);
		characterCount++;
	}

	return characterCount;
}

char *strnchr(char *pString, int iLength, char c)
{
	while (*pString && iLength)
	{
		if (*pString == c)
		{
			return pString;
		}

		pString++;
		iLength--;
	}

	return NULL;
}

void Str_trimTrailingWhitespace(char *s)
{
	char *e = s + strlen(s) - 1;
	while (e >= s && *e==' ') *e--=0;
}

// *********************************************************************************
//  hexString conversions
// *********************************************************************************

unsigned char* hexStrToBinStr(char* hexString, int strLength)
{
	static char* binStr = NULL;
	static int binStrSize = 0;
	int i, len;

	// Make sure the output buffer is large enough.
	if(binStrSize < (strLength / 2) + 1)
	{
		binStr = realloc(binStr, (strLength / 2) + 1);
		binStrSize = (strLength / 2) + 1;
	}
	memset(binStr, 0, (strLength / 2) + 1); // init
	
	// limit reading to the actual length of the string
	len = strlen(hexString); // !this will let the last character fall off if we had an odd length string 

	for(i = 0; i < len / 2; i++)
	{
		int value;
		sscanf(&hexString[i*2], "%02x", &value);
		binStr[i] = value;
	}

	if(len%2)
	{
		int value;
		char tmp[3];
		tmp[0] = hexString[len-1];
		tmp[1] = '0';
		tmp[2] = 0;
		sscanf(tmp, "%02x", &value);
		binStr[i++] = value;
	}
	binStr[i] = 0;

	return binStr;
}

static INLINEDBG char getHex(int value)
{
	if (value < 10)
		return value + '0';
	return value - 10 + 'a';
}

char* binStrToHexStr(const unsigned char* binArray, int arrayLength)
{
	static char* hexStr = NULL;
	static int hexStrSize = 0;
	int i,len=-1;

	// Make sure the output buffer is large enough.
	if(hexStrSize < (arrayLength * 2) + 1)
	{
		hexStr = realloc(hexStr, (arrayLength * 2) + 1);
		hexStrSize = (arrayLength * 2) + 1;
	}

	// find last non-zero value in binArray
	for(i = arrayLength-1; i >= 0; i--)
	{
		if (binArray[i])
		{
			len = i;
			break;
		}
	}

	for(i=0;i<=len;i++)
	{
		// make sure we don't allow an odd-length string to be created
		hexStr[i*2+0] = getHex(binArray[i] >> 4);
		hexStr[i*2+1] = getHex(binArray[i] & 15);
	}
	hexStr[i*2] = '\0';
	return hexStr;
}
