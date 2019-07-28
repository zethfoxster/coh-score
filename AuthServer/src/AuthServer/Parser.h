/**
 * Parser.h
 *
 * Parsing functions and related data declaration
 *
 * author: woojeong
 *
 * created: 2002-03-11 
 *
**/

#ifndef _Parser_h_
#define _Parser_h_

enum Token 
{
	eWord,
	ePeriod, 
    eComma, 
    eMinus, 
    eColon, 
    eSemiColon, 
    eEqual, 
    eSlash, 
    eAmpersand,
	eLeftParen, 
    eRightParen, 
    eNewLine,
	eNumber,
    eEOF
};

#define MAX_TOKEN_LEN	70

extern Token GetToken(std::istream& stream, char* value, bool strOnly);

#endif
