#define RT_PRIVATE
#include "ogl.h"
#include "stringutil.h"
#include "wcw_statemgmt.h"
#include "rt_font.h" 
#include "rt_model_cache.h"
#include "rt_tex.h"
#include "timing.h"
#include "rt_state.h"
#include "rt_stats.h"
#include "cmdgame.h"

void rdrSimpleTextDirect(RdrDbgFont *pkg)
{
	FontMessage *messages = (FontMessage*)(pkg+1);
	char *str_base = (char*)(messages + pkg->count);

	rdrTextDirect(pkg->count, messages, str_base);
}

void rdrTextDirect(int count, FontMessage *messages, char *str_base)
{
	int			i,idx;
	F32			x,y;
	FontMessage	*msg;
	FontState	*fstate;
	char		*s;
	Font		*font;
	FontLetter	*letter;
	F32			v_size = 480, h_size = 640;
	F32			sts_a[2000],*sts;
	F32			xys_a[2000],*xys;
	Font		*last_font = ( Font * )-1;
	int			h,v;

	rdrBeginMarker(__FUNCTION__ ": '%s'", str_base ? str_base:"" ); 

	h_size = h	= rdr_view_state.width;
	v_size = v	= rdr_view_state.height;

	RT_STAT_ADDSPRITE(2 * pkg->count);
	modelBindDefaultBuffer();
	glMatrixMode( GL_PROJECTION ); CHECKGL;
	glPushMatrix(); CHECKGL;
	glLoadIdentity(); CHECKGL;
	glOrtho( 0.0f, h_size, 0.0f, v_size, -1.0f, 1.0f ); CHECKGL;
	WCW_SetProjectionMatrixDirty();

	glMatrixMode( GL_MODELVIEW ); CHECKGL;
	glPushMatrix(); CHECKGL;
	WCW_LoadModelViewMatrixIdentity();

	WCW_EnableGL_LIGHTING(0);
	glDisable( GL_DEPTH_TEST ); CHECKGL;
	WCW_Fog(0);
	WCW_Color4(255, 255, 255, 255);
	modelDrawState(DRAWMODE_SPRITE, FORCE_SET);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), FORCE_SET);
	WCW_PrepareToDraw();

	WCW_ClearTextureStateToWhite();

	texBindTexCoordPointer( TEXLAYER_BASE, sts_a );
	WCW_VertexPointer(2,GL_FLOAT,0,xys_a);

	for(i=0;i<count;i++)
	{
	F32	left,right,top,bot,x2,y2,y1,x1,oo_w,oo_h;

		msg		= &messages[i];
		fstate	= &msg->state;
		x		= msg->X;
		y		= msg->Y;
		font	= fstate->font;

		if (!font)
			continue;
		if (x > TEXT_JUSTIFY*0.5f)
			x = h_size - (640.f - (x - TEXT_JUSTIFY));
		if (y > TEXT_JUSTIFY*0.5f)
			y = v_size - (480.f - (y - TEXT_JUSTIFY));
		oo_w = 1.f / font->tex_width;
		oo_h = 1.f / font->tex_height;
		xys = xys_a;
		sts = sts_a;
		idx = 0;
		if (last_font != msg->state.font)
		{
			last_font = msg->state.font;
			texBind(TEXLAYER_BASE, last_font->texid);
		}
 
#if 0
		WCW_Color4(fstate->rgba[0],fstate->rgba[1],fstate->rgba[2],fstate->rgba[3]);
		for(s=str_base+ msg->StringIdx;*s;s = UTF8GetNextCharacter(s))
		{
			char chr = *s;
			if(chr & 0x80) chr = '?'; // utf-8
			
			letter = &font->Letters[font->Indices[(int)chr]];
			if (!letter->Width) continue;

			left	= (letter->X) * oo_w;
			right	= (letter->X + letter->Width) * oo_w;
			top		= (letter->Y) * oo_h;
			bot		= (letter->Y + letter->Height) * oo_h;

			x2 = (x + letter->Width * fstate->scaleX) / 1;
			x1 = x / 1;
			y2 = y + letter->Height * fstate->scaleY;
			y1 = (v_size - y) / 1;
			y2 = (v_size - y2) / 1;

#if 1	
			texSetWhite(TEXLAYER_BASE);
			WCW_Color4(70,70,70,min(fstate->rgba[3],170));
			glBegin( GL_QUADS );
			glTexCoord2f( left, top );
			glVertex2f( x1, y1 );
			glTexCoord2f( right, top );
			glVertex2f( x2, y1 );
			glTexCoord2f( right, bot );
			glVertex2f( x2, y2 );
			glTexCoord2f( left, bot );
			glVertex2f( x1, y2 );
			glEnd(); CHECKGL;
			RT_STAT_DRAW_TRIS(2)
			
			texBind(TEXLAYER_BASE, last_font->texid);
			WCW_Color4(fstate->rgba[0],fstate->rgba[1],fstate->rgba[2],fstate->rgba[3]);
			glBegin( GL_QUADS );
			glTexCoord2f( left, top );
			glVertex2f( x1, y1 );
			glTexCoord2f( right, top );
			glVertex2f( x2, y1 );
			glTexCoord2f( right, bot );
			glVertex2f( x2, y2 );
			glTexCoord2f( left, bot );
			glVertex2f( x1, y2 );
			glEnd(); CHECKGL;
			RT_STAT_DRAW_TRIS(2)
#else
			*sts++ = left;
			*sts++ = top;
			*xys++ = x;
			*xys++ = y1;

			*sts++ = right;
			*sts++ = top;
			*xys++ = x2;
			*xys++ = y1;

			*sts++ = right;
			*sts++ = bot;
			*xys++ = x2;
			*xys++ = y2;

			*sts++ = left;
			*sts++ = bot;
			*xys++ = x;
			*xys++ = y2;

			idx += 8;

#endif
			x += letter->Width * fstate->scaleX;
		}
#else
		{
			int numTris = 0;
			F32 savedX = x;

			texSetWhite(TEXLAYER_BASE);
			WCW_Color4(70,70,70,min(fstate->rgba[3],170));
			glBegin( GL_QUADS );

			for(s=str_base+ msg->StringIdx;*s;s = UTF8GetNextCharacter(s))
			{
				char chr = *s;
				if(chr & 0x80) chr = '?'; // utf-8
				
				letter = &font->Letters[font->Indices[(int)chr]];
				if (!letter->Width) continue;

				left	= (letter->X) * oo_w;
				right	= (letter->X + letter->Width) * oo_w;
				top		= (letter->Y) * oo_h;
				bot		= (letter->Y + letter->Height) * oo_h;

				x2 = (x + letter->Width * fstate->scaleX) / 1;
				x1 = x / 1;
				y2 = y + letter->Height * fstate->scaleY;
				y1 = (v_size - y) / 1;
				y2 = (v_size - y2) / 1;

				glTexCoord2f( left, top );
				glVertex2f( x1, y1 );
				glTexCoord2f( right, top );
				glVertex2f( x2, y1 );
				glTexCoord2f( right, bot );
				glVertex2f( x2, y2 );
				glTexCoord2f( left, bot );
				glVertex2f( x1, y2 );
				x += letter->Width * fstate->scaleX;
				numTris+=2;
			}

			glEnd(); CHECKGL;
			RT_STAT_DRAW_TRIS(numTris)
				
			x = savedX;
			texBind(TEXLAYER_BASE, last_font->texid);
			WCW_Color4(fstate->rgba[0],fstate->rgba[1],fstate->rgba[2],fstate->rgba[3]);
			glBegin( GL_QUADS );

			for(s=str_base+ msg->StringIdx;*s;s = UTF8GetNextCharacter(s))
			{
				char chr = *s;
				if(chr & 0x80) chr = '?'; // utf-8
				
				letter = &font->Letters[font->Indices[(int)chr]];
				if (!letter->Width) continue;

				left	= (letter->X) * oo_w;
				right	= (letter->X + letter->Width) * oo_w;
				top		= (letter->Y) * oo_h;
				bot		= (letter->Y + letter->Height) * oo_h;

				x2 = (x + letter->Width * fstate->scaleX) / 1;
				x1 = x / 1;
				y2 = y + letter->Height * fstate->scaleY;
				y1 = (v_size - y) / 1;
				y2 = (v_size - y2) / 1;

				glTexCoord2f( left, top );
				glVertex2f( x1, y1 );
				glTexCoord2f( right, top );
				glVertex2f( x2, y1 );
				glTexCoord2f( right, bot );
				glVertex2f( x2, y2 );
				glTexCoord2f( left, bot );
				glVertex2f( x1, y2 );
				x += letter->Width * fstate->scaleX;
			}

			glEnd(); CHECKGL;
			RT_STAT_DRAW_TRIS(2)
		}
#endif
#if 0
		glDrawArrays(GL_QUADS,0,idx); CHECKGL;
#endif
	}

	glMatrixMode( GL_PROJECTION ); CHECKGL;
	glPopMatrix(); CHECKGL;
	WCW_SetProjectionMatrixDirty();
	glMatrixMode( GL_MODELVIEW ); CHECKGL;
	glPopMatrix(); CHECKGL;
	WCW_SetModelViewMatrixDirty();

	WCW_EnableGL_LIGHTING(1);
	glEnable( GL_DEPTH_TEST ); CHECKGL;
	WCW_Fog(1);
	modelDrawState(DRAWMODE_SPRITE, FORCE_SET);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), FORCE_SET);
	rdrEndMarker();
}

void rdrSetup2DRenderingDirect()
{
	int		v_size = rdr_view_state.height, h_size = rdr_view_state.width;
	Font	*last_font = 0;

	rdrBeginMarker(__FUNCTION__); 
	PERFINFO_AUTO_START("rdrSetup2DRendering", 1);
		glMatrixMode( GL_PROJECTION ); CHECKGL;
		glPushMatrix(); CHECKGL;
		glLoadIdentity(); CHECKGL;
		//		h_size = game_state.screen_x;
		//		v_size = game_state.screen_y;
		glOrtho( 0.0f, h_size, 0.0f, v_size, -1.0f, 100.0f ); CHECKGL;
		WCW_SetProjectionMatrixDirty();

		//Reset all texture state just in case something went haywire
		WCW_ClearTextureStateToWhite();

		glMatrixMode( GL_MODELVIEW ); CHECKGL;
		glPushMatrix(); CHECKGL;
		WCW_LoadModelViewMatrixIdentity();

		WCW_EnableGL_LIGHTING(0);
		WCW_Fog(0);
		glDisable( GL_DEPTH_TEST ); CHECKGL;
		WCW_DepthMask(1); 
		WCW_Color4(255, 255, 255, 255);

		WCW_DisableStencilTest();

		//	undotex2();
		modelDrawState(DRAWMODE_SPRITE, FORCE_SET);
		modelBlendState(BlendMode(BLENDMODE_MODULATE, game_state.useCelShader ? BMB_SINGLE_MATERIAL : 0), FORCE_SET);
	PERFINFO_AUTO_STOP();
	rdrEndMarker();
}

void rdrFinish2DRenderingDirect()
{
	rdrBeginMarker(__FUNCTION__); 
	PERFINFO_AUTO_START("rdrFinish2DRendering", 1);
		glMatrixMode( GL_PROJECTION ); CHECKGL;
		glPopMatrix(); CHECKGL;
		WCW_SetProjectionMatrixDirty();

		glMatrixMode( GL_MODELVIEW ); CHECKGL;
		glPopMatrix(); CHECKGL;
		WCW_SetModelViewMatrixDirty();

		WCW_EnableGL_LIGHTING(1);
		WCW_Fog(1);
		glEnable( GL_DEPTH_TEST ); CHECKGL;
		WCW_DepthMask(1); 
		WCW_Color4(255, 255, 255, 255);

		WCW_DisableStencilTest();

		modelDrawState(DRAWMODE_SPRITE, FORCE_SET);
		modelBlendState(BlendMode(BLENDMODE_MODULATE,0), FORCE_SET);
	PERFINFO_AUTO_STOP();
	rdrEndMarker();
}


