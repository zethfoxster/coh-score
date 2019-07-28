#ifndef TTFONTUTIL_H
#define TTFONTUTIL_H

#include "ArrayOld.h"
#include "uiBox.h"

typedef struct TTDrawContext TTDrawContext;


/**********************************************************************************************
 * Formatted Text
 *	Intended to break long strings into word-wrapped lines.
 */
typedef struct{
	unsigned short* text;
	unsigned int characterCount;
	unsigned int lineWidthRemainder;
	unsigned int lineWidth;
	unsigned int xOffset;
} FormattedLine;

typedef struct{
	unsigned int useless;
	unsigned int useless2;
	unsigned short* sharedTextBuffer;	// Shared by all this formatted text's formatted lines.
	unsigned int bufferSize;
	unsigned int characterCount;
	Array lines;
	// Cachability info:
	float xScale;
	float yScale;
	int fitWidth;
	int fontSize;
} FormattedText;

FormattedText* createFormattedText();
void psudeoDestroyFormattedText(FormattedText* text);
void destroyFormattedText(FormattedText* ft);

FormattedText* formatText(FormattedText* text, TTDrawContext* font, float xScale, float yScale, int fitWidth, int style, char* str, int indent);
FormattedText* formatTextEx(FormattedText* text, TTDrawContext* font, float xScale, float yScale, int* fitWidths, int fitWidthCount, int style, char* str, int indent);

Array* formatTextBlocks(Array* formattedTexts, TTDrawContext* font, float xScale, float yScale, int fitWidth, int* style, char** strings, int stringCount, int indent);
int formattedTextBlockLineCount(Array*);
/*
 **********************************************************************************************/


// I really don't like the insane number of parameters that must be passed to these functions
// It would probably be somewhat better if some less frequently changed parameters were placed in a structure.
/**********************************************************************************************
 * String drawing utilities
 *
 */
enum
{
	// used in PrintBasic*
	CENTER_X	= (1<<0),
	CENTER_Y	= (1<<1),

	// used in higher level text functions (vprnt() & higher)
	NO_MSPRINT	= (1<<2),
	NO_PREPEND	= (1<<3),
	FIT_WIDTH	= (1<<4), // internal use

	// if you want to add more prefixes, check loadMessageTypeData()!
	PREPEND_P	= (1<<5),
	PREPEND_L_P	= (1<<6),
	RIGHT_X		= (1<<7)
};
void printBasic(TTDrawContext*	font, float x, float y, float z, float xScale, float yScale, int center, const char *str, int charsToPrint, int rgba[4]);
void printBasicWide(TTDrawContext*	font, float x, float y, float z, float xScale, float yScale, int center, unsigned short *str, int charsToPrint, int rgba[4]);
void printBasicWideAbsolute(TTDrawContext*	font, float x, float y, float z, float xScale, float yScale, int center, unsigned short *str, int charsToPrint, int rgba[4]);
/*
 * String drawing utilities
 **********************************************************************************************/

/**********************************************************************************************
 * String Dimensions utilties
 */
void strDimensionsBasic(TTDrawContext* font, float xScale, float yScale, const char* str, int charsToPrint, int* stringWidth, int* stringHeight, int* nextGlyphLeft);
void strDimensionsBasicWide(TTDrawContext* font, float xScale, float yScale, unsigned short* str, int charsToPrint, int* stringWidth, int* stringHeight, int* nextGlyphLeft);
void strDimensionsScaledWide(TTDrawContext* font, float xScale, float yScale, unsigned short* str, int charsToPrint, float* stringWidth, float* stringHeight, float* nextGlyphLeft);
/*
 * String Dimensions utilties
 **********************************************************************************************/

//---------------------------------------------------------------------------------------------
// TTTextWrapper
//---------------------------------------------------------------------------------------------
typedef struct TTTextWrapper TTTextWrapper;
typedef struct  
{
	unsigned short* wideText;
	int characterCount;		// How many characters from "text" should be in this line?

	int fitWidth;
	int lineWidth;
	int remainderWidth;
	int lineHeight;
	TTDrawContext* font;
} TTTextLine;

TTTextWrapper* ttTextWrapperCreate(void);
void ttTextWrapperDestroy(TTTextWrapper* obj);

TTTextLine* ttTextLineCreate(void);
void ttTextLineDestroy(TTTextLine* obj);
TTTextLine* ttTextLineCone(TTTextLine* obj);

void ttWrapperSetTextWide(TTTextWrapper* obj, unsigned short* text, unsigned int textLength);
void ttWrapperSetFont(TTTextWrapper* obj, TTDrawContext* font);
TTTextLine* ttGetLine(TTTextWrapper* obj, unsigned int curLineWidth, unsigned int nextLineWidth, float sc);

//-------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------
FormattedText* stFormatText(const char* text, int width, float scale);
void stDrawFormattedText(UIBox bounds, float z, FormattedText* text, float scale);
unsigned int stGetTextWidth(FormattedText* text);
unsigned int stGetTextHeight(FormattedText* text, float scale);

#endif
