#include "uiUtil.h"
#include "uiUtilGame.h"
#include "uiGame.h"
#include "wdwbase.h"
#include "mathutil.h"

#include "sprite_base.h"
#include "sprite_text.h"
#include "sprite_font.h"
#include "textureatlas.h"

#include "entVarUpdate.h"   // for TIMESTEP

#include "input.h"
#include "uiInput.h"
#include "ttFontUtil.h"
#include "language/langClientUtil.h"

#include "sound.h"
#include "timing.h"

#include "cmdcommon.h"
#include "strings_opt.h"
#include "cmdgame.h"
#include "gametypes.h"
#include "entity.h"
#include "entPlayer.h"

//
//
void build_cboxxy( AtlasTex *spr, CBox *c, float xsc, float ysc, int xp, int yp )
{
	c->lx = xp;
	c->ly = yp;
	c->hx = c->lx + ( int )( ( float )spr->width * xsc );
	c->hy = c->ly + ( int )( ( float )spr->height * ysc );
}

//
//
void brightenColor( int *color, int amount )
{
	int r = (*color & 0xff000000) >> 24;
	int g = (*color & 0x00ff0000) >> 16;
	int b = (*color & 0x0000ff00) >> 8;

	r += amount;
	if( r > 255 )
		r = 255;
	if( r < 0 )
		r = 0;

	g += amount;
	if( g > 255 )
		g = 255;
	if( g < 0 )
		g = 0;

	b+= amount;
	if( b > 255 )
		b = 255;
	if( b < 0 )
		b = 0;

	*color = (r << 24) + (g << 16) + (b << 8) + 0x000000ff;
}

//
// timer_and_vec:
//
// Take an integer timer and modulate it into a given period.  Create sawtooth wave
// in vec by oscillating it between 0.f & 1.f then back to 0.f within period.
//
void wave_vec_from_timer( int timer, int period, F32 *vec )
{
	int half = period / 2;

	timer = timer % ( period - 1 );
	if( timer >= half )
		*vec = ( float )( ( period - 1 ) - timer ) / ( float )half;
	else *vec = ( float )timer / ( float )half;
}

void drawStdButtonMeat( float x, float y, float z, float wd, float ht, int color )
{
	int clrBack = color & 0xff;
	float tx, sc;

	// sigh, load a trillion textures now
	AtlasTex *highL		= atlasLoadTexture( "GenericButton_rest_L_highlight.tga" );
 	AtlasTex *meatL		= atlasLoadTexture( "GenericButton_rest_L_meat.tga" );
	AtlasTex *shadowL	= atlasLoadTexture( "GenericButton_rest_L_shadow.tga" );
	AtlasTex *dropL		= atlasLoadTexture( "GenericButton_rest_L_dropshadow.tga" );

	AtlasTex *highR		= atlasLoadTexture( "GenericButton_rest_R_highlight.tga" );
	AtlasTex *meatR		= atlasLoadTexture( "GenericButton_rest_R_meat.tga" );
	AtlasTex *shadowR	= atlasLoadTexture( "GenericButton_rest_R_shadow.tga" );
	AtlasTex *dropR		= atlasLoadTexture( "GenericButton_rest_R_dropshadow.tga" );

	AtlasTex *highM		= atlasLoadTexture( "GenericButton_rest_MID_highlight.tga" );
	AtlasTex *meatM		= atlasLoadTexture( "GenericButton_rest_MID_meat.tga" );
	AtlasTex *shadowM	= atlasLoadTexture( "GenericButton_rest_MID_shadow.tga" );
	AtlasTex *dropM		= atlasLoadTexture( "GenericButton_rest_MID_dropshadow.tga" );

	// damage control if values are crazy
	sc = ht/meatM->height;

	if( wd < (meatL->width + meatR->width) * sc )
		wd = (meatL->width + meatR->width) * sc;

	// left side
	display_sprite( dropL,	 x, y,   z, sc, sc, clrBack );
	display_sprite( shadowL, x, y, z+2, sc, sc, color );
	display_sprite( meatL,   x, y, z+1, sc, sc, color     );
	display_sprite( highL,   x, y, z+2, sc, sc, 0xffffff00 | color );

	// right side
	tx = x + wd - meatR->width*sc;
	display_sprite( dropR,	 tx, y,   z, sc, sc, clrBack );
	display_sprite( shadowR, tx, y, z+2, sc, sc, color );
	display_sprite( meatR,   tx, y, z+1, sc, sc, color     );
	display_sprite( highR,   tx, y, z+2, sc, sc, 0xffffff00 | color );

	// middle
	tx = x + meatL->width*sc;
 	display_sprite( dropM,	 tx, y,   z, (wd - (meatL->width + meatR->width)*sc)/dropM->width,   sc, clrBack );
	display_sprite( shadowM, tx, y, z+2, (wd - (meatL->width + meatR->width)*sc)/shadowM->width, sc, color );	
	display_sprite( meatM,   tx, y, z+1, (wd - (meatL->width + meatR->width)*sc)/meatM->width,   sc, color   );
	display_sprite( highM,   tx, y, z+2, (wd - (meatL->width + meatR->width)*sc)/highM->width,   sc, 0xffffff00 | color );
}
//
//
int drawStdButton( float cx, float cy, float z, float wd, float ht, int color, const char *txt, float txtSc, int flags )
{
	AtlasTex *shadowL, *shadowR, *shadowM, *meatL, *meatR, *meatM, *highL, *highR, *highM, *dropL, *dropM, *dropR;
	CBox box;

 	int retVal = D_NONE, txtclr1 = color, txtclr2 = color, c1 = CLR_BLACK, clrBack = CLR_BLACK;
	float tx, ty, sc, depressSc = 1.f, gradient, dsc = 1.f;
	char press[8] = "rest", texName[256];
	int oldColor1 = font_color_get_top(), oldColor2 = font_color_get_bottom();

	static float timer = 0, first = 0;

	float screenScaleX, screenScaleY;

	calc_scrn_scaling(&screenScaleX, &screenScaleY);

	// sigh, load a trillion textures now
	#define LOAD(var, tail) {				\
		STR_COMBINE_BEGIN(texName);			\
		STR_COMBINE_CAT("GenericButton_");	\
		STR_COMBINE_CAT(press);				\
		STR_COMBINE_CAT("_");				\
		STR_COMBINE_CAT(tail);				\
		STR_COMBINE_CAT(".tga");			\
		STR_COMBINE_END();					\
		var = atlasLoadTexture(texName);	\
	}

	LOAD(highL,		"L_highlight");
	LOAD(meatL,		"L_meat");
	LOAD(shadowL,	"L_shadow");
	LOAD(dropL,		"L_dropshadow");

	LOAD(highR,		"R_highlight");
	LOAD(meatR,		"R_meat");
	LOAD(shadowR,	"R_shadow");
	LOAD(dropR,		"R_dropshadow");

	LOAD(highM,		"MID_highlight");
	LOAD(meatM,		"MID_meat");
	LOAD(shadowM,	"MID_shadow");
	LOAD(dropM,		"MID_dropshadow");

	// Don't upscale the textures; we don't want artifacts.
	if (ht > meatL->height * screenScaleY)
		ht = meatL->height * screenScaleY;

	BuildCBox( &box, cx - wd/2, cy - ht/2, wd, ht );
	brightenColor( &color, -20 );

   	txtclr2 = 0xffffffbf;//((color & 0xffffff00) | 0xBF);
 	txtclr1 = 0xffffffbf;

   	font( &game_12 );

 	// deal with mouse state
   	if( mouseCollision( &box ) && !(flags & (BUTTON_LOCKED)) )
	{
		retVal = D_MOUSEOVER;

		c1 = CLR_WHITE;

		timer += TIMESTEP*.15;
		gradient = sinf( timer );

		dsc = (((gradient+1)/2)*.05 + 1);

        color &= 0xffffffdf;

 		txtclr2 = 0xffffffff;
		txtclr1 = 0xffffffff;

  		if( isDown( MS_LEFT ) )
		{
 			depressSc = .95f;
			retVal = D_MOUSEDOWN;
		}
	}

   	if( (flags & (BUTTON_LOCKED)) )
	{
   		color &= 0xffffff00;
 		color |= 0x88;
		clrBack = c1 = 0x88;
		txtclr2 = 0xffffff44;
		txtclr1 = 0xffffff44;
	}

	else if ( (flags & (BUTTON_DIMMED)) )
	{
		color &= 0xffffff00;
 		color |= 0x88;
		clrBack = 0x88;
	}

	font_color( txtclr1, txtclr2 );

 	if( retVal == D_MOUSEDOWN )
		strcpy( press, "press");

	// damage control if values are crazy
	sc = ht/meatM->height;
	sc *= depressSc * screenScaleY;

	if( wd < (meatL->width + meatR->width) * sc )
		wd = (meatL->width + meatR->width) * sc;

	// left side
 	tx = cx - wd*depressSc/2;
	ty = cy - ht*depressSc/2;
	tx *= screenScaleX;
	ty *= screenScaleY;

   	display_sprite_menu( dropL,	 tx, screenScaleY * (cy - ht*depressSc*dsc/2), z, sc, sc*dsc, c1 );
	display_sprite_menu( shadowL, tx, ty, z+2, sc, sc, color );
 	display_sprite_menu( meatL,   tx, ty, z+1, sc, sc, color     );
	display_sprite_menu( highL,   tx, ty, z+2, sc, sc, CLR_WHITE );

	// right side
  	tx = screenScaleX * (cx + wd*depressSc/2) - meatR->width*sc;
   	display_sprite_menu( dropR,	 screenScaleX * (cx + wd*depressSc/2) - meatR->width*sc, screenScaleY * (cy - ht*depressSc*dsc/2), z, sc, sc*dsc, c1 );
	display_sprite_menu( shadowR, tx, ty, z+2, sc, sc, color );
	display_sprite_menu( meatR,   tx, ty, z+1, sc, sc, color     );
	display_sprite_menu( highR,   tx, ty, z+2, sc, sc, CLR_WHITE );

	// middle
	tx = screenScaleX * (cx - wd*depressSc/2) + meatL->width*sc;
   	display_sprite_menu( dropM,	 tx, screenScaleY * (cy - ht*depressSc*dsc/2), z, (screenScaleX * wd*depressSc - (meatL->width + meatR->width)*sc)/dropM->width, dsc*sc, c1 );
   	display_sprite_menu( shadowM, tx, ty, z+2, (screenScaleX * wd*depressSc - (meatL->width + meatR->width)*sc)/shadowM->width, sc, color );	
  	display_sprite_menu( meatM,   tx, ty, z+1, (screenScaleX * wd*depressSc - (meatL->width + meatR->width)*sc)/meatM->width,   sc, color   );
	display_sprite_menu( highM,   tx, ty, z+2, (screenScaleX * wd*depressSc - (meatL->width + meatR->width)*sc)/highM->width,   sc, CLR_WHITE );

  	if( txt )
	{
 		// scale text to fit within button
   		if( str_wd( font_grp, txtSc, txtSc, txt ) > wd-5 )
 			txtSc = str_sc( font_grp, wd-5, txt );

   		cprntEx(cx, cy, z+3, txtSc*depressSc, txtSc*depressSc, (CENTER_X | CENTER_Y), txt );		
	}

	font_color(oldColor1, oldColor2);
 
	if( mouseClickHit( &box, MS_LEFT ) )
	{
		if( (flags & (BUTTON_LOCKED)) )
			return D_MOUSELOCKEDHIT;
		else
			return D_MOUSEHIT;
	}
	else
		return retVal;
	
}

UISkin skinFromMapAndEnt(Entity *e)
{
	UISkin retval;

	switch (game_state.team_area)
	{
		// Hero territory (eg. Paragon City).
		case MAP_TEAM_HEROES_ONLY:
		default:
			retval = UISKIN_HEROES;
			break;

		// Villain territory (eg. Rogue Isles).
		case MAP_TEAM_VILLAINS_ONLY:
			retval = UISKIN_VILLAINS;
			break;

		// Praetoria.
		case MAP_TEAM_PRAETORIANS:
			retval = UISKIN_PRAETORIANS;
			break;

			break;

		// Shared territory (eg. Pocket D). Everyone uses their native
		// currency. Praetorians get included in this 
		case MAP_TEAM_EVERYBODY:
		case MAP_TEAM_PRIMAL_COMMON:
			// Default to hero currency if there's no player to check.
			if (ENT_IS_IN_PRAETORIA(e) || ENT_IS_LEAVING_PRAETORIA(e))
				retval = UISKIN_PRAETORIANS;
			else if (ENT_IS_VILLAIN(e))
				retval = UISKIN_VILLAINS;
			else
				retval = UISKIN_HEROES;
			break;
	}

	return retval;
}
