#include "timing.h"
#include "cmdgame.h"
#include "mathutil.h"
#include "wcw_statemgmt.h"

#define RT_PRIVATE
#include "rt_graphfps.h"
#include "rt_state.h"
#include "rt_font.h"
#include "rt_model_cache.h"
#include "rt_pbuffer.h"

Font* fontPtr(S32 font_index);

#define FRAME_HISTORY 256
#define FRAME_STEP (1.0f/60.0f*1000.0f)
#define FRAME_STEP_COUNT 6
#define FRAME_STRINGS 100
#define FRAME_STRING_TEXT (FRAME_STRINGS*32)

#define FRAME_INDEX(i) (((unsigned int)(i)) % ((unsigned int)FRAME_HISTORY))

typedef struct frameColorTable
{
	float limit;
	unsigned char color[4];
} frameColorTable;

typedef struct frameTimeText
{
	int count;
	int offset;
	FontMessage msgs[FRAME_STRINGS];
	char strings[FRAME_STRING_TEXT];
} frameTimeText;

typedef struct frameTimeGlobals
{
	unsigned int timer;
	unsigned int curFrame;
	float cpuFrames[FRAME_HISTORY];
	float gpuFrames[FRAME_HISTORY];
	float swapFrames[FRAME_HISTORY];
	float sliFrames[FRAME_HISTORY];
	frameTimeText text;
} frameTimeGlobals;

static frameTimeGlobals ft;

static void frameHistoryText(int x, int y, const unsigned char color[4], const char * s)
{
	FontMessage * msg;
	size_t length = strlen(s)+1;

	if((ft.text.count >= FRAME_STRINGS) || ((ft.text.offset + length) >= FRAME_STRING_TEXT)) {
		return;
	}

	msg = ft.text.msgs + ft.text.count;
	msg->StringIdx = ft.text.offset;
	msg->X = x;
	msg->Y = rdr_view_state.height-y;
	msg->state.font = fontPtr(0);
	msg->state.scaleX = 1.0f;
	msg->state.scaleY = 1.0f;
	memcpy(msg->state.rgba, color, sizeof(color));
	strcpy(ft.text.strings + ft.text.offset, s);

	ft.text.offset += length;
	ft.text.count++;
}

void rdrStartFrameHistory()
{
	if(!ft.timer) {
		ft.timer = timerAlloc();
	}

	ft.curFrame = FRAME_INDEX(ft.curFrame + 1);
	timerStart(ft.timer);
}

void rdrCpuFrameDone()
{
	if(!ft.timer) {
		ft.timer = timerAlloc();
	}

	ft.cpuFrames[ft.curFrame] = timerElapsed(ft.timer) * 1000.0f;
}

void rdrGpuFrameDone()
{
	if(!ft.timer) {
		ft.timer = timerAlloc();
	}

	ft.gpuFrames[ft.curFrame] = timerElapsed(ft.timer) * 1000.0f;
}

void rdrEndFrameHistory(int framesInProgress)
{
	if(!ft.timer) {
		ft.timer = timerAlloc();
	}

	ft.swapFrames[ft.curFrame] = timerElapsed(ft.timer) * 1000.0f;
	ft.sliFrames[ft.curFrame] = (float)framesInProgress;
}

static void rdrDrawCounter(const char * name, float * data, float y, float yh, float step, float steps, float legend, const frameColorTable * table)
{
	const unsigned char legendColor[4] = {0, 0, 0xFF, 0xFF};
	char buf[32];
	int i;
	float x = rdr_view_state.width - 20;
	float xs = -2;
	float ys = yh / steps;

	// Add title
	frameHistoryText(x + xs*FRAME_HISTORY, y, legendColor, name);

	// Add legend
	for(i=1; i<=steps; i++) {
		snprintf(buf, sizeof(buf), "%.0f", legend ? (legend/(i*step)) : (i*step));
		frameHistoryText(x, y + i*ys, legendColor, buf);
	}

	for(i=0; i<FRAME_HISTORY; i++) {
		int j;
		int cx, cy;
		bool isMin = true, isMax = true;
		float c, r;
		unsigned int curIndex = FRAME_INDEX(ft.curFrame - i);
		const frameColorTable * entry;

		c = data[curIndex];
		r = MIN(c / step, steps);
		cx = x + i*xs;
		cy = y + MAX(r*ys, yh);

		for(entry = table; (entry->limit != 0.0f) && (entry->limit < r); entry++);

		// Check if we are the current worst or best entry in the current search range
		for(j=MAX(i-30,0); j<=MIN(i+30,FRAME_HISTORY); j++) {
			float vs = data[FRAME_INDEX(ft.curFrame - j)];
			if(vs > c) {
				isMax = false;
			}
			if(vs < c) {
				isMin = false;
			}
		}
		if(isMax || isMin) {
			snprintf(buf, sizeof(buf), "%.2f", c);
			frameHistoryText(cx, cy, entry->color, buf);
		}

		WCW_Color4_Force(entry->color[0], entry->color[1], entry->color[2], entry->color[3]);
		glVertex2f(cx, y);
		glVertex2f(cx, cy);
	}
}

void rdrDrawFrameHistory()
{
	const frameColorTable fps_table[] = {
		{2.0f, {0, 0xFF, 0, 0xFF}},
		{3.0f, {0xFF, 0xFF, 0, 0xFF}},
		{0.0f, {0xFF, 0, 0, 0xFF}},
	};
	const frameColorTable sli_table[] = {
		{0.0f, {0, 0, 0xFF, 0xFF}},
	};
	int i;
	int count = 0;
	float top = rdr_view_state.height - 20;
	float bottom = 20;
	float border = 10;
	float y, yh;

	if(!game_state.graphfps) {
		return;
	}

	// Reset the font memory
	ft.text.count = 0;
	ft.text.offset = 0;
	
	modelBindDefaultBuffer();
	modelDrawState(DRAWMODE_SPRITE, FORCE_SET);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), FORCE_SET);

	WCW_EnableGL_LIGHTING(0);
	WCW_DisableBlend();
	WCW_Fog(0);
	WCW_ClearTextureStateToWhite();
	WCW_PrepareToDraw();

	glPushAttrib(GL_ALL_ATTRIB_BITS); CHECKGL;
	glDisable(GL_DEPTH_TEST); CHECKGL;
	glDisable(GL_ALPHA_TEST); CHECKGL;
	glMatrixMode(GL_PROJECTION ); CHECKGL;
	glLoadIdentity(); CHECKGL;
	glOrtho(0.0f, rdr_view_state.width, 0.0f, rdr_view_state.height, -1.0f, 1.0f); CHECKGL;
	glMatrixMode(GL_MODELVIEW); CHECKGL;
	glLoadIdentity(); CHECKGL;

	glBegin(GL_LINES);

	for(i = 0; i < GRAPHFPS_COUNTERS; i++) {
		if(game_state.graphfps & (1<<i)) {
			count++;
		}
	}

	y = top;
	yh = - (top - bottom - (border * count)) / (float)count;

	if(game_state.graphfps & GRAPHFPS_SHOW_SWAP) {
		rdrDrawCounter("SWAP", ft.swapFrames, y, yh, FRAME_STEP, FRAME_STEP_COUNT, 1000.0f, fps_table);
		y += yh - border;
	}

	if(game_state.graphfps & GRAPHFPS_SHOW_GPU) {
		rdrDrawCounter("GPU", ft.gpuFrames, y, yh, FRAME_STEP, FRAME_STEP_COUNT, 1000.0f, fps_table);
		y += yh - border;
	}

	if(game_state.graphfps & GRAPHFPS_SHOW_CPU) {
		rdrDrawCounter("CPU", ft.cpuFrames, y, yh, FRAME_STEP, FRAME_STEP_COUNT, 1000.0f, fps_table);
		y += yh - border;
	}

	// Display the current SLI depth
	if(game_state.graphfps & GRAPHFPS_SHOW_SLI) {
		char buf[32];
		snprintf(buf, sizeof(buf), "SLI (limit %d)", rdr_frame_state.sliLimit);

		rdrDrawCounter(buf, ft.sliFrames, y, yh, 1, MAX_FRAMES, 0.0f, sli_table);
		y += yh - border;
	}

	glEnd(); CHECKGL;
	glPopAttrib(); CHECKGL;

	rdrTextDirect(ft.text.count, ft.text.msgs, ft.text.strings);
}

