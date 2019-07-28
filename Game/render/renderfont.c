#include "truetype/ttDebug.h"
#include "rt_font.h"
#include "sprite.h"
#include "cmdgame.h"
#include "rt_queue.h"
#include "timing.h"
#include "uiGame.h"

void rdrRenderEditor( FontMessage messages[], S32 message_count, int textbuf_count, char *textbuf )
{
	RdrDbgFont	*pkg;
	int			msgs_size;
	U8			*text_base;

	if(!message_count)
		return;
	msgs_size = sizeof(FontMessage) * message_count;
	pkg = rdrQueueAlloc(DRAWCMD_SIMPLETEXT,sizeof(RdrDbgFont) + msgs_size + textbuf_count);
	pkg->count = message_count;
	memcpy(pkg+1,messages,msgs_size);
	text_base = ((U8*)(pkg+1)) + msgs_size;
	memcpy(text_base,textbuf,textbuf_count);
	rdrQueueSend();
}

void rdrSetup2DRendering()
{
	rdrQueueCmd(DRAWCMD_SETUP2D);
}

void rdrFinish2DRendering()
{
	rdrQueueCmd(DRAWCMD_FINISH2D);
}

void rdrRenderGame()
{
	rdrSetup2DRendering();

	if( !ttDebugEnabled() && !game_state.disableGameGUI) 
	{
		PERFINFO_AUTO_START("drawAllSprites", 1);
			drawAllSprites();	// Draw all sprites and strings.
		PERFINFO_AUTO_STOP();
	}
	else
	{
		ttDebug();
	}

	rdrFinish2DRendering();
}


