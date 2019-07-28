#include "uiRenderStats.h"
#include "uiWindows.h"
#include "uiUtilGame.h"
#include "uiUtil.h"
#include "uiInput.h"

#include "mathutil.h"
#include "win_init.h"

#include "tex.h"
#include "textureatlas.h"
#include "ttFontUtil.h"
#include "sprite_base.h"
#include "sprite_text.h"
#include "sprite_font.h"

#include "cmdgame.h"
#include "renderstats.h"
#include "timing.h"

#if RDRSTATS_ON

unsigned int getHeatColor(float heat, int cycle);
void drawProfilerBar(F32 x1, F32 y1, F32 x2, F32 y2, F32 depth, int color);
double drawBreakdownBar(float x, float y, float width, float height, float z, double cost1, double cost2, int color1, int color2, double maxCost);

#define COLOR_OBJCOST   0x376FEFFF
#define COLOR_TRICOST   0xAF8FFFFF
#define COLOR_BLACK		0x000000FF
#define COLOR_GREY		0x7F7F7FFF
#define COLOR_WHITE		0xFFFFFFFF

#define MAX_LEVEL_COST (17000.f)

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

#define STATFRAME_HT 70
#define DEBUGFRAME_HT 95
#define STATUSFRAME_HT 30

void rdrStatsWindow_StatFrame(SimpleStats *stats, float x, float y, float z, float wd, float ht, float sc, int color, int bcolor, double maxCost, int isMinSpec, char *title)
{
	// simple stats for artists
	double heat;

	drawFrame(PIX3, R10, x, y, z-2, wd, ht, sc, color, bcolor);

	x+=10*sc;
	y+=10*sc;

	font_set(&game_12, 0, 0, 1, 1, 0x0070ffff, 0x00ffffff);
	cprntEx(x, y+8*sc, z, sc, sc, 0, "%s", title);

	font_set(&game_12, 0, 0, 1, 1, COLOR_OBJCOST, COLOR_OBJCOST);
	cprntEx(x, y+22*sc, z, sc, sc, 0, "Objects drawn: %d  (Cost: %d)", stats->num_objects, (int)stats->obj_cost);

	font_set(&game_12, 0, 0, 1, 1, COLOR_WHITE, COLOR_WHITE);
	cprntEx(x+230*sc, y+22*sc, z, sc, sc, 0, "Models: %d", stats->num_models);

	font_set(&game_12, 0, 0, 1, 1, COLOR_TRICOST, COLOR_TRICOST);
	cprntEx(x, y+35*sc, z, sc, sc, 0, "Triangles drawn: %d  (Cost: %d)", stats->num_tris, (int)stats->tri_cost);

	heat = drawBreakdownBar(x, y+sc*38, 250*sc, 15*sc, z, stats->obj_cost, stats->tri_cost, COLOR_OBJCOST, COLOR_TRICOST, maxCost);


	x += 255*sc;
	y += 23*sc;

	if (isMinSpec)
	{
		float x2 = x + sc*50;
		float y2 = y + sc*30;

		drawProfilerBar(x++, y++, x2--, y2--, z++, COLOR_BLACK);
		drawProfilerBar(x++, y++, x2--, y2--, z++, COLOR_WHITE);
		drawProfilerBar(x++, y++, x2--, y2--, z++, COLOR_GREY);

		{
			float hw = (x2-x) * 0.5f;
			float hh = (y2-y) * 0.5f;

			x += hw;
			y += hh;
			hw *= heat;
			hh *= heat;
			x2 = x + hw;
			y2 = y + hh;
			x -= hw;
			y -= hh;
			drawProfilerBar(x, y, x2, y2, z++, getHeatColor(heat,1));
		}

		if (heat >= 1)
		{
			font_set(&game_12, 0, 0, 1, 1, COLOR_WHITE, COLOR_WHITE);
			cprntEx((x+x2)*0.5f, (y+y2)*0.5f, z++, sc, sc, CENTER_X|CENTER_Y, "OVER!");
		}
	}

}

void rdrStatsWindow_DebugFrame(float x, float y, float z, float wd, float ht, float sc, int color, int bcolor)
{
	int color2;
	int buttonwidth = sc * 100, buttonheight = sc * 18;

	drawFrame(PIX3, R10, x, y, z-2, wd, ht, sc, color, bcolor);

	color = 0x7f7f7fff;
	color2 = 0xffffffff;

	x+=10*sc;
	y+=10*sc;

	font_set(&game_12, 0, 0, 1, 1, COLOR_WHITE, COLOR_WHITE);
	if (D_MOUSEHIT == drawStdButton(x + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, color, "Graphics HUD", sc, 0))
		cmdParse("gfxhud");

	if (D_MOUSEHIT == drawStdButton(x + buttonwidth + 5*sc + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, game_state.oldVideoTest?color2:color, game_state.oldVideoTest?"Normal Mode":"Geforce 4 Mode", sc, 0))
		cmdParse("++oldvid");

	if (D_MOUSEHIT == drawStdButton(x + buttonwidth + 5*sc + buttonwidth + 5*sc + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, (game_state.vis_scale==0.5f)?color2:color, "Vis Scale 0.5", sc, 0))
		cmdParse("visscale 0.5");

	y += buttonheight + 10*sc;

	if (D_MOUSEHIT == drawStdButton(x + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, game_state.tintOccluders?color2:color, game_state.tintOccluders?"Untint Occluders":"Tint Occluders", sc, 0))
	{
		cmdParse("++showoccluders");
		if (game_state.tintOccluders)
			cmdParse("showalpha 0");
	}

	if (D_MOUSEHIT == drawStdButton(x + buttonwidth + 5*sc + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, game_state.tintAlphaObjects?color2:color, game_state.tintAlphaObjects?"Untint Alpha Sorted":"Tint Alpha Sorted", sc, 0))
	{
		cmdParse("++showalpha");
		if (game_state.tintAlphaObjects)
			cmdParse("showoccluders 0");
	}

	if (D_MOUSEHIT == drawStdButton(x + buttonwidth + 5*sc + buttonwidth + 5*sc + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, (game_state.vis_scale==1.0f)?color2:color, "Vis Scale 1.0", sc, 0))
		cmdParse("visscale 1.0");

	y += buttonheight + 10*sc;

	if (!game_state.zocclusion_debug)
	{
		if (D_MOUSEHIT == drawStdButton(x + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, color, "Show Occluders", sc, 0))
			cmdParse("zodebug 4");
	}
	else
	{
		if (D_MOUSEHIT == drawStdButton(x + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, color2, "Hide Occluders", sc, 0))
			cmdParse("zodebug 0");
	}

	if (D_MOUSEHIT == drawStdButton(x + buttonwidth + 5*sc + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, game_state.no_welding?color2:color, game_state.no_welding?"Enable Welding":"Disable Welding", sc, 0))
		cmdParse("++nowelding");

	if (D_MOUSEHIT == drawStdButton(x + buttonwidth + 5*sc + buttonwidth + 5*sc + buttonwidth/2, y + buttonheight/2, z, buttonwidth, buttonheight, (game_state.vis_scale==2.0f)?color2:color, "Vis Scale 2.0", sc, 0))
		cmdParse("visscale 2.0");
}

void rdrStatsWindow_StatusFrame(float x, float y, float z, float wd, float ht, float sc, int color, int bcolor)
{
	drawFrame(PIX3, R10, x, y, z-2, wd, ht, sc, color, bcolor);

	x+=10*sc;
	y+=10*sc;

	font_set(&game_12, 0, 0, 1, 1, COLOR_WHITE, COLOR_WHITE);
	cprntEx(x, y+11*sc, z, sc, sc, 0, "Textures: %d.%03dM", last_render_stats.texture_usage_map/1048576, last_render_stats.texture_usage_map/1024 % 1000);
	cprntEx(x+220*sc, y+11*sc, z, sc, sc, 0, "Vis Scale: %.1f", game_state.vis_scale);
}

int rdrStatsWindow(void)
{
	float x, y, z, wd, ht, sc;
	int w, h, color, bcolor, viewht = 0;
	static SimpleStats curstats, minstats;

	windowClientSizeThisFrame(&w, &h);  
	if (!window_getDims( WDW_RENDER_STATS, &x, &y, &z, &wd, &ht, &sc, &color, &bcolor))
		return 0;

	rdrStatsGetLevelCost(&curstats, &minstats);
	rdrStatsClearLevelCost();

	{
		float cury = y;
		char title[1024];
		if (game_state.draw_scale == MIN_SPEC_DRAWSCALE)
			sprintf(title, "Min Spec world geometry (Vis Scale: %.2f)", MIN_SPEC_DRAWSCALE);
		else
			sprintf(title, "Min Spec world geometry estimate (Vis Scale: %.2f)", MIN_SPEC_DRAWSCALE);
		rdrStatsWindow_StatFrame(&minstats, x, cury, z, wd, STATFRAME_HT*sc, sc, color, bcolor, MAX_LEVEL_COST, 1, title);
		cury += (STATFRAME_HT-PIX3)*sc;
		rdrStatsWindow_DebugFrame(x, cury, z, wd, DEBUGFRAME_HT*sc, sc, color, bcolor);
		cury += (DEBUGFRAME_HT-PIX3)*sc;
		rdrStatsWindow_StatusFrame(x, cury, z, wd, STATUSFRAME_HT*sc, sc, color, bcolor);
	}

	return 0;
}

#endif
