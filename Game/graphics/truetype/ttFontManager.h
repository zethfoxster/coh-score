#ifndef TTFONTMANAGER_H
#define TTFONTMANAGER_H

#include "ArrayOld.h"
#include "ttFont.h"

#include "truetype/ttFontBitmap.h"
#include "rt_font.h"

typedef struct AtlasTex AtlasTex;
typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

//----------------------------------------------------------------------
// TTFMCacheElement
//----------------------------------------------------------------------
/* Structure TTFMCacheElement
 *	Stores information on where a cached glyph texture can be found and 
 *	its bounds within the said texture.
 *
 */
typedef struct TTFMCacheElement{
	// WARNING!
	//  Do not move these 3 fields!
	TTCompositeFont* font;				// The font that generated the bitmap.
	TTFontRenderParams renderParams;	// What kind of render params were used to generate this glyph?

	TTFontBitmapInfo bitmapInfo;
	TTFontBitmap *bitmap;				// System memory copy of a bitmap for software rendering

	AtlasTex *tex;
} TTFMCacheElement;
TTFMCacheElement* createTTFMCacheElement();

//----------------------------------------------------------------------
// TTFontManager
//----------------------------------------------------------------------
/* Structure TTFontManager
 *	
 *	It knows how to:
 *		- Cache a glyph's bitmap.
 *		- Retrieve a cached glyph bitmap.
 *	It knows:
 *		- Which fonts it is managing.
 *		- How much texture memory it is allowed to use.	(not implemented)
 *		- How much texture memory it has already used.	(not implemented)
 *		- How often a glyph is used.					(not implemented)
 *		- How discard a glyph.							(not implemented)
 *		- How to store a bitmap as texture.				(Partially implemented)
 *
 *	It does NOT know:
 *		- How to generate a glyph.  (it's up to the font)
 *		- How to draw a texture. (it's up to the draw context)
 *
 *	Todo -
 *		Implement texture packing to avoid texture switches when drawing glyphs.
 */		

typedef struct TTFontManager{
	Array* fonts;		// Fonts under the manager's control
	StashTable glyphCache;	// Stores cache elements.
} TTFontManager;
TTFontManager* createTTFontManager();
void destroyTTFontManager(TTFontManager* manager);

void ttFMAddFont(TTFontManager* manager, TTCompositeFont* font);
void ttFMFreeFont(TTFontManager* manager, TTCompositeFont* font);
void ttFMSetMemoryLimit(TTFontManager* manager, unsigned int memoryLimit);
void ttFMClearCache(TTFontManager* manager);
TTFMCacheElement* ttFMGetCachedTexture(TTCompositeFont* font, TTFontRenderParams* renderParams);
TTFMCacheElement* ttFMGetTexture(TTFontManager* manager, TTCompositeFont* font, TTFontRenderParams* renderParams);
TTFMCacheElement* ttFMAddTexture(TTFontManager* manager, TTCompositeFont* font, TTFontRenderParams* renderParams, TTFontBitmap* bitmap);


#endif
