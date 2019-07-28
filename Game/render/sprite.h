#ifndef _SPRITE_H
#define _SPRITE_H

#include "sprite_base.h"
#include "stdtypes.h"

typedef struct AtlasTex AtlasTex;
typedef struct BasicTexture BasicTexture;

typedef struct DisplaySprite
{
	AtlasTex	*atex;
	BasicTexture *btex;
	float		z, zclip;
	Vec2		ul;
	Vec2		lr;
	int			rgba[4];
	float		u1, v1;
	float		u2, v2;
	float		angle;
	int			scissor_x, scissor_y, scissor_width, scissor_height;
	U32			additive : 1;
	U32			use_scissor : 1;
} DisplaySprite;

void drawSprite(DisplaySprite *sprite);
void drawAllSprites();
bool isDrawingSprites(void);

#endif
