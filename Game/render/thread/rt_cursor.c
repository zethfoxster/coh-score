#define RT_PRIVATE
#include "rt_queue.h"
#include "ogl.h"
#include "wcw_statemgmt.h"
#include "rt_state.h"
#include "rt_pbuffer.h"
#include "rt_tex.h"
#include "rt_cursor.h"
#include "rt_stats.h"
#include "cmdgame.h"

PBuffer	cursor_pbuf;

void hwcursorClearDirect()
{
	pbufMakeCurrentDirect(&cursor_pbuf);
	glClearColor( 0,0,0,0 ); CHECKGL;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECKGL;
	winMakeCurrentDirect();
}

void hwcursorBlitDirect( RdrSpritesCmd *cmd)
{
	WCW_Fog(1); // Because we're going to call it in rdrInit anyway, but on a different context
	pbufMakeCurrentDirect(&cursor_pbuf);
 	WCW_EnableBlend();
	rdrInitDirect();

	WCW_LoadProjectionMatrixIdentity();
	if (rdr_caps.supports_pbuffers) {
		glOrtho( 0.0f, cursor_pbuf.virtual_width, 0, cursor_pbuf.virtual_height, -1.0f, 1.0f ); CHECKGL;
	} else {
		// Tricky projection to trick our sprites which are already tricked by GL to draw in the lower left
		glOrtho( 0.0f, rdr_view_state.origWidth, cursor_pbuf.virtual_height - cursor_pbuf.height, cursor_pbuf.virtual_height + rdr_view_state.origHeight - cursor_pbuf.height, -1.0f, 1.0f ); CHECKGL;
	}
	WCW_LoadModelViewMatrixIdentity();
	glDisable( GL_LIGHTING ); CHECKGL;
	glDisable( GL_FOG ); CHECKGL;

	modelDrawState(DRAWMODE_SPRITE, FORCE_SET);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), FORCE_SET);
	texSetWhite(TEXLAYER_GENERIC);

	drawSpriteDirect(cmd);

	winMakeCurrentDirect();
	// Reset these since the state management is now confused
	modelDrawState(DRAWMODE_SPRITE, FORCE_SET);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), FORCE_SET);
	WCW_PrepareToDraw();
}

