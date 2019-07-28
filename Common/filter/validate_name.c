/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <string.h>
#include "assert.h"
#include "StringUtil.h"
#include "stdtypes.h"
#include "earray.h"
#include "utils.h"

#ifndef CHATSERVER
#include "titles.h"
#include "profanity.h"
#include "entVarUpdate.h"
#include "AppLocale.h"
#endif

#if SERVER
#include "reserved_names.h"
#endif

#include "validate_name.h"

// Allowed korean character ranges
#define HANGUL_FIRST 44032
#define HANGUL_LAST 55203 
#define JAMO_FIRST 4352
#define JAMO_LAST 4607 
#define JAMOC_FIRST 12592
#define JAMOC_LAST 12687


#ifndef CHATSERVER

/**********************************************************************func*
 * IsTitledName
 *
 */
static int TitleIsInDictionary(const char *pch, const TitleDictionary *pDict)
{
	int i;

	// Disallow names with the title prefixes
	// Skip the base definitions ("the" and "none", right now)
	for(i=eaSize(&pDict->ppOrigins)-1; i>0; i--)
	{
		int j;
		// modified to not check against ""
		for(j=eaSize(&pDict->ppOrigins[i]->ppchTitles)-1; j>=0; j--)
		{
			if(strlen(pDict->ppOrigins[i]->ppchTitles[j]) &&
				strnicmp(pDict->ppOrigins[i]->ppchTitles[j], pch,
				strlen(pDict->ppOrigins[i]->ppchTitles[j]))==0)
			{
				return true;
			}
		}
	}

	return false;
}

int IsTitledName(const char *pch)
{
	int retval = false;

	if (TitleIsInDictionary(pch, &g_TitleDictionary))
		retval = true;
	else if (TitleIsInDictionary(pch, &g_v_TitleDictionary))
		retval = true;
	else if (TitleIsInDictionary(pch, &g_l_TitleDictionary))
		retval = true;
	else if (TitleIsInDictionary(pch, &g_p_TitleDictionary))
		retval = true;

	return retval;
}

/**********************************************************************func*
* ValidateHeroName
*	Validates custom object names (i.e. custom villain group names and custom critter names
*	Only does length, white space, and profanity/copyright checks
*/
ValidateNameResult ValidateCustomObjectName(char *pchName, int max_length, int isName)
{
	int utfLen;
	utfLen = UTF8GetLength(pchName);
	if ( utfLen > max_length)
	{
		return VNR_TooLong;
	}
	else if (utfLen < 3)
	{
		return VNR_TooShort;
	}
	else if( (isName && multipleNonAciiCharacters(pchName)) || (pchName[0]==' ') || (pchName[strlen(pchName)-1]==' ') || strstr(pchName, "  "))
	{
		return VNR_Malformed;
	}
	else
	{
		if (IsAnyWordProfaneOrCopyright(pchName))
		{
			return VNR_Profane;
		}
	}

	return VNR_Valid;
}

/**********************************************************************func*
 * ValidateHeroName
 *
 */
ValidateNameResult ValidateName(const char *pchName, const char *pchAuth, bool bPlayerName, int max_length)
{
	int locale = getCurrentLocale(), maxchars;
	bool nonascii, nonkorean;
	nonascii = NonAsciiChars(pchName);
	nonkorean = NonKoreanChars(pchName);

	if (bPlayerName) 
	{//we're probably doing a player name, do special language stuff
		if (locale == 3 && !nonkorean) {
			maxchars = 6;
		} else {
			maxchars = max_length;
		}		
	}
	else
		maxchars = max_length;

	if (UTF8GetLength(pchName) > maxchars)
	{
		return VNR_TooLong;
	}
	else if (UTF8GetLength(pchName) < 3)
	{
		return VNR_TooShort;
	}
#if SERVER
	else if(IsReservedName(pchName, pchAuth))
	{
		return VNR_Reserved;
	}
#endif
#if !defined(CHATSERVER)
	else if(bPlayerName && BeginsWithOrigin(pchName))
	{
		return VNR_Malformed;
	}
#endif
	else if((bPlayerName && multipleNonAciiCharacters(pchName)) || pchName[0]==' ' || pchName[strlen(pchName)-1]==' ' || strstr(pchName, "  "))
	{
		return VNR_Malformed;
	}
	else if(locale != 3 && nonascii) //if not korean, enforce ascii
	//else if(nonascii) //remove korean name support for now
	{
		return VNR_WrongLang;
	} 
	else if (locale == 3 && nonascii && nonkorean) 
	{
		return VNR_WrongLang;
	} 
	else 
	{
		// We check for profanity word by word
		char *pchCur;
		char *pchBuff = _alloca(strlen(pchName)+2);
		pchBuff[strlen(pchName)+1]='\0'; // HACK: just let it go.

		strcpy(pchBuff, pchName);

		pchCur=strtok(pchBuff, "-_?.,! ");

		while(pchCur!=NULL)
		{
			if(IsProfanity(pchCur, strlen(pchCur))
				|| IsProfanity(pchName+(pchCur-pchBuff), strlen(pchCur)))
			{
				return VNR_Profane;
			}

			pchCur=strtok(NULL, "-_?.,! ");
		}
	}

	return VNR_Valid;
}

#endif

// Allow only hangul and numbers
bool wcharKoreanFilter( wchar_t ch) {
	if( ch )
	{
		if( ch < '0' || ch > '9' ) // numbers
		{
			if( ch < HANGUL_FIRST || ch > HANGUL_LAST ) // A-Z
			{
				if( ch < JAMO_FIRST || ch > JAMO_LAST ) // A-Z
				{
					if( ch < JAMOC_FIRST || ch > JAMOC_LAST ) // A-Z
					{
						if( ch != '\''  && // apostrophe
							ch != '-'  && // hyphen
							ch != ' '  && // space
							ch != '.' && // period
							ch != '?' )	// question mark
						{
							return true;	
						}
					}
				}
			}
		}
	}
	return false;
}

static bool utf8KoreanFilter( const char *str )
{
	return wcharKoreanFilter(UTF8ToWideCharConvert(str));
}

bool wcharFilter( wchar_t ch )
{
	// don't filter terminator
	if( ch )
	{
		if( ch < '0' || ch > '9' ) // numbers
		{
			if( ch < 'A' || ch > 'Z' ) // A-Z
			{
				if( ch < 'a' || ch > 'z' ) // a-z
				{
					if( ch != '\''  && // apostrophe
						ch != '-'  && // hyphen
						ch != ' '  && // space
						ch != '.' && // period
						ch != '?' )	// question mark
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

int isInvalidFilename(char *pch)
{
	// if the first character is a period, the file scan will not like it
	if (pch && *pch == '.')
		return 1;

	while(pch && *pch)
	{
		switch(*pch)
		{
		case '?':
		case '\\':
		case '//':
		case '*':
		case ':':
		case ',':
			return 1;
			break;
		}
		pch++;
	}
	return 0;
}

static bool utf8Filter( const char *str )
{
	return wcharFilter(UTF8ToWideCharConvert(str));
}

/**********************************************************************func*
* characterFilter
*
*/
int characterFilter( char ch )
{
	return !wcharFilter( ch );
// 	if( ch < '0' || ch > '9' ) // numbers
// 	{
// 		if( ch < 'A' || ch > 'Z' ) // A-Z
// 		{
// 			if( ch < 'a' || ch > 'z' ) // a-z
// 			{
// 				if( ch != '\''  && // apostrophe
// 					ch != '-'  && // hyphen
// 					ch != ' '  && // space
// 					ch != '.' && // period
// 					ch != '?' )	// question mark
// 				{
// 					return 0;
// 				}
// 			}
// 		}
// 	}

// 	return 1;
}

/**********************************************************************func*
* stripNonAsciiChars
*
*/
void stripNonAsciiChars( char * str )
{	
	if(verify(str))
	{
		do
		{
			char *tmp = str;
			int lenRemove;
			for( lenRemove = 0; utf8Filter(tmp); ++lenRemove )
			{
				tmp = UTF8GetNextCharacter( tmp );
			}
			UTF8RemoveChars(str, 0, lenRemove );
		} while( *str && (str = UTF8GetNextCharacter( str )) );
	}
}


/**********************************************************************func*
* stripNonAsciiChars
*
*/
int NonAsciiChars(const char * str )
{
	int i, len = strlen( str );

	for( i = 0; i < len; i++ )
	{
		if( !characterFilter( str[i] ) )
		{
			return true;
		}
	}
	return false;
}

int NonKoreanChars(const char * str )
{
	if(verify(str))
	{
		do
		{
			if (utf8KoreanFilter(str)) {
				return true;
			}
		} while( *str && (str = UTF8GetNextCharacter( str )) );
	}
	return false;
}

int multipleNonAciiCharacters( const char * str )
{
	int i = 0;
	int count = 0;

	for( i = 0; i < (int)strlen(str); i++ )
	{
		char ch = str[i];

		if( ch == 39  || // apostrophe
			ch == 45  || // hyphen
			ch == '.' || // period
			ch == 63 )  // question mark
		{
			count++;
		}
	}

	return  ((count > 1) || (count == 1 && strlen(str) == 1));
}

int IsWhiteSpace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int BeginsOrEndsWithWhiteSpace(char *str)
{
	int len = strlen(str);

	if (len && (IsWhiteSpace(str[0]) || IsWhiteSpace(str[len-1])))
		return 1;

	return 0;
}

#if !defined(CHATSERVER)
bool BeginsWithOrigin( const char * str )
{
	int i, j;
	size_t lenStr = strlen(str);

	if(!getCurrentLocale())	// only for english
	{
		for(i = 0; i < eaSize(&g_TitleDictionary.ppOrigins); i++ )
		{
			if( stricmp( "Base", g_TitleDictionary.ppOrigins[i]->pchOrigin ) == 0 )
			{
				for(j = 0; j < eaSize(&g_TitleDictionary.ppOrigins[i]->ppchTitles); j++)
				{
					const char* originTitle = g_TitleDictionary.ppOrigins[i]->ppchTitles[j];
					size_t lenTitle = strlen(originTitle);
					if( lenStr >= (lenTitle + 1) &&
						strnicmp( originTitle, str, lenTitle ) == 0 &&
						str[lenTitle] == ' ')
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}
#endif

void stripInvalidKoreanChars(char *str)
{
	if(verify(str))
	{
		do
		{
			char *tmp = str;
			int lenRemove;
			for( lenRemove = 0; utf8KoreanFilter(tmp); ++lenRemove )
			{
				tmp = UTF8GetNextCharacter( tmp );
			}
			UTF8RemoveChars(str, 0, lenRemove );
		} while( *str && (str = UTF8GetNextCharacter( str )) );
	}
}

void stripNonRegionChars(char *str, int locale)
{
	// call locale if valid, or U.S. one by default
	if (locale != 3) {
		//if (true) { //remove korean name support for now
		stripNonAsciiChars(str);
		return;
	} else {
		bool na = utf8Filter(str), nk = utf8KoreanFilter(str); //check first letter
		if (!nk) { //if all korean, let that stay
			stripInvalidKoreanChars(str);
		} else { //else strip all non  ascii
			stripNonAsciiChars(str); 
		}
	}
}

//------------------------------------------------------------
//  Filter by locale
//----------------------------------------------------------
void filterRegistrationName( char *str, int locale )
{	
	stripNonRegionChars(str,locale);
}
