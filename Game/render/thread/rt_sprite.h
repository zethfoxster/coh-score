#ifndef _RT_SPRITE_H
#define _RT_SPRITE_H

#include "stdtypes.h"

typedef struct
{
	int			texid;
	int			target; // GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_NEGATIVEX_ARB, etc for cubemap face
	int			scissor_x, scissor_y, scissor_width, scissor_height;
	U32			additive : 1;
	U32			use_scissor : 1;
} RdrSpriteState;

typedef struct
{
	Vec3	point;
	U8		rgba[4];
	Vec2	texcoord;
} RdrSpriteVertex;

typedef struct
{
	RdrSpriteState	state;
	int				sprite_count;
} RdrSpritesCmd;

typedef struct
{
	U32				write_depth : 1;
	U32				test_depth : 1;
	int				vertex_count;
	int				sprite_cmd_count;
} RdrSpritesPkg;


#ifdef RT_PRIVATE
void drawAllSpritesDirect(RdrSpritesPkg *pkg);
void drawSpriteDirect(RdrSpritesCmd *cmd);
#endif

#endif
