
#include "win_init.h" // for windowClientSize

#include "uiUtil.h"
#include "uiGame.h"
#include "uiInput.h"
#include "uiWindows.h"
#include "uiUtilMenu.h"
#include "uiUtilGame.h"
#include "uiScrollBar.h"

#include "sprite_base.h"
#include "textureatlas.h"

#include "wdwbase.h"
#include "mathutil.h"
#include "cmdcommon.h"
#include "win_init.h"

#include "uiUtilGame.h"
#include "uiOptions.h"
#include "uiBox.h"
#include "uiSlider.h"
#include "uiLogin.h"
#include "cmdgame.h"

int gScrollBarDragging = 0;
int gScrollBarDraggingLastFrame = 0;

// SCROLL BAR
//----------------------------------------------------------------------------------------------------------

#define SB_EXPAND_SPEED .1f
#define SB_MIN_SCALE	 .5f


// adds vertical bars to visually represent visible & not visible sizes
int doScrollBarDebug(ScrollBar *sb, int view_ht, int doc_ht, int xp, int yp, int zp )
{
	CBox box;
 
	// doc height
  	BuildCBox(&box, xp-15, yp, 10, view_ht); 
	drawBox(&box, zp-1, CLR_RED, CLR_RED);

	// view height
  	BuildCBox(&box, xp+5, yp - sb->offset, 10, doc_ht); 
  	drawBox(&box, zp-1, CLR_YELLOW, CLR_YELLOW);

	return doScrollBar(sb, view_ht, doc_ht, xp, yp, zp, 0, 0);
}



int doScrollBarEx( ScrollBar *sb, int view_ht, int doc_ht, int xp, int yp, int zp, CBox * cbox, UIBox * uibox, float screenScaleX, float screenScaleY )
{
	int color = CLR_WHITE, bcolor, gripColor = 0xffffff88, i;
	static int upTimer = 0, downTimer = 0;
  	float	x = 0, y = 0, z = zp, wd = 0, ht = 0, scale = 1.f;
	float	scrollHt, extraSpace, topY, buttonY, buttonScale, buttonSize, pages, shellSc = 1.f, upHit = 0, downHit = 0;
	int scrollable = 0;

	CBox midBox, bigBox, columnBox;

	static AtlasTex *bot, *mid, *rib, *top, *up, *down;
	static AtlasTex *hbot, *hmid, *hrib, *htop, *hup, *hdown;
	static AtlasTex *arrow_h, *arrow, *arrow_s, *inside_h, *inside_s_low, *inside_s_mid, *inside_s_top, *mid_end, *mid_mid, *outside_end, *outside_end_mid;

	static int texBindInit=0;
	float screenScale = MIN(screenScaleX, screenScaleY);
 	if (!texBindInit)
	{
		texBindInit = 1;
		bot	= atlasLoadTexture("scrollbar_btm.tga");
  		mid	= atlasLoadTexture("scrollbar_mid_plain.tga");
  		rib	= atlasLoadTexture( "scrollbar_mid_rib.tga" );
   		top	= atlasLoadTexture("scrollbar_top.tga");
		up	= atlasLoadTexture( "map_zoom_uparrow.tga" );
		down= atlasLoadTexture( "map_zoom_downarrow.tga" );

		hbot	= atlasLoadTexture( "h_scrollbar_top.tga");
		hmid	= atlasLoadTexture( "h_scrollbar_mid_plain.tga");
		hrib	= atlasLoadTexture( "h_scrollbar_mid_rib.tga" );
		htop	= atlasLoadTexture( "h_scrollbar_btm.tga"); 
		hup		= atlasLoadTexture( "map_zoom_leftarrow.tga" );
		hdown	= atlasLoadTexture( "map_zoom_rightarrow.tga" );

		arrow_h	= atlasLoadTexture("scrollbar_002_arrow_highlight.tga" ); 
		arrow_s	= atlasLoadTexture("scrollbar_002_arrow_shadow.tga" );  
		arrow	= atlasLoadTexture("scrollbar_002_arrow_outline.tga" );  

		inside_h = atlasLoadTexture("scrollbar_002_inside_highlight.tga" );  
		inside_s_low = atlasLoadTexture("scrollbar_002_inside_shadow_btm.tga" );  
		inside_s_mid = atlasLoadTexture("scrollbar_002_inside_shadow_mid.tga" );  
		inside_s_top = atlasLoadTexture("scrollbar_002_inside_shadow_top.tga" );  

		mid_end	= atlasLoadTexture("scrollbar_002_mid_end.tga" );  
		mid_mid	= atlasLoadTexture("scrollbar_002_mid_mid.tga" );  
		outside_end	= atlasLoadTexture("scrollbar_002_outside_end.tga" );  
		outside_end_mid	= atlasLoadTexture("scrollbar_002_outside_end_mid.tga" );  
	}

	// put new scrollbox stuff in here, old scrollbox stuff to be phased out
 	if( view_ht > doc_ht && !sb->offset )
		return 0;
	
	x = xp;
 	if( sb->wdw >= 0)	// JS: Hack to detach scroll bars from windows and not get crazy colors
	{
		if( sb->wdw )
		{
  			if( !window_getDims( sb->wdw, &x, &y, &z, &wd, &ht, &scale, &color, &bcolor ) )
				return FALSE;

			if( sb->horizontal )
				y = y + yp;
			else
	 			x = x+wd;

 			color |= 0xff;
		}
		else if ( shell_menu() )
		{
			int w, h;
			windowClientSizeThisFrame( &w, &h );

			if( sb->horizontal )
				y = y + yp;

			color = SELECT_FROM_UISKIN( 0x2a77ffff, 0x444444ff, 0x6e6e6eff );
			gripColor = SELECT_FROM_UISKIN( 0x2a77ff88, 0x44444488, 0x6e6e6e88 );

			if (sb->horizontal)
				shellSc = ((float)DEFAULT_SCRN_WD/w);
			else
				shellSc = ((float)DEFAULT_SCRN_HT/h);
		}
		else
  			color = gripColor = winDefs[1].loc.color|0xff;
	}

	if (sb->use_color)
		color = gripColor = sb->color;
  	if(sb->architect==1)
	{
 		color = CLR_MM_TEXT; 
		gripColor = CLR_MM_TEXT&0xffffff88;
	}
 	else if(sb->architect==2)
	{
 		color = CLR_MM_BUTTON_ERROR_TEXT_LIT; 
		gripColor = CLR_MM_BUTTON_ERROR_TEXT_LIT&0xffffff88;
	}
	
	// this is the effective scrollbar height
   	scrollHt = view_ht; 

	// first calculate the size of the scroll button
	pages = (float)(doc_ht)/view_ht;
	if ( pages < 1 )
		pages = 1;

 	buttonSize = scrollHt/(pages); // the button size represents one page

	// this is the difference between the document area and the view area
	extraSpace = doc_ht-view_ht;
	if( extraSpace < 0 )
		extraSpace = 0;

	// the top of the document
	if( sb->horizontal )
		topY = x + xp + htop->width*scale;
	else
	 	topY = y + yp + top->height*scale;

	if ( extraSpace == 0 )
		buttonY = topY;
	else
		buttonY = topY + ( sb->offset / extraSpace ) * scrollHt;

 	if( sb->horizontal )
	{
		buttonScale = ((buttonSize) - (htop->width + hbot->width)*scale)/(hmid->width);
		if ( buttonScale < 2 )
			buttonScale = 2.f;

		scrollHt -= (htop->width+hbot->width)*scale + hmid->width*buttonScale;

		BuildCBox( &midBox, ((sb->offset/extraSpace)*scrollHt + topY)*screenScaleX - htop->width*screenScale, y*screenScaleY-htop->height*screenScale/2,
			htop->width*screenScale + buttonScale*hmid->width*screenScaleX + hbot->width*screenScale, hmid->height*screenScale );

		BuildCBox( &bigBox, ((sb->offset/extraSpace)*scrollHt + topY)*screenScaleX - htop->width*2*screenScale, y*screenScaleY-htop->height*screenScale, 
 			htop->width*screenScale + buttonScale*hmid->width*screenScaleX + hbot->width*2*screenScale, hmid->height*2*screenScale );

	 	BuildCBox( &columnBox, (x+xp)*screenScaleX, y*screenScaleY-htop->height*screenScale, view_ht*screenScaleX, hmid->height*2*screenScaleY );
		
	}
 	else
	{
 		buttonScale = ((buttonSize) - (top->height + bot->height)*scale)/mid->height;
		if ( buttonScale < 2 )
			buttonScale = 2.f;

		scrollHt -= (top->height+bot->height)*scale + mid->height*buttonScale;

		BuildCBox( &midBox, x*screenScaleX-top->width*screenScale/2, ((sb->offset/extraSpace)*scrollHt + topY)*screenScaleY - top->height*screenScale, mid->width*screenScale, 
 			top->height*screenScale + buttonScale*mid->height*screenScaleY + bot->height*screenScale );
		
		BuildCBox( &bigBox, x*screenScaleX-top->width*screenScale, ((sb->offset/extraSpace)*scrollHt + topY)*screenScaleY - top->height*2*screenScale, mid->width*2*screenScale, 
			top->height*screenScale + buttonScale*mid->height*screenScaleY + bot->height*2*screenScale );

		BuildCBox( &columnBox, x*screenScaleX-top->width*screenScale, (y+yp)*screenScaleY, mid->width*2*screenScale, view_ht*screenScaleY );

	}

   	if( mouseDownHit( &midBox, MS_LEFT ) && extraSpace > 0 )
	{
		sb->grabbed = TRUE;
		if( sb->horizontal )
		{
			sb->ydiff = (sb->offset/extraSpace)*scrollHt - gMouseInpCur.x*shellSc;
			sb->yclick = (gMouseInpCur.x*shellSc - topY - (sb->offset/extraSpace)*scrollHt)/(hmid->height*buttonScale);
		}
		else
		{
			sb->ydiff = (sb->offset/extraSpace)*scrollHt - gMouseInpCur.y*shellSc;
			sb->yclick = (gMouseInpCur.y*shellSc - topY - (sb->offset/extraSpace)*scrollHt)/(mid->width*buttonScale);
		}
		collisions_off_for_rest_of_frame = TRUE;
	}
	else if( !isDown( MS_LEFT ) ) //|| !extraSpace )
		sb->grabbed = FALSE;

	if( mouseCollision( &columnBox ) )
		scrollable = 1;
	if( mouseCollision( &bigBox ) )
	{
		sb->xsc += TIMESTEP*SB_EXPAND_SPEED;
		if ( sb->xsc > 1.f )
			sb->xsc = 1.f;
		scrollable = 1;
		collisions_off_for_rest_of_frame = TRUE;
	}
	else
	{
		if( shell_menu() )
			sb->xsc = 1.f;
		else
		{
			sb->xsc -= TIMESTEP*SB_EXPAND_SPEED;
			if ( sb->xsc < SB_MIN_SCALE )
				sb->xsc = SB_MIN_SCALE;
		}
	}

  	if( sb->grabbed ) // they are dragging the bar
	{
		gScrollBarDragging = TRUE;
		sb->xsc = 1.f;

		if( sb->horizontal )
			sb->offset = (gMouseInpCur.x*shellSc + sb->ydiff)*extraSpace/scrollHt;
		else
			sb->offset = (gMouseInpCur.y*shellSc + sb->ydiff)*extraSpace/scrollHt;

		if ( sb->ribs < 10 )
			sb->ribs += 2;
	}
	else
	{
		if( sb->ribs > 0 )
			sb->ribs -=2;
	}

	// top arrow
	if( sb->loginSlider )
	{
   		drawSliderBackground( columnBox.lx, columnBox.ly + 16*screenScale, z, columnBox.hx-columnBox.lx, screenScale, 0 );
 		upHit = drawNextCircle( columnBox.lx-20*screenScale, columnBox.ly + 16*screenScale, z, screenScale, 0, !scrollHt);
		downHit =  drawNextCircle( columnBox.hx+20*screenScale, columnBox.ly + 16*screenScale, z, screenScale, 1, !scrollHt);
	}
   	else if( scrollHt )
	{
		if( !sb->horizontal )
		{
     		upHit = drawGenericButton( up, x*screenScaleX - (up->width/2+3)*scale*screenScale, (y + yp)*screenScaleY - up->height*scale*screenScale, zp+2, 1.1f*scale*screenScale, color, color, TRUE );
			downHit = drawGenericButton( down, x*screenScaleX - (down->width/2+3)*scale*screenScale, (y + yp + view_ht)*screenScaleY, zp+2, 1.1f*scale*screenScale, color, color, TRUE );
			
 	 		if( sb->offset < extraSpace && sb->pulseArrow )
			{
				float sc;
				int col;

				sb->pulse += TIMESTEP*.06f;
				if( sb->pulse > 1.f )
					sb->pulse = 0.f;

				sc = scale*(1+sb->pulse*2);
				col = color;
				brightenColor( &col, (1-sb->pulse)*100 );
				col = (col&0xffffff00)|(int)((1-sb->pulse)*255);
				
     			display_sprite( down, x*screenScaleX - (down->width/2)*sc*screenScale - 3*scale*screenScale, (y + yp + view_ht)*screenScaleY , zp+2, 1.1f*sc*screenScale, 1.1f*sc*screenScale, col );
			}
		}
		else
		{
 			upHit = drawGenericButton( hup, (x + xp)*screenScaleX - hup->width*scale*screenScale, y*screenScaleY - (hup->height/2+3)*scale*screenScale, zp+2, 1.1f*scale*screenScale, color, color, TRUE );
			downHit = drawGenericButton( hdown, (x + xp + view_ht)*screenScaleX, y*screenScaleY - (hdown->height/2+3)*scale*screenScale, zp+2, 1.1f*scale*screenScale, color, color, TRUE );
		}
	}

	// this could probably be cleaned up a little
	{
		CBox box;
		float wheelx,wheely,wheelwd,wheelht;

 		if( cbox )
			memcpy(&box, cbox, sizeof(CBox) );
		else if( uibox )
			uiBoxToCBox(uibox, &box );
		else if( sb->wdw && window_getDims( sb->wdw, &wheelx, &wheely, 0, &wheelwd, &wheelht, 0, 0, 0 ))
			BuildCBox(&box, wheelx, wheely, wheelwd, wheelht );

		if( mouseCollision(&box) )
			scrollable = 1;
	}

	if( upHit || downHit )
		collisions_off_for_rest_of_frame = TRUE;

	if( scrollable && !optionGet(kUO_DisableMouseScroll) )
	{
		int scroll_wheel = gMouseInpCur.z - gMouseInpLast.z;
		if(scroll_wheel < 0)
			sb->offset += optionGetf(kUO_MouseScrollSpeed)*scale;
		else if(scroll_wheel > 0)
			sb->offset -= optionGetf(kUO_MouseScrollSpeed)*scale;
		game_state.scrolled = 1;
	}

 	if( sb->loginSlider )
	{
		if( upHit )
			setCharPage( 0 );
		if( downHit )
			setCharPage( 1 );
	}
	else
	{
   		if( upHit == D_MOUSEHIT )
			sb->offset -= 15*scale;
		if( downHit == D_MOUSEHIT )
			sb->offset += 15*scale;
	}

	if( upHit == D_MOUSEDOWN || sb->buttonHeld == 1 )
	{
        sb->buttonHeld = 1;
		sb->offset -= 5*scale*TIMESTEP*10;
	}
	if( downHit == D_MOUSEDOWN || sb->buttonHeld == 2 )
	{
        sb->buttonHeld = 2;
		sb->offset += 5*scale*TIMESTEP*10;
	}

	if( sb->buttonHeld )
	{
		if( !isDown( MS_LEFT ) )
			sb->buttonHeld = 0;
		else
			gScrollBarDragging = TRUE;
	}

	// keep pill inbounds
 	if( sb->offset > extraSpace)// + buttonSize - (top->height + buttonScale*mid->height + bot->height) )
		sb->offset = extraSpace ;//+ buttonSize - (top->height + buttonScale*mid->height + bot->height);
	if( sb->offset < 0 )
		sb->offset = 0;

	if( sb->horizontal )
	{
		for( i = 0; i < sb->ribs/2; i++ ) 
		{
			int xloc =  topY*screenScaleX + (hmid->width*buttonScale*screenScaleX-1*scale*screenScale)/2 + (sb->offset/extraSpace)*scrollHt*screenScaleX + hrib->width*(i+1)*scale*screenScale;

			if( xloc < (sb->offset/extraSpace)*scrollHt*screenScaleX + topY*screenScaleX + (hmid->width*buttonScale*scale*screenScaleX - hrib->width*scale)*screenScale )
				display_sprite( hrib, xloc, y*screenScaleY - (sb->xsc*hmid->height/2 + 2)*scale*screenScale, zp+4, scale*screenScale, sb->xsc*scale*screenScale, gripColor );
		}

		for( i = 0; i < sb->ribs/2; i++ )
		{
			int xloc =  topY*screenScaleX + (hmid->width*buttonScale*screenScaleX-1*scale*screenScale)/2 + (sb->offset/extraSpace)*scrollHt*screenScaleX - hrib->width*(i+1)*scale*screenScale;

			if( xloc > (sb->offset/extraSpace)*scrollHt*screenScaleX + topY*screenScaleX )
				display_sprite( hrib, xloc, y*screenScaleY - (sb->xsc*hmid->height/2 + 2)*scale*screenScale, zp+4, scale*screenScale, sb->xsc*scale*screenScale, gripColor );
		}

		// display top
		display_sprite( htop, ((sb->offset/extraSpace)*scrollHt + topY)*screenScaleX - htop->width*scale*screenScale, y*screenScaleY - (sb->xsc*htop->height/2 + 2)*scale*screenScale, zp+2, scale*screenScale, sb->xsc*scale*screenScale, color);
		// display mid
 		display_sprite( hmid, ((sb->offset/extraSpace)*scrollHt + topY)*screenScaleX, y*screenScaleY - (sb->xsc*hmid->height/2 + 2)*scale*screenScale, zp+1, buttonScale*screenScaleX, sb->xsc*scale*screenScale, color);
		// display bot
		display_sprite( hbot, (((sb->offset/extraSpace)*scrollHt + topY)*screenScaleX + hmid->width*buttonScale*screenScaleX)-1, y*screenScaleY - (sb->xsc*hbot->height/2 + 2)*scale*screenScale, zp+3, scale*screenScale, sb->xsc*scale*screenScale, color );
	}
	else
	{
   		for( i = 0; i < sb->ribs/2; i++ ) 
 		{
   			int yloc = topY*screenScaleY + (mid->height*buttonScale*screenScaleY-1*scale*screenScale)/2 + (sb->offset/extraSpace)*scrollHt*screenScaleY + rib->height*(i+1)*scale*screenScale;

			if( yloc < (sb->offset/extraSpace)*scrollHt*screenScaleY + topY*screenScaleY + mid->height*buttonScale*scale*screenScaleY - rib->height*scale*screenScale )
				display_sprite( rib, x*screenScaleX - (sb->xsc*mid->width/2 + 2)*scale*screenScale, yloc, zp+4, sb->xsc*scale*screenScale, scale*screenScale, gripColor );
		}

  		for( i = 0; i < sb->ribs/2; i++ )
		{
			int yloc = topY*screenScaleY + (mid->height*buttonScale*screenScaleY-1*scale*screenScale)/2 + (sb->offset/extraSpace)*scrollHt*screenScaleY - rib->height*(i+1)*scale*screenScale;

			if( yloc > (sb->offset/extraSpace)*scrollHt*screenScaleY + topY*screenScaleY )
				display_sprite( rib, x*screenScaleX - (sb->xsc*mid->width/2 + 2)*scale*screenScale, yloc, zp+4, sb->xsc*scale*screenScale, scale*screenScale, gripColor );
		}

		// display top
		display_sprite( top, x*screenScaleX - (sb->xsc*top->width/2 + 2)*scale*screenScale, ((sb->offset/extraSpace)*scrollHt + topY)*screenScaleY - (top->height*scale*screenScale), zp+2, sb->xsc*scale*screenScale, scale*screenScale, color );
  		// display mid
		display_sprite( mid, x*screenScaleX - (sb->xsc*mid->width/2 + 2)*scale*screenScale, ((sb->offset/extraSpace)*scrollHt + topY)*screenScaleY, zp+1, sb->xsc*scale*screenScale, buttonScale*screenScaleY, color );
		// display bot
		display_sprite( bot, x*screenScaleX - (sb->xsc*bot->width/2 + 2)*scale*screenScale, ((sb->offset/extraSpace)*scrollHt + topY)*screenScaleY + (mid->height*buttonScale*screenScaleY)-1, zp+3, sb->xsc*scale*screenScale, scale*screenScale, color );
	}
	return 0;
}

