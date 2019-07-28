
#include "uiSlider.h"
#include "wdwbase.h"		// for cbox
#include "uiInput.h"
#include "uiUtilGame.h"
#include "player.h"			// for playerPtr()
#include "sprite_base.h"	// for calc_scaled_pont
#include "uiGame.h"		// for shell_menu()
#include "uiUtil.h"
#include "sound.h"
#include "textureatlas.h"
#include "cmdgame.h"
#include "uiClipper.h"
#include "uiBox.h"
//------------------------------------------------------------

void drawSliderBackground( float x, float y, float z, float wd, float scale, int flags )
{
	AtlasTex * left;
	AtlasTex * mid;
	AtlasTex * right;

	if (game_state.skin != UISKIN_HEROES || flags&BUTTON_ULTRA )
	{
		left		= atlasLoadTexture( "bodytype_scroll_bar_L_tintable.tga" );
		mid			= atlasLoadTexture( "bodytype_scroll_bar_mid_tintable.tga" );
		right		= atlasLoadTexture( "bodytype_scroll_bar_R_tintable.tga" );
	}
	else
	{
		left		= atlasLoadTexture( "bodytype_scroll_bar_L.tga" );
		mid			= atlasLoadTexture( "bodytype_scroll_bar_mid.tga" );
		right		= atlasLoadTexture( "bodytype_scroll_bar_R.tga" );
	}

	if( flags&BUTTON_ULTRA )
	{
		display_sprite( left, x, y - left->height*scale/2, z, scale, scale, CLR_PARAGON );
		display_sprite( mid, x + left->width*scale, y - mid->height*scale/2, z, (wd - left->width*scale - right->width*scale)/mid->width/2, scale, CLR_PARAGON );
		display_sprite_blend( mid, x + wd/2, y - mid->height*scale/2, z, (wd - left->width*scale - right->width*scale)/mid->width/2, scale, CLR_PARAGON, CLR_YELLOW, CLR_PARAGON, CLR_YELLOW );
		display_sprite( right, x + (wd - right->width*scale), y - right->height*scale/2, z, scale, scale, CLR_YELLOW );
	}
	else
	{
		display_sprite( left, x, y - left->height*scale/2, z, scale, scale, CLR_WHITE );
		display_sprite( mid, x + left->width*scale, y - mid->height*scale/2, z, (wd - left->width*scale - right->width*scale)/mid->width, scale, CLR_WHITE );
		display_sprite( right, x + (wd - right->width*scale), y - right->height*scale/2, z, scale, scale, CLR_WHITE );
	}

}

float doSliderAll( float lx, float ly, float lz, float wd, float xsc, float ysc, float value, float minval, float maxval,
					 int type, int color, int flags, int count, float *tickValues)
{
	AtlasTex * slider	= white_tex_atlas;
	AtlasTex * sliderT;
 	AtlasTex * sliderB	= atlasLoadTexture( "scrollbar_btm.tga" );
	AtlasTex *tick = atlasLoadTexture("compass_facing_top.tga"); 

	CBox box;
	int x,i;
	int locked = !!(flags & BUTTON_LOCKED);
	int oldcolor = color;
	float sliderX, sc = 1.f;

	static int dragOffX, dragged = -1;

	x = lx - wd/2*xsc;
	if( isMenu( MENU_GAME ) )
	{
		UIBox box;
		// first draw the background bar
		sliderT = atlasLoadTexture( "scrollbar_top.tga" );

  		if( flags&BUTTON_ULTRA )
		{
			FrameColors fcolors = { 0 };
			fcolors.topLeft = fcolors.topCenter = fcolors.midLeft = fcolors.midCenter = fcolors.botLeft = fcolors.botCenter = CLR_BLACK;
			fcolors.topRight = fcolors.midRight = fcolors.botRight = CLR_YELLOW;
			fcolors.border = color;
			drawFrameStyleColor( PIX2, R4, x, ly - 6, lz, wd*xsc, 2*R4+2*PIX2*ysc, xsc, fcolors, kFrameStyle_GradientH, FRAMEPARTS_ALL );
		}
		else
			drawFrame( PIX2, R4, x, ly - 6, lz, wd*xsc, 2*R4+2*PIX2*ysc, xsc, color, CLR_BLACK );

		if( minval > -1 )
		{
 			uiBoxDefine( &box,x, ly-6, wd*xsc*((minval+1)/2), 2*R4+2*PIX2*ysc );
			clipperPushRestrict(&box);
			drawFrame( PIX2, R4, x, ly - 6, lz, wd*xsc, 2*R4+2*PIX2*ysc, xsc, CLR_GREY, CLR_GREY );
			clipperPop();
		}

		if( maxval < 1 )
		{
  			uiBoxDefine( &box, x+wd*xsc*((maxval+1)/2), ly-6, wd*xsc*(1.f-((maxval+1)/2)), 2*R4+2*PIX2*ysc );
			clipperPushRestrict(&box);
			drawFrame( PIX2, R4, x, ly - 6, lz, wd*xsc, 2*R4+2*PIX2*ysc, xsc, CLR_GREY, CLR_GREY );
			clipperPop();
		}
	}
	else
	{
		if (game_state.skin != UISKIN_HEROES)
			sliderT		= atlasLoadTexture( "bodytype_scroll_tab_tintable.tga" );
		else
			sliderT		= atlasLoadTexture( "bodytype_scroll_tab.tga" );

		drawSliderBackground( x, ly, lz, wd*xsc, MIN(xsc, ysc), flags );
	}

	//now get the slider position
 	sliderX = value;
	sliderX *= ((wd - sliderT->width)/2) * xsc;

	sliderX += lx;

	// handle mouse collisions
	if( isMenu(MENU_GAME))
		BuildCBox( &box, sliderX - (sliderT->width/2), ly - sliderT->height, sliderT->width, 2*sliderT->height );
	else
		BuildCBox( &box, sliderX - (sliderT->width*sc*xsc/2), ly - sliderT->height*sc*ysc/2, sliderT->width*sc*xsc, sliderT->height*sc*ysc );

	if( !locked && mouseCollision(&box) || dragged == type  )
		sc = 1.2f;

	if( !locked && mouseDownHit( &box, MS_LEFT ) )
	{
		float cx = gMouseInpCur.x;
		float cy;

		if( shell_menu() && !is_scrn_scaling_disabled())
			calc_scaled_point( &cx, &cy );

		dragged = type;
		dragOffX = sliderX - cx;
	}

	if( dragged == type )
	{
		float cx = gMouseInpCur.x;
		float cy;

		color = uiColors.clicked.scrollbar;

		if( shell_menu()&& !is_scrn_scaling_disabled() )
		{
			calc_scaled_point( &cx, &cy );
		}

		sliderX = cx + dragOffX;

		if( locked || !isDown( MS_LEFT ) )
			dragged = -1;
	}

	// keep slider in bounds
 	if( sliderX < lx - ((wd - sliderT->width)/2) * xsc + (((minval+1)/2)*wd*xsc))
		sliderX = lx - ((wd - sliderT->width)/2) * xsc + (((minval+1)/2)*wd*xsc);
	if( sliderX > lx + ((wd - sliderT->width)/2) * xsc - ((1-((maxval+1)/2))*wd*xsc) )
		sliderX = lx + ((wd - sliderT->width)/2) * xsc - ((1-((maxval+1)/2))*wd*xsc);

	// draw the slider tab
	if( isMenu( MENU_GAME ) )
	{
		display_sprite( sliderT, sliderX - sc*xsc*sliderT->width/2, ly - sc*sliderT->height*ysc, lz+1, sc*xsc, sc*ysc, color);
		display_sprite( sliderB, sliderX - sc*xsc*sliderB->width/2, ly, lz+1, sc*xsc, sc*ysc, color );
	}
	else
		display_sprite( sliderT, sliderX - sc*xsc*sliderT->width/2, ly - sc*sliderT->height*ysc/2, lz+1, sc*xsc, sc*ysc, color);

	for (i = 0; i < count; i++)
	{
		float xVal = tickValues[i];
		int c = oldcolor;

 		if( flags & BUTTON_ULTRA && i > 2 )
			c = CLR_YELLOW;
		else if( xVal > maxval || xVal < minval )
			c = CLR_GREY;
		xVal *= ((wd - sliderT->width)/2) * xsc;
		xVal += lx;
		display_sprite(tick,xVal- xsc*tick->width/2,ly - ysc*tick->height/2 + 9*ysc, lz, xsc, ysc, c);
	}
	
	// normalize slider value back to between -1 and 1
	sliderX = (sliderX - lx)/(((wd - (float)sliderT->width)/2.f) * xsc );

 	if(sliderX<-1.0f) sliderX=-1.0f;
	if(sliderX>1.0f) sliderX=1.0f;

	// set the value
	return sliderX;

}

float doSliderTicks( float lx, float ly, float lz, float wd, float xsc, float ysc, float value,	int type, int color, int flags, int count, float *tickValues)
{
	return doSliderAll( lx, ly, lz, wd, xsc, ysc, value, -1.f, 1.f, type, color, flags, count, tickValues);
}

float doSlider( float lx, float ly, float lz, float wd, float xsc, float ysc, float value, int type, int color, int flags )
{
	return doSliderAll( lx, ly, lz, wd, xsc, ysc, value, -1.f, 1.f, type, color, flags, 0, 0);
}

//
//
float doVerticalSlider( float lx, float ly, float lz, float ht, float xsc, float ysc, float value, int type, int color )
{
 	AtlasTex * top = atlasLoadTexture( "map_zoom_minus.tga" );
	AtlasTex * bot = atlasLoadTexture( "map_zoom_plus.tga" );
	AtlasTex * mid = atlasLoadTexture( "map_zoom_bar.tga" );
  	AtlasTex * sliderT   = atlasLoadTexture( "scrollbar_top.tga" );
	AtlasTex * sliderB	= atlasLoadTexture( "scrollbar_btm.tga" );

	CBox box;
	int y, window_drag = 0, no_coll = 0;
 	float sliderY, sc = 1.f;
	static int dragOffY, dragged = -1;

	y = ly - ht*ysc/2;

	if( type == SLIDER_MAP_SCALE && (winDefs[WDW_MAP].drag_mode || winDefs[WDW_MAP].being_dragged) )
		window_drag = 1;

	BuildCBox( &box, lx - mid->width*xsc/2, y - top->height*ysc, top->width*xsc, (ht + top->height + bot->height)*ysc );
	if( mouseCollision(&box) && !window_drag )
		no_coll = 1;

	//now get the slider position
	sliderY = value;
  	sliderY *= ((ht - sliderT->height - sliderB->height)/2)*ysc;

	sliderY += ly;

	display_sprite( top, lx - top->width*xsc/2, y - top->height*ysc, lz+1, 1.f, 1.f, CLR_WHITE );
	display_sprite( mid, lx - mid->width*xsc/2, y -  top->height/2, lz, 1.f, (ht + top->height)/mid->height, color );
 	display_sprite( bot, lx - bot->width*xsc/2, y + ht, lz+1, 1.f, 1.f, CLR_WHITE );

	BuildCBox( &box, lx - top->width*xsc/2, y - top->height*ysc, top->width*xsc, top->height*ysc );

	if( mouseCollision(&box) && isDown(MS_LEFT) && type != dragged && !window_drag )
		sliderY -= ht/20;

	BuildCBox( &box, lx - bot->width*xsc/2, y + ht, bot->width*xsc, bot->height*ysc );

	if( mouseCollision(&box) && isDown(MS_LEFT) && type != dragged && !window_drag )
		sliderY += ht/20;

	// handle mouse collisions
 	BuildCBox( &box, lx - sliderT->width*xsc/2, sliderY - sliderT->height*ysc, sliderT->width, sliderT->height*2 );

	if( mouseCollision(&box) || dragged == type && !window_drag )
		sc = 1.2f;

	if( mouseDownHit( &box, MS_LEFT ) && !window_drag )
	{
		float cy = gMouseInpCur.y;
		float cx;

		if( shell_menu() )
			calc_scaled_point( &cx, &cy );

		dragged = type;
		dragOffY = sliderY - cy;
	}

 	if( dragged == type )
	{
		float cy = gMouseInpCur.y;
		float cx;

		if( shell_menu() )
		{
			calc_scaled_point( &cx, &cy );
		}

		sliderY = cy + dragOffY;

		if( !isDown( MS_LEFT ) )
			dragged = -1;
	}

	// keep slider in bounds
   	if( sliderY < ly - ((ht - sliderT->height - sliderB->height)/2) * ysc)
		sliderY = ly - ((ht - sliderT->height - sliderB->height)/2) * ysc;
	if( sliderY > ly + ((ht - sliderT->height - sliderB->height)/2) * ysc)
		sliderY = ly + ((ht - sliderT->height - sliderB->height)/2) * ysc;

	// draw the slider tab	
 	display_sprite( sliderT, lx - sliderT->width*sc*xsc/2, sliderY - sc*sliderT->height, lz+1, sc*xsc, sc*ysc, color);
 	display_sprite( sliderB, lx - sliderT->width*sc*xsc/2, sliderY, lz+1, sc*xsc, sc*ysc, color );

	// normalize slider value back to between -1 and 1
 	sliderY = (sliderY - ly)/(((ht - sliderT->height - sliderB->height)/2)*ysc);

	if(!collisions_off_for_rest_of_frame)
		collisions_off_for_rest_of_frame = no_coll;
	// set the value
	return sliderY;

}
