#ifndef _RENDERSTATS_H
#define _RENDERSTATS_H

#if !defined(SERVER) && !defined(FINAL)
#define RDRSTATS_ON 1
#endif

#define MIN_SPEC_DRAWSCALE 0.5f

#if RDRSTATS_ON

#define STATS_NUM_CARDS 10
#define MAX_TEX_BINDS   10

typedef struct
{
	int		curmodel_changes;

	int		bonemodel_drawn;
	int		worldmodel_drawn;
	int		clothmodel_drawn;

	int		model_count;
	int		model_unique_count;

	int		tri_count;
	int		tri_unique_count;

	int		vert_count;
	int		vert_unique_count;

	int		buffer_changes;
	int		buffer_calls;

	int		texbind_changes;
	int		texbind_calls;

	int		blendstate_changes;
	int		blendstate_calls;

	int		vp_changes;

	int		drawstate_changes;
	int		drawstate_calls;

	int		drawcalls;

	int		frame_id;

	int		drawcall_texbinds[MAX_TEX_BINDS];

	double	costs[STATS_NUM_CARDS];

    S64     cycles_cpu;             //# of CPU cycles between clear and get of stats
//  S64     cycles_gpu;             //# of GPU cycles between clear and get of stats
} RTStats;

typedef struct
{
	int		vert_access_count;
	int		vert_lock_count;
	int		model_lock_count;
	int		tri_histo[128];
	int		texload_count;
	char	tex_histo[16384];

	int		texture_usage_map;
	int		texture_usage_total;
	int		texture_usage_recent_num;
	int		texture_usage_recent_size;

} RenderStats;

extern RenderStats render_stats, last_render_stats;

typedef enum
{
    RTSTAT_TYPE = 0,
    RTSTAT_DIST,
    RTSTAT_UWATER,
    RTSTAT_SHADOW,
    RTSTAT_OTHER,
    RTSTAT_COUNT
} ERTSTAT_TYPE;

enum
{
	RSTAT_TYPE   = 1 << RTSTAT_TYPE,
	RSTAT_DIST   = 1 << RTSTAT_DIST,
	RSTAT_UWATER = 1 << RTSTAT_UWATER,
	RSTAT_SHADOW = 1 << RTSTAT_SHADOW,
	RSTAT_OTHER  = 1 << RTSTAT_OTHER,
	RSTAT_ALL    = (1 << RTSTAT_COUNT) - 1
};

void rdrStatsBeginFrame(void);
void rdrStatsPassBegin(int pass);
void rdrStatsPassEnd(int pass);
RTStats* rdrStatsGetPtr(ERTSTAT_TYPE type, int pass);

void rdrStatsDisplay(void);
void rdrStatsAddCard(char *name);
void rdrStatsRemoveCard(char *name);

typedef struct VBO VBO;

typedef struct SimpleStats
{
	int num_models;
	int num_objects;
	int num_tris;
	float obj_cost;
	float tri_cost;
} SimpleStats;


void rdrStatsClearLevelCost(void);
void rdrStatsAddLevelModel(VBO *vbo, int ok_in_minspec);
void rdrStatsGetLevelCost(SimpleStats *cur, SimpleStats *minspec);

#define RDRSTAT_SET(s,d) render_stats.s = d
#define RDRSTAT_GET(s) render_stats.s
#define RDRSTAT_ADD(s,d) render_stats.s += d
#define RDRSTAT_INC(s) render_stats.s++
#define RDRSTAT_TEXLOAD(id) \
	RDRSTAT_ADD(texload_count,1); \
	RDRSTAT_ADD(tex_histo[id],1);

#else

typedef int RTStats;
typedef int RenderStats;

#define rdrStatsGetPtr(type, pass) ((RTStats*)NULL)
#define rdrStatsBeginFrame()
#define rdrStatsPassBegin(pass)
#define rdrStatsPassEnd(pass)
#define rdrStatsDisplay()
#define rdrStatsAddCard(name)
#define rdrStatsRemoveCard(name)

#define RDRSTAT_SET(s,d)
#define RDRSTAT_GET(s) 0
#define RDRSTAT_ADD(s,d)
#define RDRSTAT_INC(s)
#define RDRSTAT_TEXLOAD(id)

#endif //RDRSTATS_ON

#endif //_RENDERSTATS_H
