/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SPRITE_BASE_H__
#define SPRITE_BASE_H__

typedef struct AtlasTex AtlasTex;
typedef struct BasicTexture BasicTexture;
typedef struct Clipper2D Clipper2D;
typedef struct CBox CBox;


#define MAX_STRINGS ( 1000 )
#define MAX_STR_LEN ( 128 )

#define DEFAULT_SCRN_WD 1024
#define DEFAULT_SCRN_HT 768

extern int do_scissor;

typedef enum {
	ZCLIP_NONE,
	ZCLIP_BEHIND,
	ZCLIP_INFRONT,
} ZClipMode;

typedef enum SpriteScalingMode
{
	SPRITE_XY_SCALE,					//Scale X and Y to fit the default screed width/height
	SPRITE_XY_NO_SCALE,					//Do not scale X or Y, use absolute
	SPRITE_X_ONLY_SCALE,				//Scale X, use absolute for Y
	SPRITE_Y_ONLY_SCALE,				//Scale Y, use absolute for X
	SPRITE_X_UNIFORM_SCALE,				//Scale X, use the same scale for Y
	SPRITE_Y_UNIFORM_SCALE,				//Scale Y, use the same scale for X
	SPRITE_XY_SMALLEST_UNIFORM_SCALE,	//Scale X and Y uniformly based on which scale is smaller
} SpriteScalingMode;

typedef enum ScreenScaleOption
{
	SSO_SCALE_NONE = 0,			//Turn off screen scaling for sprites. (Fixed pixel position, Sprite is not stretched.)
	SSO_SCALE_ALL,				//Screen scale sprites position and texture. (Stretched out version of the sprite in the same relative position.)
	SSO_SCALE_POSITION,			//Screen scale sprite position only. (Sprite keeps the same relative position, but the sprite is not stretched.)
	SSO_SCALE_TEXTURE			//Screen scale sprite texture only. (Sprite has a fixed pixel position, but the sprite looks stretched.)
} ScreenScaleOption;

typedef enum HAlign
{
	H_ALIGN_LEFT,
	H_ALIGN_CENTER,
	H_ALIGN_RIGHT
} HAlign;

typedef enum VAlign
{
	V_ALIGN_TOP,
	V_ALIGN_CENTER,
	V_ALIGN_BOTTOM
} VAlign;

// the main sprite functions
void display_sprite_ex(AtlasTex *atex, BasicTexture *btex, float xp, float yp, float zp, float xscale, float yscale, int rgba, int rgba2, int rgba3, int rgba4, float u1, float v1, float u2, float v2, float angle, int additive, Clipper2D *clipper, ScreenScaleOption useScreenScale, HAlign ha, VAlign va );
void display_sprite_immediate_ex(AtlasTex *atex, BasicTexture *btex, float xp, float yp, float zp, float xscale, float yscale, int rgba, int rgba2, int rgba3, int rgba4, float u1, float v1, float u2, float v2,float angle, int additive, Clipper2D *clipper, int useScreenScale );

void spriteZClip(ZClipMode mode, float depth);

int outside_scissor( float x, float y );
void set_scissor_dbg(int x, char* filename, int lineno);
void scissor_dims(int left, int top, int wd, int ht);
#define set_scissor(x) set_scissor_dbg(x, __FILE__, __LINE__)

void calc_scaled_point(float *x, float *y);

void init_display_list();

void interp_rgba(float vec, int src, int dest, int *res);
void calc_scrn_scaling_with_offset(F32 * xsc, F32 * ysc, F32 * xOffset, F32 * yOffset);
void calc_scrn_scaling(float *xsc, float *ysc);
void set_scrn_scaling_disabled(int disabled);	//deprecated, use setSpriteScaleMode
int is_scrn_scaling_disabled();					//deprecated, use getSpriteScaleMode
void setScalingModeAndOption(SpriteScalingMode mode, ScreenScaleOption option);
void setSpriteScaleMode(SpriteScalingMode mode);
void setScreenScaleOption(ScreenScaleOption option);
SpriteScalingMode getSpriteScaleMode(void);
ScreenScaleOption getScreenScaleOption(void);
void calculateSpriteScales(F32 * scale_x, F32 * scale_y);
void calculateTextureScales(F32 * scale_x, F32 * scale_y);
void calculatePositionScales(F32 * scale_x, F32 * scale_y);
//text scaling applies to position and scale. This function checkes the cureent scaling and positioning state.
int isTextScalingRequired(void);

//build collision box based on current SpriteScalingMode
void BuildScaledCBox(CBox * cbox, F32 xp, F32 yp, F32 wd, F32 ht);

int point_cbox_clsn(int x, int y, CBox * c);
int scaled_point_cbox_clsn(int x, int y, CBox * c);

// Is the game in shell mode right now?
int shellMode();

void applyToScreenScalingFactorf(F32 * xScale, F32 * yScale);
void applyPointToScreenScalingFactorf(F32 * xp, F32 * yp);
void reverseToScreenScalingFactorf(F32 * xScale, F32 * yScale);
void reversePointToScreenScalingFactorf(F32 * xp, F32 * yp);
void applyToScreenScalingFactori(int * xScale, int * yScale);
void applyPointToScreenScalingFactori(int * xp, int * yp);
void reverseToScreenScalingFactori(int * xScale, int * yScale);
void reversePointToScreenScalingFactori(int * xp, int * yp);
void getToScreenScalingFactor(float* xScale, float* yScale);

// convenience functions:
Clipper2D *clipperGetCurrent(void);

static INLINEDBG void display_sprite_tex(BasicTexture *tex, float xp, float yp, float zp, float xscale, float yscale, int rgba)
{
	display_sprite_ex(0, tex, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 0, 1, 1, 0, 0, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP);
}

static INLINEDBG void display_sprite(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int rgba)
{
	display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 0, 1, 1, 0, 0, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP);
}

static INLINEDBG void display_sprite_menu(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int rgba)
{
	display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 0, 1, 1, 0, 0, clipperGetCurrent(), SSO_SCALE_NONE, H_ALIGN_LEFT, V_ALIGN_TOP);
}

static INLINEDBG void display_rotated_sprite(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int rgba, float angle, int additive)
{
	display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 0, 1, 1, angle, additive, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP);
}

static INLINEDBG void display_sprite_blend(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int color_ul, int color_ur, int color_lr, int color_ll )
{
 	display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, color_ul, color_ur, color_lr, color_ll, 0, 0, 1, 1, 0, 0, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP); 
}

#define FLIP_HORIZONTAL 1
#define FLIP_VERTICAL   2
#define FLIP_BOTH		(FLIP_HORIZONTAL|FLIP_VERTICAL)

static INLINEDBG void display_spriteUV(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int rgba, float left, float top, float right, float bottom)
{
	display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, left, top, right, bottom, 0, 0, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP);
}

static INLINEDBG void display_spriteFlip(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int rgba, int flip_type )
{
	if( flip_type==FLIP_BOTH )
		display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 1, 1, 0, 0, 0, 0, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP);
	else if( flip_type&FLIP_HORIZONTAL )
		display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 1, 0, 0, 1, 0, 0, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP);
	else if (flip_type&FLIP_VERTICAL )
		display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 1, 1, 0, 0, 0, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP);
	else
		display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 0, 1, 1, 0, 0, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP);
}

static INLINEDBG void display_sprite_UV_4Color(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int rgba1, int rgba2, int rgba3, int rgba4, float left, float top, float right, float bottom)
{
	display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba1, rgba2, rgba3, rgba4, left, top, right, bottom, 0, 0, clipperGetCurrent(), SSO_SCALE_ALL, H_ALIGN_LEFT, V_ALIGN_TOP);
}

void display_spriteFullscreenLetterbox(AtlasTex *spr, float zp, int rgba, F32 *xScaleOutput, F32 *yScaleOutput);

//"positional" indicates that this will use the current screenScaleOption
static INLINEDBG void display_sprite_positional(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int rgba, HAlign ha, VAlign va)
{
	display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 0, 1, 1, 0, 0, clipperGetCurrent(), getScreenScaleOption(), ha, va);
}

//"positional" indicates that this will use the current screenScaleOption
static INLINEDBG void display_sprite_positional_flip(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int rgba, int flip_type, HAlign ha, VAlign va )
{
	if( flip_type==FLIP_BOTH )
		display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 1, 1, 0, 0, 0, 0, clipperGetCurrent(), getScreenScaleOption(), ha, va);
	else if( flip_type&FLIP_HORIZONTAL )
		display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 1, 0, 0, 1, 0, 0, clipperGetCurrent(), getScreenScaleOption(), ha, va);
	else if (flip_type&FLIP_VERTICAL )
		display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 1, 1, 0, 0, 0, clipperGetCurrent(), getScreenScaleOption(), ha, va);
	else
		display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 0, 1, 1, 0, 0, clipperGetCurrent(), getScreenScaleOption(), ha, va);
}

//"positional" indicates that this will use the current screenScaleOption
static INLINEDBG void display_sprite_positional_blend(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int color_ul, int color_ur, int color_lr, int color_ll, HAlign ha, VAlign va )
{
	display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, color_ul, color_ur, color_lr, color_ll, 0, 0, 1, 1, 0, 0, clipperGetCurrent(), getScreenScaleOption(), ha, va); 
}

static INLINEDBG void display_sprite_positional_rotated(AtlasTex * spr, float xp, float yp, float zp, float xscale, float yscale, int rgba, float angle, int additive, HAlign ha, VAlign va )
{
	display_sprite_ex(spr, 0, xp, yp, zp, xscale, yscale, rgba, rgba, rgba, rgba, 0, 0, 1, 1, angle, additive, clipperGetCurrent(), getScreenScaleOption(), ha, va);
}

#endif /* #ifndef SPRITE_BASE_H__ */

/* End of File */
