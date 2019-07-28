/**
 * Parser.cpp
 *
 * Parsing functions definition
 *
 * author: woojeong
 *
 * created: 2002-03-11
 *
**/

#include "GlobalAuth.h"
#include "Log.h"
#include "Parser.h"

inline bool IsDigit(char c)
{
	return c >= '0' && c <= '9';
}

inline bool IsSpace(char c)
{
	return c == ' ';
}

inline bool IsWhiteSpace(char c)
{
	return c == ' ' || c == '\t' || c == '\r';
}

inline bool IsAlpha(char c)
{
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

Token GetToken(std::istream& stream, char* value, bool strOnly)
{
	if (stream.eof())
		return eEOF;
	unsigned char c;
	do {
		stream >> c;
	} while (IsWhiteSpace(c) && !stream.eof());
	if (stream.eof()) {
		return eEOF;
	} else {
		switch (c) {
		case '.':
			return ePeriod;
		case ',':
			return eComma;
		case ':':
			return eColon;
		case ';':
			return eSemiColon;
		case '=':
			return eEqual;
		case '/':
			return eSlash;
		case '&':
			return eAmpersand;
		case '(':
			return eLeftParen;
		case ')':
			return eRightParen;
		case '\n':
			return eNewLine;
		}
		char* valPtr = value;
		int count = 0;
		if (c == '-') {
			stream >> c;
			if (IsDigit(c)) {
				*valPtr++ = '-';
				count++;
			} else {
				stream.putback(c);
				return eMinus;
			}
		}
		if (IsDigit(c)) {
			do {
				*valPtr++ = c;
				count++;
				stream >> c;
			} while (IsDigit(c) && count < MAX_TOKEN_LEN && !stream.eof());
			if (!stream.eof()) {
				stream.putback(c);
			}
			*valPtr = 0;
			if (strOnly) {
				return eWord;
			} else {
				return eNumber;
			}
		}
		if (c == '"') {
			stream >> c;
			while (c != '"' && c != '\n' && count < MAX_TOKEN_LEN && !stream.eof()) {
				*valPtr++ = c;
				count++;
				stream >> c;
			}
			if (c == '\n') {
				stream.putback(c);
			}
			*valPtr = 0;
			return eWord;
		}
		if (IsAlpha(c) || c >= 0xa1 && c < 0xfe) {
			do {
				*valPtr++ = c;
				count++;
				stream >> c;
			} while ((IsAlpha(c) || c >= 0xa1 && c < 0xfe || c == '_') && !stream.eof());
			if (!stream.eof()) {
				stream.putback(c);
				if (stream.bad()) {
					log.AddLog(LOG_ERROR, "putback");
				}
			}
			*valPtr = 0;
			return eWord;
		} else {
			log.AddLog(LOG_ERROR, "unexpected char '%c'", c);
			return eEOF;
		}
	}
}
