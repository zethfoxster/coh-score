#ifndef TTFONTCORE_H
#define TTFONTCORE_H

#include "ArrayOld.h"
#include "ft2build.h"
#include <freetype/freetype.h>
typedef struct TTFontManager TTFontManager;
typedef struct TTFontBitmap TTFontBitmap;

//-----------------------------------------------------------------------------------
// True Type module initialization
//-----------------------------------------------------------------------------------
int ttStartup();
void ttShutdown();

//-----------------------------------------------------------------------------------
// TTCompositeFont
//-----------------------------------------------------------------------------------
/*****************************************************************************************
 * TTCompositeFont
 *	Technically, a TTCompositeFont is the only structure that is needed to draw truetype font on-screen.
 *	However, all actual text drawing code only accepts a TTDrawContext so that the following
 * 	can be done automatically:
 * 		- Glyph texture management
 *		- Glyph texture generation
 *
 *	A TTCompositeFont...
 * 		knows how to:
 *		- Generate a bitmap given a character.		
 * 
 *		knows:
 *		- Who its manager is.
 *		- Which fallback fonts to use.
 *		- Which fonts to use when various styles are requested.
 */
typedef struct TTCompositeFont TTCompositeFont;

// TTSimpleFont
//  - a single font file to be assembled into a composite font
typedef struct TTSimpleFont TTSimpleFont;



typedef enum
{
	TTFSS_NONE,				// Just initialized.
	TTFSS_FILE_NOT_PRESENT,	// Previous load attempt failed.
	TTFSS_LOADED,			// Previous load attempt succeeded.
} TTFontSubstitueState;

typedef struct TTFontSubstitue{
	TTFontSubstitueState state;
	TTSimpleFont* font;
} TTFontSubstitue;

typedef enum
{
	TTGGM_DEFAULT,
	TTGGM_INTERPRETER,
	TTGGM_AUTOHINT,
	TTGGM_NOHINT,
	TTGGM_MAX,
} TTGlyphGenMode;

struct TTCompositeFont{
	TTSimpleFont** fallbackFonts;		// How do I deal with glyphs that are not in this font?
	TTFontManager* manager;		// Who keeps track of the generated textures for me?
};

typedef struct FT_FaceRec_*  FT_Face;
typedef struct FT_StreamRec_*    FT_Stream;

struct TTSimpleFont{
	char* filename;				// Which truetype font file is this font tied to?
	FT_Face	font;				// Handle a freetype FT_FACE
	FT_Stream	stream;			// Needs to get freed on destruction

	// Font substitues to use for different font styles.
	TTFontSubstitue bold;
	TTFontSubstitue italics;
	TTFontSubstitue boldItalics;

	Array* excludeCharacters;	// Exclude the following characters when using this font.
	TTGlyphGenMode glyphGenMode;
};

//-----------------------------------------------------------------------------------
// TTFont creation + destruction
//-----------------------------------------------------------------------------------
TTCompositeFont* createTTCompositeFont();
void destroyTTCompositeFont(TTCompositeFont*);

//-----------------------------------------------------------------------------------
// TTFont loading + configuration
//-----------------------------------------------------------------------------------
TTSimpleFont* ttFontLoadCreate(const char* filename, const char *facename);
TTSimpleFont* ttFontLoadCachedFace(const char *filename, int load, const char *facename);
TTSimpleFont* ttFontLoadCached(const char *filename, int load);

void ttFontUnloadCached(const char *fontName);
void ttFontAddFallback(TTCompositeFont* font, TTSimpleFont* fallbackFont);
TTSimpleFont* ttFontGetSubfont(TTCompositeFont* font, unsigned int index);

// Cache of available fonts for enumeration
typedef struct TTFontCacheElement{
	char *name;
	TTSimpleFont *font;
	int referenceCount;
} TTFontCacheElement;
extern TTFontCacheElement **ttFontCache;

//-----------------------------------------------------------------------------------
// TTFont rendering
//-----------------------------------------------------------------------------------
typedef struct TTFontRenderParams{
	//-------------------------------------------------------------------------------------------------------
	// These fields are used to uniquely identify a cached glyph texture.
	unsigned short defaultSize;			// The glyph should be hinted at this size.
	unsigned short unicodeCharacter;	// Always keep this field as the first field used as the UID of a glyph texture.
	unsigned short renderSize;			// Approximately how large should the on-screen glyph be in pixels?
	unsigned int italicize	: 1;
	unsigned int bold		: 1;
	unsigned int outline	: 1;
	unsigned int dropShadow	: 1;
	unsigned int softShadow : 1;
	unsigned int renderToBuffer :1;		// For software rendering (no GL calls)
	unsigned char outlineWidth;
	unsigned char dropShadowXOffset;
	unsigned char dropShadowYOffset;
	unsigned char softShadowSpread;
	unsigned int face;					// For internal use only.  Which Freetype font is used?
	TTCompositeFont* font;				// For internal use only.  Which composite font is used?
} TTFontRenderParams;

TTFontBitmap* ttFontGenerateBitmap(FT_Face face);
FT_Face ttFontRenderGlyphEx(TTSimpleFont* simpleFont, TTFontRenderParams* params);


TTSimpleFont* ttFontGetFontForCharacter(TTCompositeFont* compositeFont, unsigned short unicodeCharacter);
#endif
