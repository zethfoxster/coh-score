#ifndef TTFONTDRAW_H
#define TTFONTDRAW_H

#include "truetype/ttFontCore.h" // for TTFontRenderParams

//------------------------------------------------------------------------------
// TTDrawContext
//------------------------------------------------------------------------------
/*****************************************************************************************
 * TTDrawContext
 *	All text drawing must be done using a ttDrawContext.
 *	It knows how to:
 *		- Draw a string on the screen.
 *		- Draw a string in 3D space.	(not implemented)
 *		- Draw a string to a texture.	(not implemented)
 */

typedef struct TTDrawContext{
	TTFontRenderParams renderParams;
	TTCompositeFont* font;
	int dynamic;	// Is this thing going to be continuously scaled?
	int dumpDebugInfo;
} TTDrawContext;

TTDrawContext* createTTDrawContext();
void initTTDrawContext(TTDrawContext* context, int drawSize);


//------------------------------------------------------------------------------
// Generic glyph processing
//------------------------------------------------------------------------------
typedef struct TTTextForEachGlyphParam TTTextForEachGlyphParam;
typedef struct TTFMCacheElement TTFMCacheElement;

// Callback for ttTextForEachGlyph().  This type of functions should return whether to continue processing
// the string.
typedef int (*GlyphHandler)(TTTextForEachGlyphParam* param);
struct TTTextForEachGlyphParam{
	GlyphHandler handler;
	TTFMCacheElement const* cachedTexture;

	unsigned short unicodeCharacter;

	// This is where the glyph would be drawn relative to the origin.
	float top;
	float left;
	float xScale;
	float yScale;
	float nextGlyphLeft;
	int normalized;
};
void ttTextForEachGlyph(TTDrawContext* context, TTTextForEachGlyphParam* param, float x, float y, float xScale, float yScale, unsigned short* text, unsigned int textLength, int allowScaling);


//------------------------------------------------------------------------------
// Basic text drawing
//------------------------------------------------------------------------------
/* ttDrawTextXXX
 *	These functions makes openGL calls to draw the given text immediately.
 */
void ttDrawText2DWithScaling(TTDrawContext* context, float x, float y, float z, float xScale, float yScale, int rgba[4], unsigned short* text, int textLength);
void ttDrawText2DWithScalingSprite(TTDrawContext* context, float x, float y, float z, float xScale, float yScale, int rgba[4], unsigned short* text, int textLength);

// Draws to a software buffer
typedef void (*RenderCallback)(void* userData, float top, float left, float bottom, float right, TTFontBitmap *glyph, int rgba[4]);
void ttDrawText2DWithScalingSoftware(TTDrawContext* context, float x, float y, float xScale, float yScale, int rgba[4], unsigned short* text, int textLength, RenderCallback callback, void *userData);

/* Enum QuadRGBA 
 *	The RGBA quadruple-let used by ttDrawText2DWithScaling() is used to specifiy the color for each
 *	of the 4 corners of the quad the text is drawn on.
 *	
 *	This enum allows access to each of the corners by symbol/name rather than by number.
 */
typedef enum{
	QC_topLeft, 
	QC_topRight, 
	QC_bottomRight,
	QC_bottomLeft, 
} QuadRGBA;


//------------------------------------------------------------------------------
// Basic text measuring
//------------------------------------------------------------------------------
void ttGetStringDimensionsWithScaling(TTDrawContext* context, float xScale, float yScale, unsigned short* text, unsigned int textLength, int* widthOut, int* heightOut, int* nextGlyphLeft, int allowCache);
float ttGetStringWidthNarrow(TTDrawContext* context, float xScale, float yScale, char* text);

void ttSetCacheableFontScaling(float xScale, float yScale);

//------------------------------------------------------------------------------
// Font metrics
//------------------------------------------------------------------------------
float ttGetFontHeight(TTDrawContext* context, float xScale, float yScale);
int ttFontValidateCharacter(TTDrawContext* context, unsigned short character);

//------------------------------------------------------------------------------
// TrueType bufferred text display
//------------------------------------------------------------------------------
/*typedef struct{
	float x;
	float y;
	float z;
	float xScale;
	float yScale;
	int rgba[4];
	unsigned short* text;
	int textLength;
	TTDrawContext drawContext;
} TTBufferredText;

void TTBTStartup();
void TTBTShutdown();

TTBufferredText* TTBTAddString(TTDrawContext* context, float x, float y, float z, float xScale, float yScale, int rgba[4], unsigned short* text, int textLength);
void TTBTClearAll();
void TTBTDisplayAll();
*/

#endif