/***************************************************************************
 *     Copyright (c) 2000-2003, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 ***************************************************************************/

#include "sprite.h"
#include "sprite_list.h"
#include "rt_sprite.h"
#include "textureatlas.h"
#include "tex.h"
#include "gfx.h"
#include "gfxLoadScreens.h"
#include "render.h"
#include "cmdgame.h"
#include "file.h"
#include "ogl.h"

#define DEBUG_SPRITE_DRAW_ORDER 0

#define RGBA_R(rgba) ((unsigned char) (((rgba) >> 24) & 0xff))
#define RGBA_G(rgba) ((unsigned char) (((rgba) >> 16) & 0xff))
#define RGBA_B(rgba) ((unsigned char) (((rgba) >> 8)  & 0xff))
#define RGBA_A(rgba) ((unsigned char) (((rgba) >> 0)  & 0xff))

// from sprite_list.c
extern DisplaySprite *sprites;
extern int sprites_count;
extern int sprites_max;


ZClipMode zclip_mode=ZCLIP_NONE;
static F32 zclip_depth = 0;
void spriteZClip(ZClipMode do_zclip, F32 depth)
{
	zclip_mode = do_zclip;
	zclip_depth = depth;
	if (zclip_mode != ZCLIP_NONE)
		zclip_depth = depth;
}


static INLINEDBG void spriteFromDispSprite(RdrSpriteVertex *vertices, DisplaySprite *src)
{
	float u1, v1, u2, v2;
	if (src->atex)
	{
		atlasGetModifiedUVs(src->atex, src->u1, src->v1, &u1, &v1);
		atlasGetModifiedUVs(src->atex, src->u2, src->v2, &u2, &v2);
	}
	else
	{
		u1 = src->u1 * ((float)src->btex->width) / ((float)src->btex->realWidth);
		v1 = src->v1 * ((float)src->btex->height) / ((float)src->btex->realHeight);
		u2 = src->u2 * ((float)src->btex->width) / ((float)src->btex->realWidth);
		v2 = src->v2 * ((float)src->btex->height) / ((float)src->btex->realHeight);
	}


	// increase x first
	vertices[0].point[0] = src->ul[0];
	vertices[0].point[1] = src->ul[1];
	vertices[0].point[2] = src->z;
	vertices[0].rgba[0] = RGBA_R(src->rgba[0]);
	vertices[0].rgba[1] = RGBA_G(src->rgba[0]);
	vertices[0].rgba[2] = RGBA_B(src->rgba[0]);
	vertices[0].rgba[3] = RGBA_A(src->rgba[0]);
	vertices[0].texcoord[0] = u1;
	vertices[0].texcoord[1] = v1;

	vertices[1].point[0] = src->lr[0];
	vertices[1].point[1] = src->ul[1];
	vertices[1].point[2] = src->z;
	vertices[1].rgba[0] = RGBA_R(src->rgba[1]);
	vertices[1].rgba[1] = RGBA_G(src->rgba[1]);
	vertices[1].rgba[2] = RGBA_B(src->rgba[1]);
	vertices[1].rgba[3] = RGBA_A(src->rgba[1]);
	vertices[1].texcoord[0] = u2;
	vertices[1].texcoord[1] = v1;

	vertices[2].point[0] = src->lr[0];
	vertices[2].point[1] = src->lr[1];
	vertices[2].point[2] = src->z;
	vertices[2].rgba[0] = RGBA_R(src->rgba[2]);
	vertices[2].rgba[1] = RGBA_G(src->rgba[2]);
	vertices[2].rgba[2] = RGBA_B(src->rgba[2]);
	vertices[2].rgba[3] = RGBA_A(src->rgba[2]);
	vertices[2].texcoord[0] = u2;
	vertices[2].texcoord[1] = v2;

	vertices[3].point[0] = src->ul[0];
	vertices[3].point[1] = src->lr[1];
	vertices[3].point[2] = src->z;
	vertices[3].rgba[0] = RGBA_R(src->rgba[3]);
	vertices[3].rgba[1] = RGBA_G(src->rgba[3]);
	vertices[3].rgba[2] = RGBA_B(src->rgba[3]);
	vertices[3].rgba[3] = RGBA_A(src->rgba[3]);
	vertices[3].texcoord[0] = u1;
	vertices[3].texcoord[1] = v2;
}

static INLINEDBG void spriteFromRotDispSprite(RdrSpriteVertex *vertices, DisplaySprite *src)
{
	F32		/*z = -1.001f,*/ scale=1;
	Vec3	dv,xys[4],min,max, diff;
	Mat4	m;
	float halfwidth, halfheight;

	float u1, v1, u2, v2;
	if (src->atex)
	{
		atlasGetModifiedUVs(src->atex, src->u1, src->v1, &u1, &v1);
		atlasGetModifiedUVs(src->atex, src->u2, src->v2, &u2, &v2);
	}
	else
	{
		u1 = src->u1 * ((float)src->btex->width) / ((float)src->btex->realWidth);
		v1 = src->v1 * ((float)src->btex->height) / ((float)src->btex->realHeight);
		u2 = src->u2 * ((float)src->btex->width) / ((float)src->btex->realWidth);
		v2 = src->v2 * ((float)src->btex->height) / ((float)src->btex->realHeight);
	}

	copyMat4( unitmat, m );
	yawMat3( src->angle, m );

#if 0
	min[0] = (-src->spr->width/2 ) * src->xscale * txsc;
	min[2] = (-src->spr->height/2) * src->yscale * tysc;
	max[0] = ( src->spr->width/2 ) * src->xscale * txsc;
	max[2] = ( src->spr->height/2) * src->yscale * tysc;

	diff[0] = src->xp*txsc + src->spr->width / 2 * src->xscale * txsc;
	diff[1] = 0.0;
	diff[2] = pkg_height - src->yp*tysc - src->spr->height / 2 * src->yscale * tysc;
#endif

	halfwidth = (src->lr[0] - src->ul[0]) * 0.5f;
	halfheight = (src->ul[1] - src->lr[1]) * 0.5f;

	// centered extents
	min[0] = -halfwidth;
	min[2] = -halfheight;
	max[0] = halfwidth;
	max[2] = halfheight;

	// midpoint
	diff[0] = src->ul[0] + halfwidth;
	diff[1] = 0.0;
	diff[2] = src->lr[1] + halfheight;

	dv[1] = 0;
	dv[0] = min[0];
	dv[2] = min[2];
	mulVecMat4(dv,m,xys[2]);
	dv[0] = max[0];
	dv[2] = max[2];
	mulVecMat4(dv,m,xys[0]);
	dv[0] = min[0];
	dv[2] = max[2];
	mulVecMat4(dv,m,xys[3]);
	dv[0] = max[0];
	dv[2] = min[2];
	mulVecMat4(dv,m,xys[1]);

	addVec3( xys[0], diff, xys[0] );
	addVec3( xys[1], diff, xys[1] );
	addVec3( xys[2], diff, xys[2] );
	addVec3( xys[3], diff, xys[3] );


	// increase x first
	vertices[0].point[0] = xys[3][0];
	vertices[0].point[1] = xys[3][2];
	vertices[0].point[2] = src->z;
	vertices[0].rgba[0] = RGBA_R(src->rgba[0]);
	vertices[0].rgba[1] = RGBA_G(src->rgba[0]);
	vertices[0].rgba[2] = RGBA_B(src->rgba[0]);
	vertices[0].rgba[3] = RGBA_A(src->rgba[0]);
	vertices[0].texcoord[0] = u1;
	vertices[0].texcoord[1] = v1;

	vertices[1].point[0] = xys[0][0];
	vertices[1].point[1] = xys[0][2];
	vertices[1].point[2] = src->z;
	vertices[1].rgba[0] = RGBA_R(src->rgba[1]);
	vertices[1].rgba[1] = RGBA_G(src->rgba[1]);
	vertices[1].rgba[2] = RGBA_B(src->rgba[1]);
	vertices[1].rgba[3] = RGBA_A(src->rgba[1]);
	vertices[1].texcoord[0] = u2;
	vertices[1].texcoord[1] = v1;

	vertices[2].point[0] = xys[1][0];
	vertices[2].point[1] = xys[1][2];
	vertices[2].point[2] = src->z;
	vertices[2].rgba[0] = RGBA_R(src->rgba[2]);
	vertices[2].rgba[1] = RGBA_G(src->rgba[2]);
	vertices[2].rgba[2] = RGBA_B(src->rgba[2]);
	vertices[2].rgba[3] = RGBA_A(src->rgba[2]);
	vertices[2].texcoord[0] = u2;
	vertices[2].texcoord[1] = v2;

	vertices[3].point[0] = xys[2][0];
	vertices[3].point[1] = xys[2][2];
	vertices[3].point[2] = src->z;
	vertices[3].rgba[0] = RGBA_R(src->rgba[3]);
	vertices[3].rgba[1] = RGBA_G(src->rgba[3]);
	vertices[3].rgba[2] = RGBA_B(src->rgba[3]);
	vertices[3].rgba[3] = RGBA_A(src->rgba[3]);
	vertices[3].texcoord[0] = u1;
	vertices[3].texcoord[1] = v2;
}

static INLINEDBG int spriteStatesEqual(RdrSpriteState *state1, RdrSpriteState *state2)
{
	if (state1->additive != state2->additive)
		return 0;

	if (state1->texid != state2->texid)
		return 0;

	if (state1->use_scissor != state2->use_scissor)
		return 0;

	if (state1->use_scissor)
	{
		if (state1->scissor_x != state2->scissor_x)
			return 0;
		if (state1->scissor_y != state2->scissor_y)
			return 0;
		if (state1->scissor_width != state2->scissor_width)
			return 0;
		if (state1->scissor_height != state2->scissor_height)
			return 0;
	}

	return 1;
}

static int spriteCompare(const void *a, const void *b)
{
	const DisplaySprite *dsa = *(const DisplaySprite **)a;
	const DisplaySprite *dsb = *(const DisplaySprite **)b;
	int diff;
	float fdiff;

	// sort by distance first
	if ((fdiff = dsa->zclip - dsb->zclip) != 0.0f)
	{
		if (fdiff < 0.0f)
			return -1;
		else
			return 1;
	}

	// Then group textures together
	if ((diff = dsa->atex - dsb->atex) != 0)
		return diff;

	if ((diff = dsa->btex - dsb->btex) != 0)
		return diff;

	// Then group by use_scissor
	if ((diff = dsa->use_scissor - dsb->use_scissor) != 0)
		return diff;

	// Then group by additive
	if ((diff = dsa->additive - dsb->additive) != 0)
		return diff;

	return 0;
}

static int sprite_sort_scale = 1;
static int spriteCompareAlternate(const void *a, const void *b)
{
	int reverse_scale = (global_state.global_frame_count & 1) ? 1 : -1;
	const DisplaySprite *dsa = *(const DisplaySprite **)a;
	const DisplaySprite *dsb = *(const DisplaySprite **)b;
	int diff;
	float fdiff;

	// sort by distance first
	if ((fdiff = dsa->zclip - dsb->zclip) != 0.0f)
	{
		if (fdiff < 0.0f)
			return -1;
		else
			return 1;
	}

	// Then group textures together
	if ((diff = dsa->atex - dsb->atex) != 0)
		return diff * sprite_sort_scale;

	if ((diff = dsa->btex - dsb->btex) != 0)
		return diff * sprite_sort_scale;

	// Then group by use_scissor
	if ((diff = dsa->use_scissor - dsb->use_scissor) != 0)
		return diff * reverse_scale;

	// Then group by additive
	if ((diff = dsa->additive - dsb->additive) != 0)
		return diff * sprite_sort_scale;

	return 0;
}


static INLINEDBG void getSpriteState(RdrSpriteState *state, DisplaySprite *src)
{
	// this may issue a load command to render thread
	if (src->atex) {
		state->texid = atlasDemandLoadTexture(src->atex);
		state->target = GL_TEXTURE_2D;
	} else {
		state->texid = texDemandLoad(src->btex);
		state->target = src->btex->target;
		if( state->target == GL_TEXTURE_CUBE_MAP_ARB )
			state->target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;
	}

	state->use_scissor = src->use_scissor;
	state->scissor_x = src->scissor_x;
	state->scissor_y = src->scissor_y;
	state->scissor_width = src->scissor_width;
	state->scissor_height = src->scissor_height;
	state->additive = src->additive;
}

static INLINEDBG void convertSprite(RdrSpriteVertex *vertices, DisplaySprite *src)
{
	if (src->angle == 0)
		spriteFromDispSprite(vertices, src);
	else
		spriteFromRotDispSprite(vertices, src);
}

static INLINEDBG int zclipTest(DisplaySprite *src)
{
	if (zclip_mode == ZCLIP_BEHIND)
	{
		if (src->zclip < zclip_depth)
			return 0;
	}
	else if (zclip_mode == ZCLIP_INFRONT)
	{
		if (src->zclip >= zclip_depth)
			return 0;
	}

	return 1;
}

void drawSprite(DisplaySprite *sprite)
{
	int size = sizeof(RdrSpritesCmd) + 4 * sizeof(RdrSpriteVertex);
	RdrSpritesCmd *cmd;
	RdrSpriteVertex *vertices;
	RdrSpriteState rss;

	if (!sprite)
		return;

	// important: call getSpriteState before rdrQueueAlloc since it may request texture load
	//	in render thread and that needs to happen before the drawsprite command gets processed.
	getSpriteState(&rss, sprite);

	cmd = rdrQueueAlloc(DRAWCMD_SPRITE, size);
	vertices = (RdrSpriteVertex *)(cmd+1);
	cmd->sprite_count = 1;
	memcpy( &cmd->state, &rss, sizeof(RdrSpriteState) );
	convertSprite(vertices, sprite);
	rdrQueueSend();
}

static bool g_is_drawing_sprites=false;
bool isDrawingSprites(void)
{
	return g_is_drawing_sprites;
}

extern int green_blobby_fx_id;
void drawAllSprites()
{
	static DisplaySprite	**sprites_visible = 0;
	static RdrSpritesCmd	*sprite_cmds = 0;
	static int				sprites_max = 0;

	RdrSpritesPkg	*pkg;
	int				i, curr, count, num_states, pkg_size, cmds_size, vertices_size;
	DisplaySprite	*src;
	RdrSpriteVertex *dst;

	if (!sprites_count)
		return;

	assert(isProductionMode() || !g_is_drawing_sprites);

	if (g_is_drawing_sprites) // Shouldn't happen
		return;

	g_is_drawing_sprites = true;

	if (sprites_count > sprites_max)
	{
		if (!sprites_max)
			sprites_max = 1;
		sprites_max <<= 1;
		if (sprites_count > sprites_max)
			sprites_max = sprites_count;

		sprites_visible = realloc(sprites_visible, sprites_max * sizeof(*sprites_visible));
		sprite_cmds = realloc(sprite_cmds, sprites_max * sizeof(*sprite_cmds));
	}

	// Test code to see if better sorting will result in faster rendering.
	// This should result in fewer state changes.
	// Not faster right now, but this may be faster if combined with removing the sorting
	// done in createDisplaySprite().
	// This code left here so we can use it to find potential sprite sort order problems.
	if (game_state.spriteSortDebug)
	{
		count = 0;
		for (i = 0; i < sprites_count; i++)
		{
			src = &sprites[getSpriteIndex(i)];
			if (!src)
				continue;

			if (!zclipTest(src))
				continue;

			sprites_visible[count] = src;
			count++;
		}

		if (game_state.spriteSortDebug)
		{
			// Reverse the order outside of distance sort every other frame
			// to highlight potential problems
			sprite_sort_scale = (global_state.global_frame_count & 1) ? 1 : -1;

			qsort(sprites_visible, count, sizeof(sprites_visible[0]), spriteCompareAlternate);
		}
		else
		{
			qsort(sprites_visible, count, sizeof(sprites_visible[0]), spriteCompare);
		}

		num_states = 0;
		for (i = 0; i < count; i++)
		{
			src = sprites_visible[i];

			getSpriteState(&sprite_cmds[num_states].state, src);
			if (num_states == 0 || !spriteStatesEqual(&sprite_cmds[num_states].state, &sprite_cmds[num_states-1].state))
			{
				sprite_cmds[num_states].sprite_count = 0;
				num_states++;
			}

			sprite_cmds[num_states-1].sprite_count++;
		}
	}
   	else
	{
		count = num_states = 0;
		for (i = 0; i < sprites_count; i++)
		{
			src = &sprites[getSpriteIndex(i)];
			if (!src)
				continue;

			if (!zclipTest(src))
				continue;

			sprites_visible[count] = src;

			getSpriteState(&sprite_cmds[num_states].state, src);
			if (num_states == 0 || !spriteStatesEqual(&sprite_cmds[num_states].state, &sprite_cmds[num_states-1].state))
			{
				sprite_cmds[num_states].sprite_count = 0;
				num_states++;
			}

			sprite_cmds[num_states-1].sprite_count++;
			count++;
		}
	}

	pkg_size = sizeof(*pkg);
	cmds_size = num_states * sizeof(RdrSpritesCmd);
	vertices_size = 4 * count * sizeof(RdrSpriteVertex);

	// create the render command
	pkg = rdrQueueAlloc(DRAWCMD_SPRITES, pkg_size + cmds_size + vertices_size);
	pkg->sprite_cmd_count = num_states;
	pkg->vertex_count	= 4 * count;
	pkg->test_depth		= 0;
	pkg->write_depth	= 0;

	if (green_blobby_fx_id) 
	{
		pkg->test_depth = 1;
		pkg->write_depth = 1;
	}

	memcpy(((char *)pkg) + sizeof(*pkg), sprite_cmds, cmds_size);

	curr = pkg_size + cmds_size;
	for (i = 0; i < count; i++)
	{
		src = sprites_visible[i];

		dst = (RdrSpriteVertex*)(((char *)pkg) + curr);
		convertSprite(dst, src);

		// advance pointer
		curr += 4 * sizeof(RdrSpriteVertex);
	}

	rdrQueueSend();

	// TODO : get rid of this
	{
		extern ZClipMode zclip_mode;
		if (zclip_mode == ZCLIP_NONE)
		{
			// It is up to the caller to clear this if they're clipping/multipass rending sprites
			init_display_list();
		}
	}

	g_is_drawing_sprites = false;

#if DEBUG_SPRITE_DRAW_ORDER
	if (game_state.test1)
		game_state.test1 = 0;
#endif
}

/* End of File */
