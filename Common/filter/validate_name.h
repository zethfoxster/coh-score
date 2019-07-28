/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef VALIDATE_NAME_H__
#define VALIDATE_NAME_H__

typedef enum ValidateNameResult
{
	VNR_Valid = 0, // must be 0
	VNR_Malformed,
	VNR_Profane,
	VNR_Reserved,
	VNR_Titled,
	VNR_WrongLang,
	VNR_TooLong,
	VNR_TooShort,
} ValidateNameResult;

int IsTitledName(const char *pch);
ValidateNameResult ValidateName(const char *pchName, const char *pchAuth, bool bCheckTitles, int max_length);
ValidateNameResult ValidateCustomObjectName(char *pchName, int max_length, int isName);
void stripNonAsciiChars( char * str );  // use region-specific unless you really need ASCII
void stripNonRegionChars( char * str, int locale ); // region specific filter function.
void filterRegistrationName( char *str, int locale );
int isInvalidFilename(char *str);
int characterFilter( char ch );
bool wcharFilter( wchar_t ch );
bool wcharKoreanFilter( wchar_t ch);

int NonAsciiChars(const char * str );
int NonKoreanChars(const char * str );
int multipleNonAciiCharacters( const char * str );
int BeginsOrEndsWithWhiteSpace(char *str);

#if !defined(CHATSERVER)
bool BeginsWithOrigin( const char * str );
#endif

#endif /* #ifndef VALIDATE_NAME_H__ */

/* End of File */

