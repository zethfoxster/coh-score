

#include "wdwbase.h"
#include "utils.h"
#include "cmdcommon.h"
#include "mathutil.h"
#include "textureatlas.h"
#include "sprite_base.h"
#include "color.h"

#include "uiUtil.h"
#include "uiUtilGame.h"
#include "uiInput.h"
#include "uiColorPicker.h"

#include "uiWindows.h"
#include "uiChat.h"
#include "chatClient.h"
#include "uiChatUtil.h"
#include "ttFontUtil.h"
#include "wdwbase.h"
#include "uiUtil.h"
#include "uiUtilGame.h"
#include "uiInput.h"
#include "input.h"
#include "sprite_base.h"
#include "sprite_text.h"
#include "sprite_font.h"
#include "MessageStoreUtil.h"

void HSLFromRGB( int iR, int iG, int iB, float *hue, float *sat, float *lum)
{
	float r = ((float)iR)/255.f;
	float g = ((float)iG)/255.f;
	float b = ((float)iB)/255.f;
	float delta_r, delta_g, delta_b;

	float min = MIN( r, MIN(g, b) );
	float max = MAX( r, MAX(g, b) );
	float delta_max = max - min;

	*lum = ( max + min )/2;

	if( !delta_max )
	{
		*hue = 0;
		*sat = 0;
	}
	else
	{
		if( *lum < .5f )
			*sat = delta_max/(max+min);
		else
			*sat = delta_max/(2-max-min);
	}

	delta_r = ( ((max-r)/6)+(delta_max/2) )/delta_max;
	delta_g = ( ((max-g)/6)+(delta_max/2) )/delta_max;
	delta_b = ( ((max-b)/6)+(delta_max/2) )/delta_max;

	if( r == max )
		*hue = delta_b-delta_g;
	else if( g == max )
		*hue = (1.f/3) + delta_r - delta_b;
	else if( b == max )
		*hue = (2.f/3) + delta_g - delta_r;

	while( *hue < 0 )
		*hue += 1;
	while( *hue > 1 )
		*hue -= 1;
}	

float HueToRgb( float v1, float v2, float vH )
{
 	if( vH < 0 )
		vH += 1;
	if( vH > 1 ) 
		vH -= 1;
	if(6*vH < 1)
		return (v1 + (v2-v1)*6*vH);
	if(2*vH < 1)
		return (v2);
 	if(3*vH < 2)
		return (v1 + (v2-v1)*((2.f/3)-vH)*6);
	return v1;
}

int RGBFromHSL( float hue, float sat, float lum )
{
	int r, g, b;

	if ( sat == 0 )		//HSL values = 0 - 1
	{
		r = lum * 255;	 //RGB results = 0 - 255
		g = lum * 255;
		b = lum * 255;
	}
	else
	{
		float v1, v2;

		if ( lum < 0.5 )
			v2 = lum * ( 1 + sat );
		else           
			v2 = ( lum + sat ) - ( sat * lum );

		v1 = 2 * lum - v2;

		r = 255.f * HueToRgb( v1, v2, hue + ( 1.f/3 ) ) ;
		g = 255.f * HueToRgb( v1, v2, hue );
		b = 255.f * HueToRgb( v1, v2, hue - ( 1.f/3 ) );
	} 

	return (r<<24)|(g<<16)|(b<<8)|0xff;
}

#define NUM_HUES		32  // number of actual colors
#define NUM_SATS		5	// number of saturations
#define SAT_HT			5	// ht in pixels of saturation swatch
#define NUM_GREYSCALE	3	// number of greyscale boxes
#define NUM_LUM			(NUM_HUES-NUM_GREYSCALE-1)	// number of distinct luminosities
#define LUM_HT			2  // ht of the luminosity slider in number of sat_hts

void pickerSetfromIntColor( ColorPicker * picker, int color )
{
	HSLFromRGB( ((color>>24)&0xff), ((color>>16)&0xff), ((color>>8)&0xff), &picker->hue, &picker->saturation, &picker->luminosity );
	if( picker->saturation > 0 )
	{
		picker->cur_hue_idx = MINMAX(round(picker->hue*(NUM_HUES)), 0, NUM_HUES);
		picker->cur_sat_idx = MINMAX(round(picker->saturation*NUM_SATS), 1, NUM_SATS);
	}
	else
		picker->cur_hue_idx = picker->cur_sat_idx = -1;

	picker->cur_lum_idx = MINMAX(round(picker->luminosity*(NUM_LUM)), 1, NUM_LUM-1);
}

void pickerSetfromColor( ColorPicker * picker, Color*color )
{
	if (color->integer == 0) {
		picker->hue = picker->saturation = picker->luminosity = 0;
		picker->cur_hue_idx = picker->cur_sat_idx = picker->cur_lum_idx = -1;
		return;
	}

   	HSLFromRGB(color->r, color->g, color->b, &picker->hue, &picker->saturation, &picker->luminosity );
  	if( picker->saturation > 0 )
	{
 		picker->cur_hue_idx = MINMAX(round(picker->hue*(NUM_HUES)), 0, NUM_HUES);
		picker->cur_sat_idx = MINMAX(round(picker->saturation*NUM_SATS), 1, NUM_SATS);
	}
	else
		picker->cur_hue_idx = picker->cur_sat_idx = -1;

 	picker->cur_lum_idx = MINMAX(round(picker->luminosity*(NUM_LUM)), 1, NUM_LUM-1);
}

int drawColorPicker( ColorPicker * picker, float x, float y, float z, float wd, float ht, int ignoreInput)
{
	int i, j, hit = false, border_color;
	static float border_timer;
	float hue_wd, sat_ht, lum_off = NUM_GREYSCALE+1;
	CBox box;

  	border_timer += TIMESTEP*.01f;
	if( border_timer > 1.0 )
		border_timer -= 1.f;
 	border_color = RGBFromHSL( border_timer, 1.f, .8f );

   	picker->trans_sc -= TIMESTEP*.1f;
   	picker->trans_sc = MAX( picker->trans_sc, 0.f );

 	drawFlatFrame( PIX3, R4, x, y, z, wd, ht, 1.f, 0xffffff44, 0xffffff44 );

 	z += 3;
   	x += R6;
	y += R6;
	wd -= 2*R6;
	ht -= 2*R6;

	hue_wd = wd/NUM_HUES;
  	sat_ht = ht/(NUM_SATS+LUM_HT+1+2)+1; // 1 is spacer between hue and brightness

	// hue
 	for( i = 0; i < NUM_HUES; i++ )
	{
		for( j = NUM_SATS; j > 0; j-- )
		{
  			BuildCBox( &box, x + i*hue_wd, y + (NUM_SATS-j)*sat_ht, hue_wd, sat_ht );
		
 			if(!ignoreInput && mouseClickHit(&box, MS_LEFT) )
			{
				picker->cur_hue_idx = i;
				picker->cur_sat_idx = j;
				picker->hue = ((float)i)/(NUM_HUES);
				picker->saturation = ((float)j)/NUM_SATS;
 				picker->luminosity = MINMAX( picker->luminosity, 1.f/NUM_LUM, ((float)(NUM_LUM-1))/NUM_LUM );
				hit = true;
			}
  			else if(!ignoreInput && mouseCollision(&box)) // shrink box on mouseover
			{
				BuildCBox( &box, x + i*hue_wd - 1, y + (NUM_SATS-j)*sat_ht - 1, hue_wd + 2, sat_ht + 2 );
				drawBox( &box, z, 0xff, 0 );
			}

  			if( i == picker->cur_hue_idx && j == picker->cur_sat_idx )
			{
 				BuildCBox( &box, x + i*hue_wd - 1, y + (NUM_SATS-j)*sat_ht - 1, hue_wd + 2, sat_ht + 2 );
				drawBox( &box, z, 0xff, RGBFromHSL( ((float)i)/(NUM_HUES), ((float)j)/NUM_SATS, .5f ) );
			}
			else
				drawBox( &box, z, 0, RGBFromHSL( ((float)i)/(NUM_HUES), ((float)j)/NUM_SATS, .5f ) );
		}
	}

	// greyscale
	for( i = 0; i < NUM_GREYSCALE; i++ )
	{
 		BuildCBox( &box, x + i*hue_wd, y + ht - 2*sat_ht, hue_wd, sat_ht*2 );

 		if( !ignoreInput && mouseClickHit(&box, MS_LEFT) )
		{
			picker->cur_hue_idx = -1;
			picker->cur_sat_idx = -1;
			picker->hue = 0;
			picker->saturation = 0;
			picker->luminosity = ((float)i)/(NUM_GREYSCALE-1);
			picker->cur_lum_idx = MIN(NUM_LUM-1, picker->luminosity*NUM_LUM);
			hit = true;
		}
		else if( !ignoreInput && mouseCollision(&box)) // shrink box on mouseover
			BuildCBox( &box, x + i*hue_wd + 1, y + ht - 2*sat_ht + 1, hue_wd - 2, sat_ht*2 - 2 );

		drawBox( &box, z, 0, RGBFromHSL( 0, 0, ((float)i)/(NUM_GREYSCALE-1) ) );
	}

	// brightness
 	for( i = 1; i < NUM_LUM; i++ )
	{
	 	float squished_hue_wd = (wd - hue_wd*4)/(NUM_LUM+2);
		float enlarge_sc = 0.f;
		int col = 0;
		lum_off = NUM_GREYSCALE+1;

		if( i > picker->cur_lum_idx )
			lum_off += 2*(1-picker->trans_sc);
		if( i > picker->last_lum_idx )
			lum_off += 2*picker->trans_sc;

   		if( i == picker->cur_lum_idx  )
			enlarge_sc = (1-picker->trans_sc);
		else if ( i == picker->last_lum_idx )
			enlarge_sc = picker->trans_sc;


  		BuildCBox( &box, x + (i+lum_off)*squished_hue_wd, y + ht - (2.f+.5*enlarge_sc)*sat_ht, squished_hue_wd*(1.f+2*enlarge_sc), sat_ht*(2.f+enlarge_sc) );

 		if( !ignoreInput && mouseCollision(&box)) // shrink box on mouseover
		{
			if( mouseUp(MS_LEFT) )
			{
				picker->luminosity = ((float)i)/(NUM_LUM);
				picker->last_lum_idx = picker->cur_lum_idx;
				picker->cur_lum_idx = i;
				picker->trans_sc = 1.f;
				hit = true;
			}
			BuildCBox( &box, x + (i+lum_off)*squished_hue_wd-1, y + ht - (2+.5*enlarge_sc)*sat_ht-1, floor(squished_hue_wd*(1+2*enlarge_sc)+2), floor(sat_ht*(2+enlarge_sc)+2) );	
			col = border_color;
		}

 		if( i == picker->cur_lum_idx )
		{
			BuildCBox( &box, x + (i+lum_off)*squished_hue_wd-1, y + ht - (2+.5*enlarge_sc)*sat_ht-1, floor(squished_hue_wd*(1+2*enlarge_sc)+2), floor(sat_ht*(2+enlarge_sc)+2) );
			col = border_color;
		}

		drawBox( &box, z, col, RGBFromHSL( picker->hue, picker->saturation, ((float)i)/(NUM_LUM) ) );
	}
 
	// lastly keep current rgb updated
	picker->rgb_color = RGBFromHSL( picker->hue, picker->saturation, picker->luminosity );

	return hit;
}


static ChatChannel * s_channel;
static ColorPicker cp[2] = {0};

void startColorPickerWindow( char * pchChannel )
{
	ChatChannel * pChannel = GetChannel(pchChannel);
	if( !pChannel )
		return;
	s_channel = pChannel;
	pickerSetfromIntColor(&cp[0], s_channel->color);
	pickerSetfromIntColor(&cp[1], s_channel->color2);
	window_setMode(WDW_COLORPICKER, WINDOW_GROWING);
}


int chatColorPickerWindow(void)
{
	F32 x, y, z, wd, ht, sc;
	int c1, c2, color, bcolor;

	if(!window_getDims(WDW_COLORPICKER, &x, &y, &z, &wd, &ht, &sc, &color, &bcolor))
		return 0;

	drawFrame( PIX3, R10, x, y, z, wd, ht, sc, color, bcolor);
	if( !s_channel )
	{
		window_setMode(WDW_COLORPICKER, WINDOW_SHRINKING);
		return 0;
	}

	// Title
	font( &game_12 );
	font_color( CLR_WHITE, CLR_WHITE );
	y+= 18*sc;
	cprnt( x+wd/2, y, z, sc, sc, "PickChannelColors" );  
	y+= 15*sc;
	cprntEx( x+wd/2, y, z, sc, sc, NO_MSPRINT|CENTER_X, s_channel->name );
	

	// Color Pickers
	y+= 20*sc;
	prnt(x+10*sc, y, z, sc, sc, "Color1String");
	drawColorPicker( &cp[0], x + 10*sc, y, z, wd - 20*sc, 60*sc, 0);
	y+= 80*sc;
	prnt(x+10*sc, y, z, sc, sc, "Color2String");
	drawColorPicker( &cp[1], x + 10*sc, y, z, wd - 20*sc, 60*sc, 0);
	y+= 80*sc;

	c1 = cp[0].rgb_color;
	c2 = cp[1].rgb_color;

	// Example text
	font_color( c1, c2 );
	cprnt( x+wd/2, y, z, sc, sc, "ExampleTextColor" );

	y+= 15*sc;
	// Apply Button
	if( D_MOUSEHIT == drawStdButton( x + wd/2, y, z, MIN(200*sc,wd-2*R10*sc), 20*sc, color, "AppyColors", 1.f, !s_channel ) )
	{
		s_channel->color = c1;
		s_channel->color2 = c2;
		sendChatSettingsToServer();
	}

	return 0;
}