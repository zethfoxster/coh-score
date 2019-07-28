#ifndef _RT_FONT_H
#define _RT_FONT_H


#include "stdtypes.h"
typedef struct BasicTexture BasicTexture;
typedef struct AtlasTex AtlasTex;
#define TEXT_JUSTIFY 8192.f

typedef struct
{
	U8		Letter;
	S16		Width;
	S16		X,Y;
	S16		Height;
} FontLetter;

typedef struct
{
	char		*Name;
	int			Width;
	int			Height;
	FontLetter	*Letters;
} FontInfo;

#define FONT_MAXCHARS	128

typedef struct
{
	BasicTexture *font_bind;
	int			texid;
	FontLetter	*Letters;
	U8			Indices[FONT_MAXCHARS];
	int			tex_width,tex_height;
} Font;

typedef struct
{
	U8		rgba[4];
	F32		scaleX,scaleY;
	Font	*font;
} FontState;

typedef struct
{
	F32			X,Y;
	int			StringIdx;
	FontState	state;
} FontMessage;

typedef struct
{
	int		count;
} RdrDbgFont;

#ifdef RT_PRIVATE
void rdrSimpleTextDirect(RdrDbgFont *pkg);
void rdrTextDirect(int count, FontMessage *messages, char *str_base);
void rdrSetup2DRenderingDirect();
void rdrFinish2DRenderingDirect();
#endif

#endif
