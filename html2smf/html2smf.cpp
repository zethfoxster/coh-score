// html2smf.cpp : simple tool to convert from html to smf for use in City of Heroes game engine
// This was made mainly to convert the EULA documents for use in the game.
// Use Microsoft Word to first export the document to an .html or .htm file and
// then run this tool on the .html or .htm file.
//
// Either of the "Web Page" or "Web Page, Filtered" options in the "Save As" dialog of
// Microsoft Word will work fine.
//
// IMPORTANT: Make sure the message store files (*.ms) are saved with UTF-8 encoding!!
//
// TODO: Maybe convert the <span style='font-size:11.0pt'> into a <scale 1.0>
//

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <Winnls.h>

#include <assert.h>

typedef enum
{
	PARSING_COPY,
	PARSING_LOAD_SPECIAL,
	PARSING_LOAD_TAG,
	PARSING_LOAD_COMMENT,
	PARSING_IGNORE_TAG,
	PARSING_IGNORE_TAG2
} PARSING_MODE;

static char s_IgnoreTag[256];

static wchar_t s_OutputBuffer[4096];
static size_t s_OutputBufferUsed = 0;

static bool s_LastCharNewLine = true;
static bool s_InAnchor = false;
static bool s_EatSpaces = false;
static float s_currentIndent = 0.0f;


//*********************************************************************
void Flush(FILE *outFile)
{
	s_OutputBuffer[s_OutputBufferUsed] = 0;
	fputws(s_OutputBuffer, outFile);
	s_OutputBufferUsed = 0;
}

//*********************************************************************
wchar_t *ExtractURL(wchar_t *cursor, wchar_t *url, size_t url_len)
{
	size_t len = 0;

	while(*cursor != 0 && *cursor != ' ' && *cursor != ')' && *cursor != ';' && *cursor != ',')
	{
		*url = *cursor;
		len++;
		assert(len < url_len);

		url++;
		cursor++;
	}

	// If it ends with a period, back up one character
	if (*(url-1) == '.')
	{
		url--;
		cursor--;
	}

	*url = 0;

	return cursor;
}

//*********************************************************************
void CheckForUnhandledAndFlush(FILE *outFile)
{
	s_OutputBuffer[s_OutputBufferUsed] = 0;

	if (!s_InAnchor)
	{
		wchar_t *cursor = wcsstr(s_OutputBuffer, L"http://");

		if (cursor)
		{
			wchar_t url[1024];

			// Flush stuff before the link
			*cursor = 0;
			CheckForUnhandledAndFlush(outFile);
			*cursor = 'h';

			cursor = ExtractURL(cursor, url, 1024);

			// anchor the link
			if (url[0] != 0)
			{
				fwprintf(outFile, L"<a href=\"%s\">%s</a>", url, url);
			}

			// Shift the remaining text to the beginning of s_OutputBuffer
			size_t remaining = wcslen(cursor);
			memmove(s_OutputBuffer, cursor, remaining * sizeof(wchar_t));
			s_OutputBufferUsed = remaining;

			// Look for more
			CheckForUnhandledAndFlush(outFile);
		}

		// convert "Copyright (c)" or "Copyright (C)" to "Copyright ©"
		cursor = wcsstr(s_OutputBuffer, L"copyright (c)");

		if (!cursor)
			cursor = wcsstr(s_OutputBuffer, L"copyright (C)");

		if (!cursor)
			cursor = wcsstr(s_OutputBuffer, L"Copyright (c)");

		if (!cursor)
			cursor = wcsstr(s_OutputBuffer, L"Copyright (C)");

		if (!cursor)
			cursor = wcsstr(s_OutputBuffer, L"COPYRIGHT (c)");

		if (!cursor)
			cursor = wcsstr(s_OutputBuffer, L"COPYRIGHT (C)");

		if (cursor)
		{
			wchar_t *cursor2 = wcsstr(cursor, L"(");

			// Flush stuff before the "(c)" or "(C)"
			*cursor2 = 0;
			CheckForUnhandledAndFlush(outFile);

			// Output copyright symbol
			fputwc(L'©', outFile);

			// Skip over the "(c)" or "(C)"
			cursor2 += 3;

			// Shift the remaining text to the beginning of s_OutputBuffer
			size_t remaining = wcslen(cursor2);
			memmove(s_OutputBuffer, cursor2, remaining * sizeof(wchar_t));
			s_OutputBufferUsed = remaining;

			// Look for more
			CheckForUnhandledAndFlush(outFile);
		}
	}

	Flush(outFile);
}

//*********************************************************************
void AddChar(int character)
{
	size_t size = swprintf_s(s_OutputBuffer + s_OutputBufferUsed, 4096 - s_OutputBufferUsed, L"%lc", character);
	s_OutputBufferUsed += size;
}

//*********************************************************************
int AsciiToUTF8(int character)
{
#if 1
	switch(character)
	{
		// From http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1252.TXT
		case 0x00: return 0x0000;	//NULL
		case 0x01: return 0x0001;	//START OF HEADING
		case 0x02: return 0x0002;	//START OF TEXT
		case 0x03: return 0x0003;	//END OF TEXT
		case 0x04: return 0x0004;	//END OF TRANSMISSION
		case 0x05: return 0x0005;	//ENQUIRY
		case 0x06: return 0x0006;	//ACKNOWLEDGE
		case 0x07: return 0x0007;	//BELL
		case 0x08: return 0x0008;	//BACKSPACE
		case 0x09: return 0x0009;	//HORIZONTAL TABULATION
		case 0x0A: return 0x000A;	//LINE FEED
		case 0x0B: return 0x000B;	//VERTICAL TABULATION
		case 0x0C: return 0x000C;	//FORM FEED
		case 0x0D: return 0x000D;	//CARRIAGE RETURN
		case 0x0E: return 0x000E;	//SHIFT OUT
		case 0x0F: return 0x000F;	//SHIFT IN
		case 0x10: return 0x0010;	//DATA LINK ESCAPE
		case 0x11: return 0x0011;	//DEVICE CONTROL ONE
		case 0x12: return 0x0012;	//DEVICE CONTROL TWO
		case 0x13: return 0x0013;	//DEVICE CONTROL THREE
		case 0x14: return 0x0014;	//DEVICE CONTROL FOUR
		case 0x15: return 0x0015;	//NEGATIVE ACKNOWLEDGE
		case 0x16: return 0x0016;	//SYNCHRONOUS IDLE
		case 0x17: return 0x0017;	//END OF TRANSMISSION BLOCK
		case 0x18: return 0x0018;	//CANCEL
		case 0x19: return 0x0019;	//END OF MEDIUM
		case 0x1A: return 0x001A;	//SUBSTITUTE
		case 0x1B: return 0x001B;	//ESCAPE
		case 0x1C: return 0x001C;	//FILE SEPARATOR
		case 0x1D: return 0x001D;	//GROUP SEPARATOR
		case 0x1E: return 0x001E;	//RECORD SEPARATOR
		case 0x1F: return 0x001F;	//UNIT SEPARATOR
		case 0x20: return 0x0020;	//SPACE
		case 0x21: return 0x0021;	//EXCLAMATION MARK
		case 0x22: return 0x0022;	//QUOTATION MARK
		case 0x23: return 0x0023;	//NUMBER SIGN
		case 0x24: return 0x0024;	//DOLLAR SIGN
		case 0x25: return 0x0025;	//PERCENT SIGN
		case 0x26: return 0x0026;	//AMPERSAND
		case 0x27: return 0x0027;	//APOSTROPHE
		case 0x28: return 0x0028;	//LEFT PARENTHESIS
		case 0x29: return 0x0029;	//RIGHT PARENTHESIS
		case 0x2A: return 0x002A;	//ASTERISK
		case 0x2B: return 0x002B;	//PLUS SIGN
		case 0x2C: return 0x002C;	//COMMA
		case 0x2D: return 0x002D;	//HYPHEN-MINUS
		case 0x2E: return 0x002E;	//FULL STOP
		case 0x2F: return 0x002F;	//SOLIDUS
		case 0x30: return 0x0030;	//DIGIT ZERO
		case 0x31: return 0x0031;	//DIGIT ONE
		case 0x32: return 0x0032;	//DIGIT TWO
		case 0x33: return 0x0033;	//DIGIT THREE
		case 0x34: return 0x0034;	//DIGIT FOUR
		case 0x35: return 0x0035;	//DIGIT FIVE
		case 0x36: return 0x0036;	//DIGIT SIX
		case 0x37: return 0x0037;	//DIGIT SEVEN
		case 0x38: return 0x0038;	//DIGIT EIGHT
		case 0x39: return 0x0039;	//DIGIT NINE
		case 0x3A: return 0x003A;	//COLON
		case 0x3B: return 0x003B;	//SEMICOLON
		case 0x3C: return 0x003C;	//LESS-THAN SIGN
		case 0x3D: return 0x003D;	//EQUALS SIGN
		case 0x3E: return 0x003E;	//GREATER-THAN SIGN
		case 0x3F: return 0x003F;	//QUESTION MARK
		case 0x40: return 0x0040;	//COMMERCIAL AT
		case 0x41: return 0x0041;	//LATIN CAPITAL LETTER A
		case 0x42: return 0x0042;	//LATIN CAPITAL LETTER B
		case 0x43: return 0x0043;	//LATIN CAPITAL LETTER C
		case 0x44: return 0x0044;	//LATIN CAPITAL LETTER D
		case 0x45: return 0x0045;	//LATIN CAPITAL LETTER E
		case 0x46: return 0x0046;	//LATIN CAPITAL LETTER F
		case 0x47: return 0x0047;	//LATIN CAPITAL LETTER G
		case 0x48: return 0x0048;	//LATIN CAPITAL LETTER H
		case 0x49: return 0x0049;	//LATIN CAPITAL LETTER I
		case 0x4A: return 0x004A;	//LATIN CAPITAL LETTER J
		case 0x4B: return 0x004B;	//LATIN CAPITAL LETTER K
		case 0x4C: return 0x004C;	//LATIN CAPITAL LETTER L
		case 0x4D: return 0x004D;	//LATIN CAPITAL LETTER M
		case 0x4E: return 0x004E;	//LATIN CAPITAL LETTER N
		case 0x4F: return 0x004F;	//LATIN CAPITAL LETTER O
		case 0x50: return 0x0050;	//LATIN CAPITAL LETTER P
		case 0x51: return 0x0051;	//LATIN CAPITAL LETTER Q
		case 0x52: return 0x0052;	//LATIN CAPITAL LETTER R
		case 0x53: return 0x0053;	//LATIN CAPITAL LETTER S
		case 0x54: return 0x0054;	//LATIN CAPITAL LETTER T
		case 0x55: return 0x0055;	//LATIN CAPITAL LETTER U
		case 0x56: return 0x0056;	//LATIN CAPITAL LETTER V
		case 0x57: return 0x0057;	//LATIN CAPITAL LETTER W
		case 0x58: return 0x0058;	//LATIN CAPITAL LETTER X
		case 0x59: return 0x0059;	//LATIN CAPITAL LETTER Y
		case 0x5A: return 0x005A;	//LATIN CAPITAL LETTER Z
		case 0x5B: return 0x005B;	//LEFT SQUARE BRACKET
		case 0x5C: return 0x005C;	//REVERSE SOLIDUS
		case 0x5D: return 0x005D;	//RIGHT SQUARE BRACKET
		case 0x5E: return 0x005E;	//CIRCUMFLEX ACCENT
		case 0x5F: return 0x005F;	//LOW LINE
		case 0x60: return 0x0060;	//GRAVE ACCENT
		case 0x61: return 0x0061;	//LATIN SMALL LETTER A
		case 0x62: return 0x0062;	//LATIN SMALL LETTER B
		case 0x63: return 0x0063;	//LATIN SMALL LETTER C
		case 0x64: return 0x0064;	//LATIN SMALL LETTER D
		case 0x65: return 0x0065;	//LATIN SMALL LETTER E
		case 0x66: return 0x0066;	//LATIN SMALL LETTER F
		case 0x67: return 0x0067;	//LATIN SMALL LETTER G
		case 0x68: return 0x0068;	//LATIN SMALL LETTER H
		case 0x69: return 0x0069;	//LATIN SMALL LETTER I
		case 0x6A: return 0x006A;	//LATIN SMALL LETTER J
		case 0x6B: return 0x006B;	//LATIN SMALL LETTER K
		case 0x6C: return 0x006C;	//LATIN SMALL LETTER L
		case 0x6D: return 0x006D;	//LATIN SMALL LETTER M
		case 0x6E: return 0x006E;	//LATIN SMALL LETTER N
		case 0x6F: return 0x006F;	//LATIN SMALL LETTER O
		case 0x70: return 0x0070;	//LATIN SMALL LETTER P
		case 0x71: return 0x0071;	//LATIN SMALL LETTER Q
		case 0x72: return 0x0072;	//LATIN SMALL LETTER R
		case 0x73: return 0x0073;	//LATIN SMALL LETTER S
		case 0x74: return 0x0074;	//LATIN SMALL LETTER T
		case 0x75: return 0x0075;	//LATIN SMALL LETTER U
		case 0x76: return 0x0076;	//LATIN SMALL LETTER V
		case 0x77: return 0x0077;	//LATIN SMALL LETTER W
		case 0x78: return 0x0078;	//LATIN SMALL LETTER X
		case 0x79: return 0x0079;	//LATIN SMALL LETTER Y
		case 0x7A: return 0x007A;	//LATIN SMALL LETTER Z
		case 0x7B: return 0x007B;	//LEFT CURLY BRACKET
		case 0x7C: return 0x007C;	//VERTICAL LINE
		case 0x7D: return 0x007D;	//RIGHT CURLY BRACKET
		case 0x7E: return 0x007E;	//TILDE
		case 0x7F: return 0x007F;	//DELETE
		case 0x80: return 0x20AC;	//EURO SIGN
		//case 0x81: //UNDEFINED
		case 0x82: return 0x201A;	//SINGLE LOW-9 QUOTATION MARK
		case 0x83: return 0x0192;	//LATIN SMALL LETTER F WITH HOOK
		case 0x84: return 0x201E;	//DOUBLE LOW-9 QUOTATION MARK
		case 0x85: return 0x2026;	//HORIZONTAL ELLIPSIS
		case 0x86: return 0x2020;	//DAGGER
		case 0x87: return 0x2021;	//DOUBLE DAGGER
		case 0x88: return 0x02C6;	//MODIFIER LETTER CIRCUMFLEX ACCENT
		case 0x89: return 0x2030;	//PER MILLE SIGN
		case 0x8A: return 0x0160;	//LATIN CAPITAL LETTER S WITH CARON
		case 0x8B: return 0x2039;	//SINGLE LEFT-POINTING ANGLE QUOTATION MARK
		case 0x8C: return 0x0152;	//LATIN CAPITAL LIGATURE OE
		//case 0x8D: //UNDEFINED
		case 0x8E: return 0x017D;	//LATIN CAPITAL LETTER Z WITH CARON
		//case 0x8F: //UNDEFINED
		//case 0x90: //UNDEFINED
		case 0x91: return 0x2018;	//LEFT SINGLE QUOTATION MARK
		case 0x92: return 0x2019;	//RIGHT SINGLE QUOTATION MARK
		case 0x93: return 0x201C;	//LEFT DOUBLE QUOTATION MARK
		case 0x94: return 0x201D;	//RIGHT DOUBLE QUOTATION MARK
		case 0x95: return 0x2022;	//BULLET
		case 0x96: return 0x2013;	//EN DASH
		case 0x97: return 0x2014;	//EM DASH
		case 0x98: return 0x02DC;	//SMALL TILDE
		case 0x99: return 0x2122;	//TRADE MARK SIGN
		case 0x9A: return 0x0161;	//LATIN SMALL LETTER S WITH CARON
		case 0x9B: return 0x203A;	//SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
		case 0x9C: return 0x0153;	//LATIN SMALL LIGATURE OE
		//case 0x9D: //UNDEFINED
		case 0x9E: return 0x017E;	//LATIN SMALL LETTER Z WITH CARON
		case 0x9F: return 0x0178;	//LATIN CAPITAL LETTER Y WITH DIAERESIS
		case 0xA0: return 0x00A0;	//NO-BREAK SPACE
		case 0xA1: return 0x00A1;	//INVERTED EXCLAMATION MARK
		case 0xA2: return 0x00A2;	//CENT SIGN
		case 0xA3: return 0x00A3;	//POUND SIGN
		case 0xA4: return 0x00A4;	//CURRENCY SIGN
		case 0xA5: return 0x00A5;	//YEN SIGN
		case 0xA6: return 0x00A6;	//BROKEN BAR
		case 0xA7: return 0x00A7;	//SECTION SIGN
		case 0xA8: return 0x00A8;	//DIAERESIS
		case 0xA9: return 0x00A9;	//COPYRIGHT SIGN
		case 0xAA: return 0x00AA;	//FEMININE ORDINAL INDICATOR
		case 0xAB: return 0x00AB;	//LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
		case 0xAC: return 0x00AC;	//NOT SIGN
		case 0xAD: return 0x00AD;	//SOFT HYPHEN
		case 0xAE: return 0x00AE;	//REGISTERED SIGN
		case 0xAF: return 0x00AF;	//MACRON
		case 0xB0: return 0x00B0;	//DEGREE SIGN
		case 0xB1: return 0x00B1;	//PLUS-MINUS SIGN
		case 0xB2: return 0x00B2;	//SUPERSCRIPT TWO
		case 0xB3: return 0x00B3;	//SUPERSCRIPT THREE
		case 0xB4: return 0x00B4;	//ACUTE ACCENT
		case 0xB5: return 0x00B5;	//MICRO SIGN
		case 0xB6: return 0x00B6;	//PILCROW SIGN
		case 0xB7: return 0x00B7;	//MIDDLE DOT
		case 0xB8: return 0x00B8;	//CEDILLA
		case 0xB9: return 0x00B9;	//SUPERSCRIPT ONE
		case 0xBA: return 0x00BA;	//MASCULINE ORDINAL INDICATOR
		case 0xBB: return 0x00BB;	//RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
		case 0xBC: return 0x00BC;	//VULGAR FRACTION ONE QUARTER
		case 0xBD: return 0x00BD;	//VULGAR FRACTION ONE HALF
		case 0xBE: return 0x00BE;	//VULGAR FRACTION THREE QUARTERS
		case 0xBF: return 0x00BF;	//INVERTED QUESTION MARK
		case 0xC0: return 0x00C0;	//LATIN CAPITAL LETTER A WITH GRAVE
		case 0xC1: return 0x00C1;	//LATIN CAPITAL LETTER A WITH ACUTE
		case 0xC2: return 0x00C2;	//LATIN CAPITAL LETTER A WITH CIRCUMFLEX
		case 0xC3: return 0x00C3;	//LATIN CAPITAL LETTER A WITH TILDE
		case 0xC4: return 0x00C4;	//LATIN CAPITAL LETTER A WITH DIAERESIS
		case 0xC5: return 0x00C5;	//LATIN CAPITAL LETTER A WITH RING ABOVE
		case 0xC6: return 0x00C6;	//LATIN CAPITAL LETTER AE
		case 0xC7: return 0x00C7;	//LATIN CAPITAL LETTER C WITH CEDILLA
		case 0xC8: return 0x00C8;	//LATIN CAPITAL LETTER E WITH GRAVE
		case 0xC9: return 0x00C9;	//LATIN CAPITAL LETTER E WITH ACUTE
		case 0xCA: return 0x00CA;	//LATIN CAPITAL LETTER E WITH CIRCUMFLEX
		case 0xCB: return 0x00CB;	//LATIN CAPITAL LETTER E WITH DIAERESIS
		case 0xCC: return 0x00CC;	//LATIN CAPITAL LETTER I WITH GRAVE
		case 0xCD: return 0x00CD;	//LATIN CAPITAL LETTER I WITH ACUTE
		case 0xCE: return 0x00CE;	//LATIN CAPITAL LETTER I WITH CIRCUMFLEX
		case 0xCF: return 0x00CF;	//LATIN CAPITAL LETTER I WITH DIAERESIS
		case 0xD0: return 0x00D0;	//LATIN CAPITAL LETTER ETH
		case 0xD1: return 0x00D1;	//LATIN CAPITAL LETTER N WITH TILDE
		case 0xD2: return 0x00D2;	//LATIN CAPITAL LETTER O WITH GRAVE
		case 0xD3: return 0x00D3;	//LATIN CAPITAL LETTER O WITH ACUTE
		case 0xD4: return 0x00D4;	//LATIN CAPITAL LETTER O WITH CIRCUMFLEX
		case 0xD5: return 0x00D5;	//LATIN CAPITAL LETTER O WITH TILDE
		case 0xD6: return 0x00D6;	//LATIN CAPITAL LETTER O WITH DIAERESIS
		case 0xD7: return 0x00D7;	//MULTIPLICATION SIGN
		case 0xD8: return 0x00D8;	//LATIN CAPITAL LETTER O WITH STROKE
		case 0xD9: return 0x00D9;	//LATIN CAPITAL LETTER U WITH GRAVE
		case 0xDA: return 0x00DA;	//LATIN CAPITAL LETTER U WITH ACUTE
		case 0xDB: return 0x00DB;	//LATIN CAPITAL LETTER U WITH CIRCUMFLEX
		case 0xDC: return 0x00DC;	//LATIN CAPITAL LETTER U WITH DIAERESIS
		case 0xDD: return 0x00DD;	//LATIN CAPITAL LETTER Y WITH ACUTE
		case 0xDE: return 0x00DE;	//LATIN CAPITAL LETTER THORN
		case 0xDF: return 0x00DF;	//LATIN SMALL LETTER SHARP S
		case 0xE0: return 0x00E0;	//LATIN SMALL LETTER A WITH GRAVE
		case 0xE1: return 0x00E1;	//LATIN SMALL LETTER A WITH ACUTE
		case 0xE2: return 0x00E2;	//LATIN SMALL LETTER A WITH CIRCUMFLEX
		case 0xE3: return 0x00E3;	//LATIN SMALL LETTER A WITH TILDE
		case 0xE4: return 0x00E4;	//LATIN SMALL LETTER A WITH DIAERESIS
		case 0xE5: return 0x00E5;	//LATIN SMALL LETTER A WITH RING ABOVE
		case 0xE6: return 0x00E6;	//LATIN SMALL LETTER AE
		case 0xE7: return 0x00E7;	//LATIN SMALL LETTER C WITH CEDILLA
		case 0xE8: return 0x00E8;	//LATIN SMALL LETTER E WITH GRAVE
		case 0xE9: return 0x00E9;	//LATIN SMALL LETTER E WITH ACUTE
		case 0xEA: return 0x00EA;	//LATIN SMALL LETTER E WITH CIRCUMFLEX
		case 0xEB: return 0x00EB;	//LATIN SMALL LETTER E WITH DIAERESIS
		case 0xEC: return 0x00EC;	//LATIN SMALL LETTER I WITH GRAVE
		case 0xED: return 0x00ED;	//LATIN SMALL LETTER I WITH ACUTE
		case 0xEE: return 0x00EE;	//LATIN SMALL LETTER I WITH CIRCUMFLEX
		case 0xEF: return 0x00EF;	//LATIN SMALL LETTER I WITH DIAERESIS
		case 0xF0: return 0x00F0;	//LATIN SMALL LETTER ETH
		case 0xF1: return 0x00F1;	//LATIN SMALL LETTER N WITH TILDE
		case 0xF2: return 0x00F2;	//LATIN SMALL LETTER O WITH GRAVE
		case 0xF3: return 0x00F3;	//LATIN SMALL LETTER O WITH ACUTE
		case 0xF4: return 0x00F4;	//LATIN SMALL LETTER O WITH CIRCUMFLEX
		case 0xF5: return 0x00F5;	//LATIN SMALL LETTER O WITH TILDE
		case 0xF6: return 0x00F6;	//LATIN SMALL LETTER O WITH DIAERESIS
		case 0xF7: return 0x00F7;	//DIVISION SIGN
		case 0xF8: return 0x00F8;	//LATIN SMALL LETTER O WITH STROKE
		case 0xF9: return 0x00F9;	//LATIN SMALL LETTER U WITH GRAVE
		case 0xFA: return 0x00FA;	//LATIN SMALL LETTER U WITH ACUTE
		case 0xFB: return 0x00FB;	//LATIN SMALL LETTER U WITH CIRCUMFLEX
		case 0xFC: return 0x00FC;	//LATIN SMALL LETTER U WITH DIAERESIS
		case 0xFD: return 0x00FD;	//LATIN SMALL LETTER Y WITH ACUTE
		case 0xFE: return 0x00FE;	//LATIN SMALL LETTER THORN
		case 0xFF: return 0x00FF;	//LATIN SMALL LETTER Y WITH DIAERESIS
	}

	// Undefined, return a space
	return 0x00A0;

#else
	char  dummy_string1[2];
	WCHAR dummy_string2[2];

	dummy_string1[0] = character;
	dummy_string1[1] = 0;

	// Convert from Latin1 to UTF8
	MultiByteToWideChar(CP_UTF8, 0, dummy_string1, -1, dummy_string2, 2);
	character = dummy_string2[0];

	return character;
#endif
}

//*********************************************************************
void OutputChar(int character, FILE *outFile)
{
	// Convert from Latin1 to UTF8
	character = AsciiToUTF8(character);

	// Ignore leading spaces
	if (s_EatSpaces && (character == ' ' || character == 160))
		return;

	s_EatSpaces = false;

	// Ignore leading spaces
	if (s_LastCharNewLine && (character == ' ' || character == 160))
		return;

	// Ignore duplicate spaces
	if (s_OutputBufferUsed > 0 && s_OutputBuffer[s_OutputBufferUsed-1] == ' ' && character == ' ')
		return;

	if (s_OutputBufferUsed == 4090)
	{
		CheckForUnhandledAndFlush(outFile);
		AddChar(character);
		s_LastCharNewLine = (character == '\n');
		return;
	}

	if (character != '\n')
	{
		AddChar(character);
		s_LastCharNewLine = false;
	}
	else
	{
		if (s_LastCharNewLine)
			return;

		s_OutputBuffer[s_OutputBufferUsed] = character;
		s_OutputBufferUsed++;
		CheckForUnhandledAndFlush(outFile);
		s_LastCharNewLine = true;
	}
}

//*********************************************************************
void OutputString(const char *string, FILE *outFile)
{
	size_t i, len = strlen(string);

	for (i = 0; i < len; i++)
		OutputChar(string[i], outFile);
}

//*********************************************************************
PARSING_MODE ParseTag(const char *tagBuffer, int tagSize, FILE *outFile)
{
	// "<a href="
	if (tagSize > 3 && strncmp(tagBuffer, "<a href=", 8) == 0)
	{
		assert(!s_InAnchor);
		OutputString(tagBuffer, outFile);
		Flush(outFile);
		s_InAnchor = true;
		return PARSING_COPY;
	}

	// "</a>"
	if (tagSize == 4 && strncmp(tagBuffer, "</a>", 4) == 0)
	{
		if (s_InAnchor)
		{
			OutputString(tagBuffer, outFile);
			Flush(outFile);
			s_InAnchor = false;
		}

		return PARSING_COPY;
	}

	// "<p "
	if (tagSize > 3 && strncmp(tagBuffer, "<p ", 3) == 0)
	{
		OutputString("<p> ", outFile);
		s_currentIndent = 0.0f;

		const char *cursor3 = tagBuffer;

		while(cursor3)
		{
			// Handle indentation specified by style='margin-left:
			const char *indentTag = "margin-left:";
			const char *cursor = strstr(cursor3, indentTag);

			if (cursor == NULL)
			{
				// Handle indentation specified by style='text-align:justify;text-indent:
				indentTag = "text-indent:";
				cursor = strstr(cursor3, indentTag);
			}

			if (cursor != NULL)
			{
				float value = 0;
				bool foundDecimal = false;
				float decimalValue = 0.1f;

				// skip over "style='margin-left:"
				cursor += strlen(indentTag);

				// Might be ".75in" or ".75in" or "58.0pt" or 
				const char *cursor2 = cursor;

				while(*cursor2 != '>' && *cursor2 != ';' && *cursor2 != '\'')
				{
					if (cursor2[0] == 'p' && cursor2[1] == 't')
					{
						// assumes 72 points per inch
						value /= 72.0f;
						cursor2 += 2;
						break;
					}

					if (cursor2[0] == 'i' && cursor2[1] == 'n')
					{
						cursor2 += 2;
						break;
					}

					if (*cursor2 == '.')
					{
						foundDecimal = true;
					}
					else if (*cursor2 >= '0' && *cursor2 <= '9')
					{
						if (foundDecimal)
						{
							value += (*cursor2 - '0') * decimalValue;
							decimalValue *= 0.1f;
						}
						else
						{
							value = value * 10 + (*cursor2 - '0');
						}
					}

					cursor2++;
				}

				s_currentIndent = value * 72;
				cursor = cursor2;
			}

			cursor3 = cursor;
		}

		// We use an image placeholder since smf will strip leading spaces
		// and does not support tabs
		if (s_currentIndent >= 0.1f)
		{
			char buf[256];
			sprintf_s(buf, 256, "<img src=invisible.tga width=%.1f height=1>", s_currentIndent);
			OutputString(buf, outFile);
		}

		s_EatSpaces = true;
		return PARSING_COPY;
	}

	// "<p>"
	if (tagSize == 3 && strncmp(tagBuffer, "<p>", 3) == 0)
	{
		OutputString("<p> ", outFile);
		s_currentIndent = 0.0f;
		s_EatSpaces = true;
		return PARSING_COPY;
	}

	// "</p>"
	if (tagSize == 4 && strncmp(tagBuffer, "</p>", 4) == 0)
	{
		s_currentIndent = 0.0f;

		// Added a space before "</p>" since that appears to work around a bug
		// in the SMF formatting where an empty line would be added.
		OutputString(" </p>\n", outFile);
		return PARSING_COPY;

	}

	// "<b "
	if (tagSize > 3 && strncmp(tagBuffer, "<b ", 3) == 0)
	{
		OutputString("<b>", outFile);
		return PARSING_COPY;
	}

	// "<b>"
	if (tagSize == 3 && strncmp(tagBuffer, "<b>", 3) == 0)
	{
		OutputString(tagBuffer, outFile);
		return PARSING_COPY;
	}

	// "</b>"
	if (tagSize == 4 && strncmp(tagBuffer, "</b>", 4) == 0)
	{
		OutputString(tagBuffer, outFile);
		return PARSING_COPY;
	}

	// <title>
	if (tagSize == 7 && strncmp(tagBuffer, "<title>", 7) == 0)
	{
		strcpy_s(s_IgnoreTag, 256, "</title>");
		return PARSING_IGNORE_TAG;
	}

	// "<br>"
	if (tagSize == 4 && strncmp(tagBuffer, "<br>", 4) == 0)
	{
		// The non-breaking space is here to work around an SMF parsing bug
		// where "<p>\n<br>" will not result in an empty line
		OutputString("\n&nbsp;<br>", outFile);
		s_currentIndent = 0.0f;
		s_EatSpaces = true;
		return PARSING_COPY;
	}

	// "<span >"
	if (tagSize > 6 && strncmp(tagBuffer, "<span ", 6) == 0)
	{
		// Handle indentation specified by style='margin-left:
		const char *tabTag = "mso-tab-count:";
		const char *cursor = strstr(tagBuffer, tabTag);

		if (cursor != NULL)
		{
			int tabCount = 0;

			// skip over tabtag
			cursor += strlen(tabTag);

			tabCount = atoi(cursor);

			float spacing = tabCount * 0.5f * 72;

			// We use an image placeholder since smf will strip leading spaces
			// and does not support tabs
			if (spacing >= 0.1f)
			{
				char buf[256];
				sprintf_s(buf, 256, "<img src=invisible.tga width=%.1f height=1>", spacing);
				OutputString(buf, outFile);
			}

			s_EatSpaces = true;
		}

		return PARSING_COPY;
	}
	
	return PARSING_COPY;
}

//*********************************************************************
PARSING_MODE ParseSpecial(const char *tagBuffer, int tagSize, FILE *outFile)
{
	// Change &quot; to "
	if (strcmp(tagBuffer, "&quot;") == 0)
	{
		OutputString("\"", outFile);
		return PARSING_COPY;
	}

	// These are handled propery by smf
	if (strcmp(tagBuffer, "&nbsp;") == 0 ||
		strcmp(tagBuffer, "&lt;") == 0 ||
		strcmp(tagBuffer, "&gt;") == 0 ||
		strcmp(tagBuffer, "&amp;") == 0 ||
		strcmp(tagBuffer, "&#37;") == 0)
	{
		OutputString(tagBuffer, outFile);
		return PARSING_COPY;
	}

	return PARSING_COPY;
}

//*********************************************************************
void Parse(FILE *inFile, FILE *outFile)
{
	int character;
	char tagBuffer[2048];
	int tagSize = 0;
	PARSING_MODE currentMode = PARSING_COPY;

	while((character = fgetc(inFile)) != EOF)
	{
		if (currentMode == PARSING_COPY)
		{
			if (character == '<')
			{
				currentMode = PARSING_LOAD_TAG;
				tagSize = 0;
			}
			else if (character == '&')
			{
				currentMode = PARSING_LOAD_SPECIAL;
				tagSize = 0;
			}
			else
			{
				if (character == '\n')
					character = ' ';

				OutputChar(character, outFile);
			}

		}

		if (currentMode == PARSING_IGNORE_TAG)
		{
			if (character == '<')
			{
				currentMode = PARSING_IGNORE_TAG2;
				tagSize = 0;
			}
		}
		if (currentMode == PARSING_IGNORE_TAG2)
		{
			tagBuffer[tagSize] = character;
			tagSize++;
			assert(tagSize < 2048);

			if (character == '>')
			{
				tagBuffer[tagSize] = 0;

				if (strcmp(tagBuffer, s_IgnoreTag) == 0)
				{
					currentMode = PARSING_COPY;
				}
				else
				{
					currentMode = PARSING_IGNORE_TAG;
				}
			}
		}

		// Load and parse tags
		if (currentMode == PARSING_LOAD_TAG)
		{
			if (character == '\n')
				character = ' ';

			tagBuffer[tagSize] = character;
			tagSize++;
			assert(tagSize < 2048);

			// Look for "<!--" which starts a comment
			if (tagSize == 4 && strncmp(tagBuffer, "<!--", 4) == 0)
			{
				currentMode = PARSING_LOAD_COMMENT;
				tagSize = 0;
			}

			// Look for ">" which ends a tag
			if (character == '>')
			{
				// NULL terminate the tag buffer
				tagBuffer[tagSize] = 0;

				currentMode = ParseTag(tagBuffer, tagSize, outFile);
			}
		}

		// Load and parse special characters
		if (currentMode == PARSING_LOAD_SPECIAL)
		{
			tagBuffer[tagSize] = character;
			tagSize++;
			assert(tagSize < 2048);

			// Look for ";" which ends a special character
			if (character == ';')
			{
				// NULL terminate the tag buffer
				tagBuffer[tagSize] = 0;

				currentMode = ParseSpecial(tagBuffer, tagSize, outFile);
			}
		}

		// Ignore comments
		if (currentMode == PARSING_LOAD_COMMENT)
		{
			if (tagSize < 3)
			{
				tagBuffer[tagSize] = character;
				tagSize++;
			}
			else
			{
				tagBuffer[0] = tagBuffer[1];
				tagBuffer[1] = tagBuffer[2];
				tagBuffer[2] = character;
			}

			if (tagSize == 3 && strncmp(tagBuffer, "-->", 3) == 0)
			{
				currentMode = PARSING_COPY;
			}
		}
	}

}

//*********************************************************************
int main(int argc, char* argv[])
{
	char *outputFilename = NULL;

	// Check Arguments
	if (argc < 2 || argc > 3)
	{
		printf("Usage: %s <input file> [<output file>]\n", argv[0]);
		printf("   Converts HTML files to SMF format.");
		return 1;
	}

	// Open Input File
	FILE *inFile;
	fopen_s(&inFile, argv[1], "rtS");

	if (!inFile)
	{
		printf("Could not open %s for reading\n", argv[1]);
		return 1;
	}

	printf("Opened input file %s\n", argv[1]);

	// Get Output file name
	if (argc < 3)
	{
		size_t len = strlen(argv[1]);
		outputFilename = _strdup(argv[1]);

		if (_stricmp(outputFilename + len - 4, ".htm") == 0)
		{
			strcpy_s(outputFilename + len - 4, 5, ".smf");
		}
		else if (_stricmp(outputFilename + len - 5, ".html") == 0)
		{
			strcpy_s(outputFilename + len - 5, 6, ".smf");
		}
		else
		{
			free(outputFilename);
			fclose(inFile);

			printf("Input file '%s' does not end in '.html' or '.htm'\n", argv[1]);
			return 1;
		}
	}
	else
	{
		outputFilename = argv[2];
	}

	// Open output file
	FILE *outFile;
	fopen_s(&outFile, outputFilename, "wtS, ccs=UTF-8");
	if (!outFile)
	{
		printf("Could not open %s for writing\n", outputFilename);
		return 1;
	}

	printf("Opened output file %s\n", outputFilename);

	if (argc < 2)
	{
		free(outputFilename);
	}

	// Do the work
	printf("Converting\n");
	Parse(inFile, outFile);

	// Close files
	printf("Closing output file %s\n", outputFilename);
	fclose(outFile);

	printf("Closing input file %s\n", argv[1]);
	fclose(inFile);

	return 0;
}

