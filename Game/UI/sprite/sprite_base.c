 /***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#include "sprite.h"
#include "sprite_list.h"
#include "cmdgame.h"
#include "textureatlas.h"
#include "win_init.h"
#include "uiGame.h"
#include "gfx.h"			// for loadingScreenVisible()
#include "wdwbase.h"
#include <crtdbg.h>
#include "uiClipper.h"
#include "uiBox.h"
#include "ttFont.h"
#include "seqgraphics.h"
#include "texWords.h"
#include "tex.h"
#include "gfxLoadScreens.h"
#include "timing.h"

// from sprite_list.c
extern int sprites_count;

static SpriteScalingMode sSpriteScalingMode = SPRITE_XY_SCALE;
static ScreenScaleOption sScreenScaleOption = SSO_SCALE_ALL;
static F32 sXsc, sYsc, sXoffset, sYoffset;

void init_display_list()
{
	//TTBTClearAll();		// Get rid of all bufferred strings.
	sprites_count = 0;
	spriteListReset();
	texWordsCheckReload();
}

int do_scissor;
UIBox scissorBox;

int outside_scissor( float x, float y )
{
	Clipper2D *pclip = clipperGetCurrent();
	UIBox *pbox = &scissorBox;

	if(pclip)
		pbox = clipperGetBox(pclip);

 	if( !do_scissor && !pclip)
		return FALSE;
	else
	{
		CBox box = {0};

		uiBoxToCBox(pbox, &box);
 		if( scaled_point_cbox_clsn( x, y, &box ) )
			return FALSE;
	}

	return TRUE;
}



void set_scissor_dbg(int x, char* filename, int lineno)
{
	do_scissor = x;

	if(do_scissor)
	{
		// Hook into the clipper stuff.
		UIBox box = {0};

		// Presumably, all code that use this has only one clipper active at any time.
///		assert(clipperGetCount() == 0);
		clipperPush(&scissorBox);
		clipperSetCurrentDebugInfo(filename, lineno);
	}
	else
	{
//		assert(clipperGetCount() == 1 || clipperGetCount() == 0);
		if(clipperGetCount())
			clipperPop();
	}
}

// Modifies the current scissor().
void scissor_dims(int left, int top, int wd, int ht)
{
	// Save off a copy of the specified scissor box.
	// There may not be a valid clipper on the clipper stack yet
	// if the caller called scissor_dims() before set_scissor().
	scissorBox.origin.x = left;
	scissorBox.origin.y = top;
	scissorBox.width = wd;
	scissorBox.height = ht;

	clipperModifyCurrent(&scissorBox);
}

static void convertToTop(F32 * yp, F32 ht, VAlign va)
{
	switch (va)
	{
	case V_ALIGN_TOP:
	default:
		break;
	case V_ALIGN_CENTER:
		*yp -= ht/2.f;
		break;
	case V_ALIGN_BOTTOM:
		*yp -= ht;
		break;
	}
}
static void convertToLeft(F32 * xp, F32 wd, HAlign ha)
{
	switch (ha)
	{
	case H_ALIGN_LEFT:
	default:
		break;
	case H_ALIGN_CENTER:
		*xp -= wd/2.f;
		break;
	case H_ALIGN_RIGHT:
		*xp -= wd;
		break;
	}
}

//top left refers to texture coordinates
static void convertToTopLeft(F32 * xp, F32 * yp, F32 wd, F32 ht, HAlign ha, VAlign va)
{
	convertToLeft(xp, wd, ha);
	convertToTop(yp, ht, va);
}

static void scaleSpritesByScalingMode(F32 * scale_x, F32 * scale_y)
{
	switch (sSpriteScalingMode)
	{
	case SPRITE_X_ONLY_SCALE:
		*scale_y = 1.f;
		break;
	case SPRITE_Y_ONLY_SCALE:
		*scale_x = 1.f;
		break;
	case SPRITE_XY_NO_SCALE:
		*scale_x = 1.f;
		*scale_y = 1.f;
		break;
	case SPRITE_X_UNIFORM_SCALE:
		*scale_y = *scale_x;
		break;
	case SPRITE_Y_UNIFORM_SCALE:
		*scale_x = *scale_y;
		break;
	case SPRITE_XY_SMALLEST_UNIFORM_SCALE:
		*scale_x = MIN(*scale_x, *scale_y);
		*scale_y = *scale_x;
		break;
	case SPRITE_XY_SCALE:
	default:
		//no change in scale
		break;
	}
}

static void scaleTextureScaleByOption(ScreenScaleOption option, F32 * texture_scale_x, F32 * texture_scale_y)
{
	switch (option)
	{
	case SSO_SCALE_POSITION:
	case SSO_SCALE_NONE:
		*texture_scale_x = 1.f;
		*texture_scale_y = 1.f;
		break;
	case SSO_SCALE_ALL:
	case SSO_SCALE_TEXTURE:
	default:
		break;
	}
}

static void scaleTextureScaleByOptionInverse(ScreenScaleOption option, F32 * texture_scale_x, F32 * texture_scale_y)
{
	switch (option)
	{
	case SSO_SCALE_POSITION:
	case SSO_SCALE_NONE:
		break;
	case SSO_SCALE_ALL:
	case SSO_SCALE_TEXTURE:
	default:
		*texture_scale_x = 1.f;
		*texture_scale_y = 1.f;
		break;
	}
}

static void scalePositionScaleByOption(ScreenScaleOption option, F32 * position_scale_x, F32 * position_scale_y)
{
	switch (option)
	{
	case SSO_SCALE_TEXTURE:
	case SSO_SCALE_NONE:
		*position_scale_x = 1.f;
		*position_scale_y = 1.f;
		break;
	case SSO_SCALE_ALL:
	case SSO_SCALE_POSITION:
	default:
		break;
	}
}

static void scalePositionScaleByOptionInverse(ScreenScaleOption option, F32 * position_scale_x, F32 * position_scale_y)
{
	switch (option)
	{
	case SSO_SCALE_TEXTURE:
	case SSO_SCALE_NONE:
		break;
	case SSO_SCALE_ALL:
	case SSO_SCALE_POSITION:
	default:
		*position_scale_x = 1.f;
		*position_scale_y = 1.f;
		break;
	}
}

void calculateSpriteScales(F32 * scale_x, F32 * scale_y)
{
	calc_scrn_scaling(scale_x, scale_y);
	scaleSpritesByScalingMode(scale_x, scale_y);
}

void calculateTextureScales(F32 * scale_x, F32 * scale_y)
{
	calculateSpriteScales(scale_x, scale_y);
	scaleTextureScaleByOptionInverse(getScreenScaleOption(), scale_x, scale_y);
}

void calculatePositionScales(F32 * scale_x, F32 * scale_y)
{
	calculateSpriteScales(scale_x, scale_y);
	scalePositionScaleByOptionInverse(getScreenScaleOption(), scale_x, scale_y);
}

int isTextScalingRequired(void)
{
	SpriteScalingMode ssm = getSpriteScaleMode();
	ScreenScaleOption sso = getScreenScaleOption();

	return ssm != SPRITE_XY_SCALE && (sso == SSO_SCALE_NONE || sso == SSO_SCALE_TEXTURE);
}

static int create_sprite_ex(DisplaySprite *sprite, AtlasTex *atex, BasicTexture *btex, float xp, float yp, float zp, float xscale, float yscale, int rgba, int rgba2, int rgba3, int rgba4, float u1, float v1, float u2, float v2,float angle, int additive, Clipper2D *clipper, ScreenScaleOption useScreenScale, HAlign ha, VAlign va)
{
	float sprite_width, sprite_height;
	int screen_width, screen_height;
	F32 position_scale_x, position_scale_y, texture_scale_x, texture_scale_y;
	int Z_ADJ = 98.9f; // magical?
	int in_shell = shellMode();
	UIBox *glBox = clipperGetGLBox(clipper);
	F32 scaledWd, scaledHt;
	SpriteScalingMode previousScalingMode = sSpriteScalingMode;

	if (atex)
	{
		sprite_width = atex->width;
		sprite_height = atex->height;
		btex = 0;
	}
	else if (btex)
	{
		sprite_width = btex->width;
		sprite_height = btex->height;
	}
	else
		return 0;

	if (xscale == 0 || yscale == 0)
		return 0;

	PERFINFO_AUTO_START("create_sprite_ex", 1);
	
	sSpriteScalingMode = SPRITE_XY_SCALE;
	calc_scrn_scaling(&scaledWd, &scaledHt);
	sSpriteScalingMode = previousScalingMode;

	//now that height is calculated, reset screen scales
	if (in_shell && previousScalingMode != SPRITE_XY_NO_SCALE)
	{
		position_scale_x = scaledWd;
		position_scale_y = scaledHt;
		texture_scale_x = scaledWd;
		texture_scale_y = scaledHt;
		scaleTextureScaleByOption(useScreenScale, &texture_scale_x, &texture_scale_y);
		scalePositionScaleByOption(useScreenScale, &position_scale_x, &position_scale_y);
	}
	else
	{
		position_scale_x = 1.f;
		position_scale_y = 1.f;
		texture_scale_x = 1.f;
		texture_scale_y = 1.f;
	}

	scaleSpritesByScalingMode(&texture_scale_x, &texture_scale_y);

	if (!isLetterBoxMenu())
	{
		windowClientSize(&screen_width, &screen_height);
	}
	else
	{
		screen_height = scaledHt * DEFAULT_SCRN_HT;
	}

	// Scale the z down to a reasonable value so it can be adjusted to negative below.
	sprite->zclip = zp * 0.01f;
	sprite->z = zp * 0.0001f;

	// Adjustment to bring everything into negative z.  Works with the scaling done above.
	sprite->z -= Z_ADJ;

	// width and height scaling
	sprite_width *= xscale * texture_scale_x;
	sprite_height *= yscale * texture_scale_y;

	xp *= position_scale_x;
	yp *= position_scale_y;
	convertToTopLeft(&xp, &yp, sprite_width, sprite_height, ha, va);

	// x positions.
	sprite->ul[0] = xp;
	sprite->lr[0] = sprite->ul[0] + sprite_width;

	// y positions.  Flip the y axis for sprites
	sprite->ul[1] = screen_height - yp;
	sprite->lr[1] = sprite->ul[1] - sprite_height;

	// uv coords.
	sprite->u1 = u1;
	sprite->v1 = v1;
	sprite->u2 = u2;
	sprite->v2 = v2;

	// Test clipper rejection
	if (!clipperTestValuesGLSpace(clipper, sprite->ul, sprite->lr))
	{
		PERFINFO_AUTO_STOP();
		return 0;
	}

	// Everything else.
	sprite->atex = atex;
	sprite->btex = btex;
	sprite->rgba[0] = rgba;
	sprite->rgba[1] = rgba2;
	sprite->rgba[2] = rgba3;
	sprite->rgba[3] = rgba4;
	sprite->angle = angle;
	sprite->additive = additive;

	if (glBox)
	{
		sprite->scissor_x = glBox->x;
		sprite->scissor_y = glBox->y;
		sprite->scissor_width = glBox->width;
		sprite->scissor_height = glBox->height;
		sprite->use_scissor = 1;
	}
	else
	{
		sprite->use_scissor = 0;
	}
	
	PERFINFO_AUTO_STOP();

	return 1;
}

void display_sprite_immediate_ex(AtlasTex *atex, BasicTexture *btex, float xp, float yp, float zp, float xscale, float yscale, int rgba, int rgba2, int rgba3, int rgba4, float u1, float v1, float u2, float v2,float angle, int additive, Clipper2D *clipper, int useScreenScale )
{
	DisplaySprite sprite;

	PERFINFO_AUTO_START("display_sprite_immediate_ex", 1);

	if (create_sprite_ex(&sprite, atex, btex, xp, yp, zp, xscale, yscale, rgba, rgba2, rgba3, rgba4, u1, v1, u2, v2, angle, additive, clipper, useScreenScale, H_ALIGN_LEFT, V_ALIGN_TOP))
	{
		drawSprite(&sprite);
	}

	PERFINFO_AUTO_STOP();
}

void display_sprite_ex(AtlasTex *atex, BasicTexture *btex, float xp, float yp, float zp, float xscale, float yscale, int rgba, int rgba2, int rgba3, int rgba4, float u1, float v1, float u2, float v2,float angle, int additive, Clipper2D *clipper, ScreenScaleOption useScreenScale, HAlign ha, VAlign va )
{
	DisplaySprite sprite, *sprite2;
	
	PERFINFO_AUTO_START("display_sprite_ex", 1);

	if (create_sprite_ex(&sprite, atex, btex, xp, yp, zp, xscale, yscale, rgba, rgba2, rgba3, rgba4, u1, v1, u2, v2, angle, additive, clipper, useScreenScale, ha, va))
	{
		// create the sprite in order in the list and copy our data over
		sprite2 = createDisplaySprite(zp);

		*sprite2 = sprite;
	}
	
	PERFINFO_AUTO_STOP();
}

void calc_scrn_scaling_with_offset(F32 * xsc, F32 * ysc, F32 * xOffset, F32 * yOffset)
{
	int wd, ht;
	devassert(xsc && ysc);

	windowClientSize(&wd, &ht);

	//initially zero offsets
	if (xOffset)
	{
		*xOffset = 0.f;
	}
	if (yOffset)
	{
		*yOffset = 0.f;
	}

	//need to use these to caculate offsets
	*xsc = (float) (wd) / (float) DEFAULT_SCRN_WD;
	*ysc = (float) (ht) / (float) DEFAULT_SCRN_HT;

	if (isLetterBoxMenu())
	{

		if (*xsc > *ysc)
		{
			*xsc = *ysc;
			if (xOffset)
			{
				*xOffset = (wd - (*xsc*DEFAULT_SCRN_WD)) / 2.f;  //center the x
			}
			if (yOffset)
			{
				*yOffset = 0.f;
			}
		}
		else
		{
			*ysc = *xsc;
			if (xOffset)
			{
				*xOffset = 0.f;
			}
			if (yOffset)
			{
				*yOffset = (ht - (*ysc*DEFAULT_SCRN_HT)) /2.f;  //center the y
			}
		}
	}

	if ( sSpriteScalingMode == SPRITE_XY_NO_SCALE || (!loadingScreenVisible() && !shell_menu()))
	{
		*xsc = *ysc = 1.f;
	}
}

/* Function calc_scrn_scaling()
 *	Returns the scaling required to go from virtual screen size to real screen size.
 *	Note that this function depends on game_state.screen_x + screen_y to always have
 *	the correct values.
 *
 */
void calc_scrn_scaling(float *xsc, float *ysc)
{
	calc_scrn_scaling_with_offset( xsc, ysc, NULL, NULL);
}

//
//
void set_scrn_scaling_disabled(int disabled)
{
	setSpriteScaleMode(disabled ? SPRITE_XY_NO_SCALE : SPRITE_XY_SCALE);
}

int is_scrn_scaling_disabled(void)
{
	return sSpriteScalingMode == SPRITE_XY_NO_SCALE;
}
void setScalingModeAndOption(SpriteScalingMode mode, ScreenScaleOption option)
{
	setSpriteScaleMode(mode);
	setScreenScaleOption(option);
}
void setSpriteScaleMode(SpriteScalingMode mode)
{
	sSpriteScalingMode = mode;
}
void setScreenScaleOption(ScreenScaleOption option)
{
	sScreenScaleOption = option;
}
SpriteScalingMode getSpriteScaleMode(void)
{
	return sSpriteScalingMode;
}
ScreenScaleOption getScreenScaleOption(void)
{
	return sScreenScaleOption;
}
void BuildScaledCBox(CBox * cbox, F32 xp, F32 yp, F32 wd, F32 ht)
{
	ScreenScaleOption option = getScreenScaleOption();
	switch (option)
	{
	case SSO_SCALE_NONE:
	case SSO_SCALE_TEXTURE:
		setScreenScaleOption(SSO_SCALE_ALL);
		reverseToScreenScalingFactorf(&xp, &yp);
		reverseToScreenScalingFactorf(&wd, &ht);
		setScreenScaleOption(option);
		break;
	case SSO_SCALE_ALL:
	case SSO_SCALE_POSITION:
	default:
		break;
	}

	BuildCBox(cbox, xp, yp, wd, ht);
}
//
//
void calc_scaled_point(float *x, float *y)
{
	int wd, ht;

	windowClientSize(&wd, &ht);

	if (wd == 0)
		wd = 1;
	if (ht == 0)
		ht = 1;

	if( shell_menu() )
	{
		*x = (*x * DEFAULT_SCRN_WD) / (wd);
		*y = (*y * DEFAULT_SCRN_HT) / (ht);
	}
}

//
//
int point_cbox_clsn(int x, int y, CBox * c)
{
	if (x >= c->lx && x <= c->hx && y >= c->ly && y <= c->hy && ( x != 0 && y != 0 ))
		return TRUE;

	return FALSE;
}

void cboxGetLLUR(CBox* c, PointFloatXY* lowerLeft, PointFloatXY* upperRight)
{
	lowerLeft->x = c->upperLeft.x;
	lowerLeft->y = c->lowerRight.y;
	upperRight->x = c->lowerRight.x;
	upperRight->y = c->upperLeft.y;
}

void cboxSetLLUR(CBox* c, PointFloatXY* lowerLeft, PointFloatXY* upperRight)
{
	c->upperLeft.x = lowerLeft->x;
	c->lowerRight.y = lowerLeft->y;
	c->lowerRight.x = upperRight->x;
	c->upperLeft.y = upperRight->y;
}


//
// Shell is now scaled, coords are constant.
// game scales draw coordinates one time when screen size is changed.
//
int scaled_point_cbox_clsn(int x, int y, CBox * c)
{
	CBox nbox;
	float xsc, ysc;
	int bwd, bht;

 	if (shell_menu() )
	{
		F32 xOffset, yOffset;
		calc_scrn_scaling_with_offset(&xsc, &ysc, &xOffset, &yOffset);
		bwd = CBoxWidth(c);
		bht = CBoxHeight(c);
		BuildCBox(&nbox, c->left*xsc+xOffset, c->top*ysc+yOffset, bwd*xsc, bht*ysc);
		return point_cbox_clsn(x, y, &nbox);
	}
	else
		return point_cbox_clsn(x, y, c);
}

//
//
static int interp(	float vec,			//always 0 - 1.0f
					unsigned int src,   //always less than dest.
					unsigned int dest, unsigned int mask, int shift)
{
	U32 delta;

	if ((src & mask) > (dest & mask))
		delta = (src & mask) - (dest & mask);
	else
		delta = (dest & mask) - (src & mask);

	delta = delta >> shift;
	delta = (U32) ((float) delta * vec);
	delta = (delta & 0xff) << shift;

	if ((src & mask) > (dest & mask))
		src = ((src - delta) & mask);
	else
		src = ((src + delta) & mask);

	return src;
}

//
//
void interp_rgba(float vec, int src, int dest, int *res)
{
	*res = interp(vec, src, dest, 0xff000000, 24)
		| interp(vec, src, dest, 0x00ff0000, 16)
		| interp(vec, src, dest, 0x0000ff00, 8)
		| interp(vec, src, dest, 0x000000ff, 0);
}


int shellMode()
{
	return (shell_menu() || loadingScreenVisible() ) && !doingHeadShot() && !game_state.making_map_images;
}

static void applyToScreenScalingFactorfEx(F32 * x, F32 * y, int applyOffset)
{
	float toScreenXScale;
	float toScreenYScale;

	if(shellMode()){
		F32 xOffset, yOffset;
		calc_scrn_scaling_with_offset(&toScreenXScale, &toScreenYScale, &xOffset, &yOffset);
		if(x && toScreenXScale)
		{
			*x *= toScreenXScale;
			if (applyOffset)
			{
				*x += xOffset;
			}
		}
		if(y && toScreenYScale)
		{
			*y *= toScreenYScale;
			if (applyOffset)
			{
				*y += yOffset;
			}
		}
	}

}
void applyToScreenScalingFactorf(float* xScale, float* yScale)
{
	applyToScreenScalingFactorfEx(xScale, yScale, 0);
}
void applyPointToScreenScalingFactorf(F32 * xp, F32 * yp)
{
	applyToScreenScalingFactorfEx(xp, yp, 1);
}

void reverseToScreenScalingFactorfEx(F32 * x, F32 * y, int applyOffset)
{
	float toScreenXScale;
	float toScreenYScale;

	if(shellMode()){
		F32 xOffset, yOffset;
		calc_scrn_scaling_with_offset(&toScreenXScale, &toScreenYScale, &xOffset, &yOffset);

		// Scale the given integers, but never down to zero.
		if(x && toScreenXScale)
		{
			if (applyOffset)
			{
				*x -= xOffset;
			}
			*x /= toScreenXScale;
		}
		if(y && toScreenYScale)
		{
			if (applyOffset)
			{
				*y -= yOffset;
			}
			*y /= toScreenYScale;
		}
	}
}
void reverseToScreenScalingFactorf(F32 * xScale, F32 * yScale)
{
	reverseToScreenScalingFactorfEx(xScale, yScale, 0);
}
void reversePointToScreenScalingFactorf(F32 * xp, F32 * yp)
{
	reverseToScreenScalingFactorfEx(xp, yp, 1);
}

void applyToScreenScalingFactoriEx(int * x, int * y, int applyOffset)
{
	float toScreenXScale;
	float toScreenYScale;

	if(shellMode()){
		F32 xOffset, yOffset;
		calc_scrn_scaling_with_offset(&toScreenXScale, &toScreenYScale, &xOffset, &yOffset);

		// Scale the given integers, but never down to zero.
		if(x && toScreenXScale)
		{
			*x *= toScreenXScale;
			if (applyOffset)
			{
				*x += xOffset;
			}
		}
		if(y && toScreenYScale)
		{
			*y *= toScreenYScale;
			if (applyOffset)
			{
				*y += yOffset;
			}
		}
	}
}
void applyToScreenScalingFactori(int * xScale, int * yScale)
{
	applyToScreenScalingFactoriEx(xScale, yScale, 0);
}
void applyPointToScreenScalingFactori(int * xp, int * yp)
{
	applyToScreenScalingFactoriEx(xp, yp, 1);
}

void reverseToScreenScalingFactoriEx(int * x, int * y, int applyOffset)
{
	float toScreenXScale;
	float toScreenYScale;

	if(shellMode()){
		F32 xOffset, yOffset;
		calc_scrn_scaling_with_offset(&toScreenXScale, &toScreenYScale, &xOffset, &yOffset);

		// Scale the given integers, but never down to zero.
		if(x && toScreenXScale)
		{
			if (applyOffset)
			{
				*x -= xOffset;
			}
			*x /= toScreenXScale;
		}
		if(y && toScreenYScale)
		{
			if (applyOffset)
			{
				*y -= yOffset;
			}
			*y /= toScreenYScale;
		}
	}
}
void reverseToScreenScalingFactori(int * xScale, int * yScale)
{
	reverseToScreenScalingFactoriEx(xScale, yScale, 0);
}
void reversePointToScreenScalingFactori(int * xp, int * yp)
{
	reverseToScreenScalingFactoriEx(xp, yp, 1);
}

void getToScreenScalingFactor(float* xScale, float* yScale)
{
	float toScreenXScale;
	float toScreenYScale;

	if(shellMode()){
		calc_scrn_scaling(&toScreenXScale, &toScreenYScale);
		if(xScale)
			*xScale = toScreenXScale;
		if(yScale)
			*yScale = toScreenYScale;
	}
	else
	{
		if(xScale)
			*xScale = 1;
		if(yScale)
			*yScale = 1;
	}

}


void display_spriteFullscreenLetterbox(AtlasTex *sprite, float zp, int rgba, F32 *xScaleOutput, F32 *yScaleOutput)
{
	// We have loading screen textures authored at 1:1 that were intended to be stretched to a
	//  fullscreen 4:3.
	static const F32 TARGET_ASPECT = 4.0 / 3.0;
	F32 xScale, yScale;
	F32 xOffset = 0.0f, yOffset = 0.0f;
	int windowWidth, windowHeight;
	int borderColor = rgba & 0x000000ff;

	windowClientSize(&windowWidth, &windowHeight);

	display_sprite_ex(white_tex_atlas, 0, 0, 0, zp - 1, (F32)windowWidth / white_tex->width,
		(F32)windowHeight / white_tex->height, borderColor, borderColor, borderColor, borderColor,
		0, 0, 1, 1, 0, 0, clipperGetCurrent(), 0, H_ALIGN_LEFT, V_ALIGN_TOP);

	if ((F32)windowWidth / windowHeight > TARGET_ASPECT)
	{
		yScale = (float)windowHeight / sprite->height;
		xScale = (yScale * sprite->height / sprite->width) * TARGET_ASPECT;
		xOffset = (windowWidth - xScale * sprite->width) * 0.5f;

		if (yScaleOutput)
			*yScaleOutput = 1;
		if (xScaleOutput)
			*xScaleOutput = TARGET_ASPECT / ((F32)windowWidth / windowHeight);
	}
	else
	{
		xScale = (float)windowWidth / sprite->width;
		yScale = (xScale * sprite->width / sprite->height) / TARGET_ASPECT;
		yOffset = (windowHeight - yScale * sprite->height) * 0.5f;

		if (xScaleOutput)
			*xScaleOutput = 1;
		if (yScaleOutput)
			*yScaleOutput = ((F32)windowWidth / windowHeight) / TARGET_ASPECT;
	}
	display_sprite_ex(sprite, 0, xOffset, yOffset, zp, xScale, yScale, rgba, rgba, rgba, rgba, 0, 0,
		1, 1, 0, 0, clipperGetCurrent(), 0, H_ALIGN_LEFT, V_ALIGN_TOP);
}
