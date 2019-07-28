#include <string.h>
#include "renderstats.h"
#include "font.h"
#include "cmdgame.h"
#include "memcheck.h"
#include "tex.h"
#include "entDebug.h"
#include "sprite_base.h"
#include "sprite_text.h"
#include "sprite_font.h"
#include "mathutil.h"
#include "renderprim.h"
#include "win_init.h"
#include "edit_cmd.h"
#include "timing.h"
#include "textureatlas.h"
#include "utils.h"
#include "perfcounter.h"

#if RDRSTATS_ON

#define RT_ALLOW_VBO
#include "rt_model_cache.h"

#define MAX_COST (12000.f)
#define MAX_COST_TOTAL (14000.f)

void rdrStatsDrawBar(float x, float y, float width, float height, float z, int stats, int card_idx, double maxCost);
char *rdrStatsCardName(int idx);

static unsigned int g_statsAccess[RENDERPASS_COUNT][RTSTAT_COUNT] = {
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
static RTStats g_statsDisplay[RENDERPASS_COUNT][RTSTAT_COUNT];  //stats container for displaying
RTStats g_statsGather[RENDERPASS_COUNT][RTSTAT_COUNT];          //stats container for gathering data
RenderStats render_stats, last_render_stats;

//containers to track stats for CPU cycles on the main thread
S64 g_statsGatherCyclesBegin[RENDERPASS_COUNT] = { 0, 0, 0, 0, 0, 0, 0, 0, 0};
S64 g_statsGatherCyclesTotal[RENDERPASS_COUNT] = { 0, 0, 0, 0, 0, 0, 0, 0, 0};
S64 g_statsDisplayCycles[RENDERPASS_COUNT] = { 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define STAT_DIST(p,s)   (g_statsDisplay[p][RTSTAT_DIST].s)
#define STAT_TYPE(p,s)   (g_statsDisplay[p][RTSTAT_TYPE].s)
#define STAT_UWATER(p,s) (g_statsDisplay[p][RTSTAT_UWATER].s)
#define STAT_SHADOW(p,s) (g_statsDisplay[p][RTSTAT_SHADOW].s)
#define STAT_OTHER(p,s)  (g_statsDisplay[p][RTSTAT_OTHER].s)
#define STAT_TOTAL(p,s) ( STAT_DIST(p,s) + \
                          STAT_TYPE(p,s) + \
                          STAT_UWATER(p,s) + \
                          STAT_SHADOW(p,s) + \
                          STAT_OTHER(p,s) )

#define COLOR_LOWHEAT   0x00FF00FF
#define COLOR_MIDHEAT   0xFFFF00FF
#define COLOR_HIHEAT    0xFF0000FF
#define COLOR_FULLHEAT  0xFF7F7FFF

#define COLOR_WHITE		0xFFFFFFFF

void rdrStatsBeginFrame(void)
{
    int p, s;

    rdrQueueCmd(DRAWCMD_FRAMESTART);

    //move our gathered stats from last frame into display struct
    for(p = 0; p < RENDERPASS_COUNT; p++)
    {
        bool bPassStatsChanged = false;
        bool bPassStatsZero = true;
        for(s = 0; s < RTSTAT_COUNT; s++)
        {
            bPassStatsZero &= !g_statsAccess[p][s];
            if(!g_statsAccess[p][s])
            {
                //never accesed previous frame, so zero us out
                memset(&g_statsGather[p][s], 0, sizeof(g_statsGather[p][s]));
                bPassStatsChanged = true;
            }
            if( g_statsGather[p][s].frame_id != g_statsDisplay[p][s].frame_id)
            {
                memcpy(&g_statsDisplay[p][s], &g_statsGather[p][s], sizeof(g_statsGather[p][s]));
                bPassStatsChanged = true;
            }
        }

        if(bPassStatsChanged)
        {
            //if none of the individual stats were gathered, display zero til at least one is captured
            g_statsDisplayCycles[p] = (bPassStatsZero) ? 0 : g_statsGatherCyclesTotal[p];
        }
    }

    memset(g_statsAccess, 0, sizeof(g_statsAccess));
}

void rdrStatsPassBegin(int pass)
{
    devassertmsg(pass >= RENDERPASS_UNKNOWN && pass < RENDERPASS_COUNT, "Passed an invalid pass!");
    g_statsGatherCyclesBegin[pass] = timerCpuTicks64();
}

void rdrStatsPassEnd(int pass)
{
    devassertmsg(pass >= RENDERPASS_UNKNOWN && pass < RENDERPASS_COUNT, "Passed an invalid pass!");
    devassertmsg(g_statsGatherCyclesBegin[pass] > 0, "rdrStatsPassEnd called without first calling rdrStatsPassBegin");

    g_statsGatherCyclesTotal[pass] = timerCpuTicks64() - g_statsGatherCyclesBegin[pass];
    g_statsGatherCyclesBegin[pass] = 0;
}


RTStats* rdrStatsGetPtr(ERTSTAT_TYPE type, int pass)
{
    devassertmsg(type < RTSTAT_COUNT, "Passed an invalid type!");

    if(pass >= RENDERPASS_UNKNOWN && pass < RENDERPASS_COUNT)
    {
        g_statsAccess[pass][type]++;
        return &g_statsGather[pass][type];
    }

    g_statsAccess[RENDERPASS_UNKNOWN][type]++;
    return &g_statsGather[RENDERPASS_UNKNOWN][type];
}


static unsigned int colorLerpComponent(unsigned int color0, unsigned int color1, float t, unsigned int mask, unsigned int shift)
{
	unsigned int c0, c1;
	unsigned int c;
	c0 = (color0 & mask) >> shift;
	c1 = (color1 & mask) >> shift;
	c = c0 * (1.f-t) + c1 * t;
	if (c > 0xFF)
		c = 0xFF;
	if (c < 0)
		c = 0;

	return c << shift;
}

unsigned int colorLerp(unsigned int color0, unsigned int color1, float t)
{
	int color = 0;

	if (t < 0)
		t = 0;
	if (t > 1)
		t = 1;

	color |= colorLerpComponent(color0, color1, t, 0xFF000000, 24);
	color |= colorLerpComponent(color0, color1, t, 0x00FF0000, 16);
	color |= colorLerpComponent(color0, color1, t, 0x0000FF00, 8);
	color |= colorLerpComponent(color0, color1, t, 0x000000FF, 0);

	return color;
}

static float fullHeatCycle = 0;
unsigned int getHeatColor(float heat, int cycle)
{
	if (heat > 1)
		heat = 1;

	if (heat < 0.6f)
		return COLOR_LOWHEAT;
	else if (heat < 0.8f)
		return colorLerp(COLOR_LOWHEAT, COLOR_MIDHEAT, (heat-0.6f) * 5.f);
	else if (heat < 1)
		return colorLerp(COLOR_MIDHEAT, COLOR_HIHEAT, (heat-0.8f) * 5.f);
	else if (cycle)
		return colorLerp(COLOR_HIHEAT, COLOR_FULLHEAT, 0.5f + (0.5f * sinf(fullHeatCycle - 1.5708f)));

	return COLOR_HIHEAT;
}


void drawProfilerBar(F32 x1, F32 y1, F32 x2, F32 y2, F32 depth, int color)
{
	display_sprite(white_tex_atlas,
		x1,
		y1,
		depth,
		(double)(x2-x1)/8.0,
		(double)(y2-y1)/8.0,
		color);
}

#define BORDER 2

void drawCostBar(float x, float y, float width, float height, float z, double cost, double maxCost)
{
	double heat;
	F32 x2;
	heat = cost / maxCost;

	drawProfilerBar(x,y,x+width,y+height,z++,0x000000ff);
	drawProfilerBar(x+BORDER-1,y+BORDER-1,x+width-(BORDER-1),y+height-(BORDER-1),z++,0xffffffff);
	drawProfilerBar(x+BORDER,y+BORDER,x+width-BORDER,y+height-BORDER,z++,0x7f7f7fff);

	width -= BORDER+BORDER;
	height -= BORDER+BORDER;
	y += BORDER;
	x += BORDER;

	if (heat >= 1)
		heat = 1;

	x2 = x + heat * width;
	drawProfilerBar(x,y,x2,y+height,z++,getHeatColor(heat, 1));
}

double drawBreakdownBar(float x, float y, float width, float height, float z, double cost1, double cost2, int color1, int color2, double maxCost)
{
	float x2;
	double heat;

	if (cost1 + cost2 > maxCost)
	{
		float mult = maxCost / (cost1 + cost2);
		cost1 *= mult;
		cost2 *= mult;
	}

	heat = (cost1 + cost2) / maxCost;

	drawProfilerBar(x,y,x+width,y+height,z++,0x000000ff);
	drawProfilerBar(x+BORDER-1,y+BORDER-1,x+width-(BORDER-1),y+height-(BORDER-1),z++,0xffffffff);
	drawProfilerBar(x+BORDER,y+BORDER,x+width-BORDER,y+height-BORDER,z++,0x7f7f7fff);

	width -= BORDER+BORDER;
	height -= BORDER+BORDER;

	y += BORDER;
	x += BORDER;

	x2 = x + (cost1 / maxCost) * width;
	drawProfilerBar(x,y,x2,y+height,z++,color1);

	x = x2;
	x2 = x + (cost2 / maxCost) * width;
	drawProfilerBar(x,y,x2,y+height,z++,color2);

	return heat;
}

static float cost_hist[640] = {0};
static int cost_hist_idx = 0;

static void drawCostGraph(int scale, int card_idx)
{

	int		i,idx;
	float	height;
	int		w, h;

    double total_cost = 0.0;

    for(i = 0; i <= RENDERPASS_COUNT; i++)
    {
        total_cost += STAT_TOTAL(i, costs[card_idx]);
    }

	cost_hist[cost_hist_idx++] = total_cost;
	if (cost_hist_idx >= ARRAY_SIZE(cost_hist))
		cost_hist_idx -= ARRAY_SIZE(cost_hist);

	if (!game_state.costgraph)
		return;

	fontSet(0);

	windowSize(&w, &h);

	for(i=0;i<ARRAY_SIZE(cost_hist);i++)
	{
		idx = cost_hist_idx + i;
		if (idx >= ARRAY_SIZE(cost_hist))
			idx -= ARRAY_SIZE(cost_hist);

		height = cost_hist[idx] / MAX_COST_TOTAL;

		drawLine(w - i, 0, w - i + 1, height * scale, getHeatColor(height, 0) >> 8 | 0xFF000000);
	}
}


static void printRenderStats(int offset_x, int offset_y, int line_height)
{
    char* labels[] = {
      //"Unknown",
        "Color",
        "Shadow1",
        "Shadow2",
        "Shadow3",
        "Shadow4",
        "Cubeface",
        "Reflect",
		"Reflect2",
        //"ImageCap",
    };
    char* labels_sub[] = {
        "opaque",
        "alpha",
        "uwater"
    };
    const int color_default = -1;
    const int sub_color_0 = 6;
    const int sub_color_1 = 4;
    const int color_highlight = 7;
    const int color_total = 7;
    const int color_hi = -1; // fpe disabled hi color until we figure out what's reasonable -- 10;
    const int col_sep = 60;
    const int num_sub_pass = (game_state.showrdrstats > 1) ? ARRAY_SIZE(labels_sub) : 0;
    const int num_val_lines = ARRAY_SIZE(labels) * (num_sub_pass + 1) + 1;
    const S64 cycles_per_ms = timerCpuSpeed64() / 1000;

    char buffer[1024] = "\0";
    int i, j;
    int total;
    int sum;

    font_set(&game_12, 0, 0, 1, 1, COLOR_WHITE, COLOR_WHITE);
    sprintf(buffer, "FPS: %6.2f", game_state.fps);
    printDebugString(buffer, offset_x, offset_y, 1.0f, line_height, color_default, 0, 255, NULL);

    //
    //print the labels
    //
    offset_y = 1 + (num_val_lines + 1) * line_height;
    printDebugString("Pass", offset_x, offset_y, 1.0f, line_height, color_default, 0, 255, NULL);
    offset_y -= line_height;

    for(i = 0; i < ARRAY_SIZE(labels); i++)
    {
        printDebugString(labels[i], offset_x, offset_y, 1.0f, line_height, color_default, 0, 255, NULL);
        offset_y -= line_height;
        
        for(j = 0; j < num_sub_pass; j++)
        {
            printDebugString(labels_sub[j], offset_x + 10, offset_y, 1.0f, line_height, (j & 0x01) ? sub_color_0 : sub_color_1, 0, 255, NULL);
            offset_y -= line_height;
        }
    }
    printDebugString("TOTAL", offset_x, offset_y, 1.0f, line_height, color_total, 0, 255, NULL);

    offset_x += col_sep;

#define PRINT_HEADER(h) \
    offset_y = 1 + (num_val_lines + 1) * line_height; \
    printDebugString((h), offset_x, offset_y, 1.0f, line_height, color_default, 0, 255, NULL); \
    offset_y -= line_height; 

#define PRINT_NUMBERS(s, hi) \
    sum = STAT_TOTAL(RENDERPASS_UNKNOWN, s); \
    for(i = 1; i < RENDERPASS_STATS_COUNT; i++) \
    { \
        total = STAT_TOTAL(i, s); \
        sum += total; \
        printDebugString(getCommaSeparatedInt(total), offset_x, offset_y, 1.0f, line_height, (total > hi) ? color_hi : color_default, 0, 255, NULL); \
        offset_y -= line_height * (num_sub_pass + 1); \
    } 

#define GET_STAT_TOTAL(idx, s, v) \
    total = v(idx, s); \

#define PRINT_NUMBERS_SUB(s) \
    if(game_state.showrdrstats > 1) \
    {  \
        offset_y = 1 + num_val_lines * line_height; \
        for(i = 1; i < RENDERPASS_STATS_COUNT; i++) \
        { \
            offset_y -= line_height; \
            GET_STAT_TOTAL(i, s, STAT_TYPE) \
            printDebugString(getCommaSeparatedInt(total), offset_x, offset_y, 1.0f, line_height, sub_color_1, 0, 255, NULL); \
            offset_y -= line_height; \
            GET_STAT_TOTAL(i, s, STAT_DIST) \
            printDebugString(getCommaSeparatedInt(total), offset_x, offset_y, 1.0f, line_height, sub_color_0, 0, 255, NULL); \
            offset_y -= line_height;\
            GET_STAT_TOTAL(i, s, STAT_UWATER) \
            printDebugString(getCommaSeparatedInt(total), offset_x, offset_y, 1.0f, line_height, sub_color_1, 0, 255, NULL); \
            offset_y -= line_height; \
        } \
    }

#define PRINT_FOOTER \
    printDebugString(getCommaSeparatedInt(sum), offset_x, offset_y, 1.0f, line_height, color_total, 0, 255, NULL); 

    //
    // sum stats for passes
    //
    PRINT_HEADER("ObjBone");
    PRINT_NUMBERS(bonemodel_drawn, 100);
    PRINT_NUMBERS_SUB(bonemodel_drawn);
    PRINT_FOOTER;

    offset_x += col_sep;

    PRINT_HEADER("ObjWorld");
    PRINT_NUMBERS(worldmodel_drawn, 1200);
    PRINT_NUMBERS_SUB(worldmodel_drawn);
    PRINT_FOOTER;

    offset_x += col_sep;

    PRINT_HEADER("ObjCloth");
    PRINT_NUMBERS(clothmodel_drawn, 5);
    PRINT_NUMBERS_SUB(clothmodel_drawn);
    PRINT_FOOTER;

    offset_x += col_sep;

    PRINT_HEADER("DrawCall");
    PRINT_NUMBERS(drawcalls, 1250);
    PRINT_NUMBERS_SUB(drawcalls);
    PRINT_FOOTER;

    offset_x += col_sep;

    PRINT_HEADER("Tris");
    PRINT_NUMBERS(tri_count, 30000);
    PRINT_NUMBERS_SUB(tri_count);
    PRINT_FOOTER;

    offset_x += col_sep;

    PRINT_HEADER("CPU (rt)");
    //PRINT_NUMBERS(cycles_cpu);
    //PRINT_NUMBERS_SUB(cycles_cpu);
   //PRINT_FOOTER;

    sum = STAT_TOTAL(RENDERPASS_UNKNOWN, cycles_cpu);
    for(i = 1; i < RENDERPASS_STATS_COUNT; i++) 
    { 
        S64 total64 = STAT_TOTAL(i, cycles_cpu); 
        sum += total64; 
        sprintf(buffer, "%6.3f", total64 / (F64) cycles_per_ms);
        printDebugString(buffer, offset_x, offset_y, 1.0f, line_height, color_default, 0, 255, NULL); 
        offset_y -= line_height;

        if(game_state.showrdrstats > 1)
        {
            total64 = STAT_TYPE(i,cycles_cpu);
            sprintf(buffer, "%6.3f", total64 / (F64) cycles_per_ms);
            printDebugString(buffer, offset_x, offset_y, 1.0f, line_height, sub_color_1, 0, 255, NULL); 
            offset_y -= line_height; 

            total64 = STAT_DIST(i,cycles_cpu);
            sprintf(buffer, "%6.3f", total64 / (F64) cycles_per_ms);
            printDebugString(buffer, offset_x, offset_y, 1.0f, line_height, sub_color_0, 0, 255, NULL); 
            offset_y -= line_height;

            total64 = STAT_UWATER(i,cycles_cpu);
            sprintf(buffer, "%6.3f", total64 / (F64) cycles_per_ms);
            printDebugString(buffer, offset_x, offset_y, 1.0f, line_height, sub_color_1, 0, 255, NULL); 
            offset_y -= line_height; 
        }
    } 

    sprintf(buffer, "%6.3f", sum / (F64) cycles_per_ms);
    printDebugString(buffer, offset_x, offset_y, 1.0f, line_height, color_total, 0, 255, NULL); 

    offset_x += col_sep;

    PRINT_HEADER("CPU (mt)");

    sum = g_statsDisplayCycles[RENDERPASS_UNKNOWN];
    for(i = 1; i < RENDERPASS_STATS_COUNT; i++) 
    { 
        S64 total64 = g_statsDisplayCycles[i]; 
        sum += total64; 
        sprintf(buffer, "%6.3f", total64 / (F64) cycles_per_ms);
        printDebugString(buffer, offset_x, offset_y, 1.0f, line_height, color_default, 0, 255, NULL); 
        offset_y -= line_height;

        offset_y -= line_height * num_sub_pass; 
    } 

    sprintf(buffer, "%6.3f", sum / (F64) cycles_per_ms);
    printDebugString(buffer, offset_x, offset_y, 1.0f, line_height, color_total, 0, 255, NULL); 

    //offset_x += col_sep;

    //perfCounterGetValue(PERF_COUNTER_GPU_BUSY, 0, &events, &sum);

    //PRINT_HEADER("GPU busy");
  //PRINT_NUMBERS(cycles_gpu);
    //PRINT_FOOTER;

#undef PRINT_HEADER
#undef PRINT_NUMBERS
#undef PRINT_NUMBERS_SUB
#undef PRINT_FOOTER
}

void rdrStatsDisplay()
{
	int			i,unique_tex=0,low_card;
	extern int	ctri_total;
	static int	heat_timer = 0;

    unsigned total_tris = 0;
    unsigned total_drawcalls = 0;
    unsigned total_tex_changes = 0;

    for(i = 0; i <= RENDERPASS_COUNT; i++)
    {
        total_tris += STAT_TOTAL(i, tri_count);
        total_drawcalls += STAT_TYPE(i, drawcalls);
        total_tex_changes += STAT_TOTAL(i, texbind_changes);
    }

	ADD_MISC_COUNT(total_tris, "Triangles Drawn");
	ADD_MISC_COUNT(total_drawcalls, "Draw Calls");
	ADD_MISC_COUNT(total_tex_changes, "Texture Changes");

	if (!heat_timer)
	{
		heat_timer = timerAlloc();
		timerStart(heat_timer);
	}
	else
	{
		fullHeatCycle += 10.f * timerElapsedAndStart(heat_timer);
		while (fullHeatCycle >= TWOPI)
			fullHeatCycle -= TWOPI;
	}

	if (game_state.info)
	{
        unsigned total_model = 0;
        unsigned total_vert = 0;
        unsigned total_vert_unique = 0;
        unsigned total_blend = 0;
        unsigned total_buff_call = 0;
        unsigned total_buff_change = 0;

        unsigned total_model_type = 0;
        unsigned total_tri_shadow = 0;

        unsigned total_drawnmodel_world = 0;
        unsigned total_drawnmodel_bone = 0;
        unsigned total_drawnmodel_cloth = 0;

		char buffer[1024];

        for(i = 0; i <= RENDERPASS_COUNT; i++)
        {
            total_model += STAT_TOTAL(i, model_count);
            total_model_type += STAT_TYPE(i, model_count);
            total_vert += STAT_TOTAL(i, vert_count);
            total_vert_unique += STAT_TOTAL(i, vert_unique_count);
            total_tri_shadow += STAT_SHADOW(i, tri_count);
            total_blend += STAT_TOTAL(i, blendstate_changes);
            total_buff_call += STAT_TOTAL(i, buffer_calls);
            total_buff_change += STAT_TOTAL(i, buffer_changes);
            total_drawnmodel_world += STAT_TOTAL(i, worldmodel_drawn);
            total_drawnmodel_bone += STAT_TOTAL(i, bonemodel_drawn);
            total_drawnmodel_cloth += STAT_TOTAL(i, clothmodel_drawn); 
        }

		sprintf(buffer, "O %d/%d  P % 4d  V %4d/%d/%d  SP % 4d  BC %d Tex: %d.%03dM/%d.%03dM Recent: %d.%03dM (%d) Queued: %d  VBOChanges: %d/%d",
			total_model, total_model_type,
			total_tris /* + STAT_TOTAL(shadow_tri_count)*/,
			total_vert,total_vert_unique,ctri_total,
			total_tri_shadow,
			total_blend,
			render_stats.texture_usage_map/1000000, render_stats.texture_usage_map/1000 % 1000,
			render_stats.texture_usage_total/1000000, render_stats.texture_usage_total/1000 % 1000,
			render_stats.texture_usage_recent_size/1000000, render_stats.texture_usage_recent_size/1000 % 1000,
			render_stats.texture_usage_recent_num,
			texLoadsPending(1),
			total_buff_call, total_buff_change);

		//xyprintf(0,480/8-1 + TEXT_JUSTIFY,"%s");
		printDebugString(buffer, 1, 1, 1, 11, -1, 0, 255, NULL);
		xyprintf(5,24,"%d %d %d",total_drawnmodel_world,total_drawnmodel_bone,total_drawnmodel_cloth);
	}
	if (game_state.tri_hist)
	{
		for(i=0;i<20;i++)
		{
			xyprintf(i * 5,480/8-4 + TEXT_JUSTIFY,"%d",1 << i);
			xyprintf(i * 5,480/8-3 + TEXT_JUSTIFY,"%d",render_stats.tri_histo[i]);
		}
		for(i=0;i<ARRAY_SIZE(render_stats.tex_histo);i++)
		{
			if (render_stats.tex_histo[i])
				unique_tex++;
		}
		xyprintf(0,480/8-5 + TEXT_JUSTIFY,"Unique tex %d",unique_tex);
	}

	low_card = -1;
	for (i = 0; i < STATS_NUM_CARDS; i++)
	{
		int card = 1 << i;
		if (game_state.stats_cards & card)
		{
			low_card = i;
			break;
		}
	}

	if (game_state.showcomplexity)
	{
        char* labels[] = {
            "Cost(in ms):",
            "Vert Program Changes:",
            "Draw Mode Changes:",
            "Blend Mode Changes:",
            "Buffer Changes:",
            "Texture binds[9]:",
            "Texture binds[8]:",
            "Texture binds[7]:",
            "Texture binds[6]:",
            "Texture binds[5]:",
            "Texture binds[4]:",
            "Texture binds[3]:",
            "Texture binds[2]:",
            "Texture binds[1]:",
            "Texture binds[0]:",
            "Texture Changes:",
            "Texture Calls:",
            "Draw calls:",
            "Verts:",
            "Triangles:",
            "Models:",
        };
        const int offset_total = 144;
        const int offset_spacing = 60;
        const int line_height = 11;
        const int default_color = -1;
        const int color_total = 8;
        const int category_color = 5;

		char buffer[1024];
		int y = 1;
        int j = 0;
        int t = 0;

        float total_cost[RTSTAT_COUNT] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        unsigned totals[ARRAY_SIZE(labels) - 1][RTSTAT_COUNT];
        memset(totals, 0, sizeof(totals));


        font_set(&game_12, 0, 0, 1, 1, COLOR_WHITE, COLOR_WHITE);

        //
        //print the labels
        //

        for(i = (low_card >= 0) ? 0 : 1; i < ARRAY_SIZE(labels); i++)
        {
            printDebugString(labels[i], 1, y, 1, line_height, default_color, 0, 255, NULL);
            y += line_height;
        }

        //
        // sum each category into totals
        //
        if(low_card >= 0)
        {
            for(i = 0; i < RENDERPASS_COUNT; i++)
                for(j = 0; j < RTSTAT_COUNT; j++)
                    total_cost[j] += g_statsDisplay[i][j].costs[low_card] / 1000.f;
        }
        for(i = 0; i < RENDERPASS_COUNT; i++)
        {
            for(j = 0; j < RTSTAT_COUNT; j++)
            {
                //NOTE: has to remain in same order as labels
                totals[0][j] += g_statsDisplay[i][j].vp_changes;
                totals[1][j] += g_statsDisplay[i][j].drawstate_changes;
                totals[2][j] += g_statsDisplay[i][j].blendstate_changes;
                totals[3][j] += g_statsDisplay[i][j].buffer_changes;

                for(t = 0; t < MAX_TEX_BINDS; t++)
                    totals[4 + MAX_TEX_BINDS - t - 1][j] += g_statsDisplay[i][j].drawcall_texbinds[t];

                totals[4 + MAX_TEX_BINDS][j] += g_statsDisplay[i][j].texbind_changes;
                totals[5 + MAX_TEX_BINDS][j] += g_statsDisplay[i][j].texbind_calls;
                totals[6 + MAX_TEX_BINDS][j] += g_statsDisplay[i][j].drawcalls;
                totals[7 + MAX_TEX_BINDS][j] += g_statsDisplay[i][j].vert_count;
                totals[8 + MAX_TEX_BINDS][j] += g_statsDisplay[i][j].tri_count;
                totals[9 + MAX_TEX_BINDS][j] += g_statsDisplay[i][j].model_count;
            }
        }

        //
        // print the totals column
        //
        y = 1;

        if(low_card >= 0)
		{
            sprintf(buffer, "%.3f", total_cost[0] + total_cost[1] + total_cost[2] + total_cost[3] + total_cost[4]);
            printDebugString(buffer, offset_total, y, 1.0f, line_height, color_total, 0, 255, NULL);
            y += line_height;
		}

        for(i = 0; i < ARRAY_SIZE(labels) - 1; i++)
        {
            unsigned display_total = 0; 
            for(j = 0; j < RTSTAT_COUNT; j++)
                display_total += totals[i][j];

            _itoa(display_total, buffer, 10);
            printDebugString(buffer, offset_total, y, 1.0f, line_height, color_total, 0, 255, NULL);
            y += line_height;
        }

        //
        // print the individual category columns
        //
        for(i = 0; i < RTSTAT_COUNT; i++)
        {
            y = 1;

            if(low_card >= 0)
            {
                sprintf(buffer, "%.3f", total_cost[i]);
                printDebugString(buffer, offset_total + offset_spacing * (i + 1), y, 1.0f, line_height, category_color, 0, 255, NULL);
                y+= line_height;
            }

            for(j = 0; j < ARRAY_SIZE(labels) - 1; j++)
            {
                _itoa(totals[j][i], buffer, 10);
                printDebugString(buffer, offset_total + offset_spacing * (i + 1), y, 1.0f, line_height, category_color, 0, 255, NULL);
                y+= line_height;
            }

        }

        font_set(&game_12, 0, 0, 1, 1, COLOR_WHITE, COLOR_WHITE);
		sprintf(buffer, "FPS: %6.2f                   Total   | Opaque | Alpha  | A-Water | Shadow | Other", game_state.fps);
		printDebugString(buffer, 1, y, 1, line_height, -1, 0, 255, NULL);
	}

    if (game_state.showrdrstats)
    {
        printRenderStats(1, 1, 11);
        //perfCounterDisplay(1, 150, 11);
    }
    else
    {
        //perfCounterStop();
    }

	if (!editMode() && game_state.costbars && low_card >= 0)
	{
		font_set(&game_12, 0, 0, 1, 1, COLOR_WHITE, COLOR_WHITE);
		cprntEx(300, 32, 1, 1, 1, 0, "Total");
		rdrStatsDrawBar(350, 20, 250, 15, 1, RSTAT_ALL, low_card, MAX_COST_TOTAL);
		cprntEx(300, 52, 1, 1, 1, 0, "Type");
		rdrStatsDrawBar(350, 40, 250, 15, 1, RSTAT_TYPE, low_card, MAX_COST_TOTAL);
		cprntEx(300, 72, 1, 1, 1, 0, "Dist");
		rdrStatsDrawBar(350, 60, 250, 15, 1, RSTAT_DIST, low_card, MAX_COST_TOTAL);
	    cprntEx(300, 92, 1, 1, 1, 0, "UnderWater");
	    rdrStatsDrawBar(350, 80, 250, 15, 1, RSTAT_UWATER, low_card, MAX_COST_TOTAL);
		cprntEx(300, 112, 1, 1, 1, 0, "Shadow");
		rdrStatsDrawBar(350, 100, 250, 15, 1, RSTAT_SHADOW, low_card, MAX_COST_TOTAL);
		cprntEx(300, 132, 1, 1, 1, 0, "Other");
		rdrStatsDrawBar(350, 120, 250, 15, 1, RSTAT_OTHER, low_card, MAX_COST_TOTAL);
	}
	else if (!editMode() && game_state.stats_cards)
	{
		int y = 20;
		for (i = 0; i < STATS_NUM_CARDS; i++)
		{
			int card = 1 << i;
			if (!(game_state.stats_cards & card))
				continue;
			font_set(&game_12, 0, 0, 1, 1, COLOR_WHITE, COLOR_WHITE);
			cprntEx(250, y+12, 1, 1, 1, 0, rdrStatsCardName(i));
			rdrStatsDrawBar(350, y, 250, 15, 1, RSTAT_TYPE | RSTAT_DIST | RSTAT_UWATER, i, MAX_COST);
			y += 20;
		}
	}

	drawCostGraph(40, low_card);

	CopyStructs(&last_render_stats, &render_stats, 1);
	ZeroStruct(&render_stats);
}

void rdrStatsDrawBar(float x, float y, float width, float height, float z, int stats, int card_idx, double maxCost)
{
	double cost = 0;
    int i;

	if (stats & RSTAT_TYPE)
	{
        for(i = 0; i < RENDERPASS_COUNT; i++)
		    cost += STAT_TYPE(i, costs[card_idx]);
	}

	if (stats & RSTAT_DIST)
	{
        for(i = 0; i < RENDERPASS_COUNT; i++)
    		cost += STAT_DIST(i, costs[card_idx]);
	}

	if (stats & RSTAT_UWATER)
	{
        for(i = 0; i < RENDERPASS_COUNT; i++)
	    	cost += STAT_UWATER(i, costs[card_idx]);
	}

	if (stats & RSTAT_SHADOW)
	{
        for(i = 0; i < RENDERPASS_COUNT; i++)
    		cost += STAT_SHADOW(i, costs[card_idx]);
	}

	if (stats & RSTAT_OTHER)
	{
        for(i = 0; i < RENDERPASS_COUNT; i++)
	    	cost += STAT_OTHER(i, costs[card_idx]);
	}

	// draw the bar
	drawCostBar(x, y, width, height, z, cost, maxCost);
}

void rdrStatsAddCard(char *name)
{
	int i;
	for (i = 0; i < STATS_NUM_CARDS; i++)
	{
		if (simpleMatch(name, rdrStatsCardName(i)))
			game_state.stats_cards |= 1 << i;
	}
}

void rdrStatsRemoveCard(char *name)
{
	int i;
	for (i = 0; i < STATS_NUM_CARDS; i++)
	{
		if (simpleMatch(name, rdrStatsCardName(i)))
			game_state.stats_cards &= ~(1 << i);
	}
}

static SimpleStats rdr_stats_minspec, rdr_stats_current;

void rdrStatsClearLevelCost(void)
{
	ZeroStruct(&rdr_stats_minspec);
	ZeroStruct(&rdr_stats_current);
}

void rdrStatsAddLevelModel(VBO *vbo, int ok_in_minspec)
{
	rdr_stats_current.num_models++;
	rdr_stats_current.num_objects += vbo->tex_count;
	rdr_stats_current.num_tris += vbo->tri_count;
	rdr_stats_current.obj_cost += vbo->tex_count * 7.688888;
	rdr_stats_current.tri_cost += vbo->tri_count * 0.278472;

	if (ok_in_minspec)
	{
		rdr_stats_minspec.num_models++;
		rdr_stats_minspec.num_objects += vbo->tex_count;
		rdr_stats_minspec.num_tris += vbo->tri_count;
		rdr_stats_minspec.obj_cost += vbo->tex_count * 7.688888;
		rdr_stats_minspec.tri_cost += vbo->tri_count * 0.278472;
	}
}

void rdrStatsGetLevelCost(SimpleStats *cur, SimpleStats *minspec)
{
	if (cur)
	{
		CopyStructs(cur, &rdr_stats_current, 1);
	}

	if (minspec)
	{
		CopyStructs(minspec, &rdr_stats_minspec, 1);
	}
}

#endif
