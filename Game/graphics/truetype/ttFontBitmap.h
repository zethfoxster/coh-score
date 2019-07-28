#ifndef TTFONTBITMAP_H
#define TTFONTBITMAP_H


typedef struct TTFontRenderParams TTFontRenderParams;

//------------------------------------------------------------------------------
// TTFontBitmap
//------------------------------------------------------------------------------
/* Structure TTFontBitMapInfo
*	Used to describe the characteristics of a bitmap
*	so the bitmap can be manipulated.
*/
typedef struct TTFontBitmapInfo{
	// Describes the region that are actually occupied with image data.
	unsigned short width;
	unsigned short height;

	// Offsets that must be applied to the drawing position when drawing this bitmap.
	// These allows letters to be lined up correctly.
	short bitmapTop;
	short bitmapLeft;
	short advanceX;		// How much to advance the drawing position to draw the next letter.
} TTFontBitmapInfo;

typedef struct TTFontBitmap{
	TTFontBitmapInfo bitmapInfo;
	unsigned short* bitmap;
	unsigned char* bitmapRGBA;
} TTFontBitmap;

TTFontBitmap* createTTFontBitmap();
void destroyTTFontBitmap(TTFontBitmap* bitmap);
TTFontBitmap* cloneTTFontBitmap(TTFontBitmap* original);
void ttFontBitmapClear(TTFontBitmap* bitmap);
int ttFontBitmapGetBitmapMemFootprint(TTFontBitmap* bitmap);

//------------------------------------------------------------------------------
// TTFontBitmap utilities
//------------------------------------------------------------------------------
typedef struct{
	int x;
	int y;
	int width;
	int height;
} TTFontRectangle;

typedef union LumAlphaPixel
{
	unsigned short integer;
	struct{
		unsigned char luminance;
		unsigned char alpha;
	};
} LumAlphaPixel;

typedef struct{
	TTFontBitmap* src;
	int srcX;
	int srcY;

	TTFontBitmap* dst;
	int dstX;
	int dstY;

	LumAlphaPixel* srcPixel;
	LumAlphaPixel* dstPixel;
} TTFontBBlitContext;

typedef void (*ttFontBitmapBlitProcessor)(TTFontBBlitContext* context);
void ttFontBitmapBlit(TTFontBitmap* src, TTFontRectangle* srcRegion, TTFontBitmap* dst, TTFontRectangle* dstRegion, ttFontBitmapBlitProcessor proc);


//------------------------------------------------------------------------------
// TTFontBitmap alteration functions
//------------------------------------------------------------------------------
void ttFontBitmapMakeBold(TTFontBitmap** origBitmap, TTFontRenderParams* renderParams);
void ttFontBitmapAddOutline(TTFontBitmap** origBitmap, TTFontRenderParams* renderParams);
void ttFontBitmapAddDropShadow(TTFontBitmap** origBitmap, TTFontRenderParams* renderParams);
void ttFontBitmapDumpToFile(TTFontBitmap** origBitmap, TTFontRenderParams* renderParams, char* filename);

#endif