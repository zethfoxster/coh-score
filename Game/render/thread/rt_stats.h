#ifndef _RT_STATS_H
#define _RT_STATS_H

#include "renderstats.h"

#if RDRSTATS_ON

extern RTStats rt_stats;

enum
{
	STATS_CHANGE_BUFFER = 1,
	STATS_CHANGE_BLENDMODE = 2,
	STATS_CHANGE_VP = 4,
	
	STATS_TABLE_COLUMNS = 32,

	STATS_CHANGE_TEXTURE0 = 8,
	STATS_CHANGE_TEXTURE1 = 16,
	STATS_CHANGE_TEXTURE2 = 32,
	STATS_CHANGE_TEXTURE3 = 64,
	STATS_CHANGE_TEXTURE4 = 128,
	STATS_CHANGE_TEXTURE5 = 256,
	STATS_CHANGE_TEXTURE6 = 512,
	STATS_CHANGE_TEXTURE7 = 1024,
	STATS_CHANGE_TEXTURE8 = 2048,
};

#define RT_STAT_ADD(s,d) rt_stats.s += d

#define RT_STAT_INC(s) rt_stats.s++

#define RT_STAT_ADDMODEL(vbo)								\
{															\
	if (vbo->frame_id != rt_stats.frame_id)					\
	{														\
		RT_STAT_ADD(vert_unique_count,vbo->vert_count);		\
		RT_STAT_ADD(tri_unique_count,vbo->tri_count);		\
		RT_STAT_INC(model_unique_count);					\
		vbo->frame_id = rt_stats.frame_id;					\
	}														\
	RT_STAT_ADD(vert_count,vbo->vert_count);				\
	RT_STAT_INC(model_count);								\
}

#define RT_STAT_ADDSPRITE(num_tris)							\
{															\
}


#define RT_STAT_BUFFER_CHANGE								\
{															\
	RT_STAT_INC(buffer_changes);							\
	rt_stats.curmodel_changes |= STATS_CHANGE_BUFFER;		\
}

#define RT_STAT_TEXBIND_CHANGE(texlayer)					\
{															\
	RT_STAT_INC(texbind_changes);							\
	switch (texlayer)										\
	{														\
	xcase 0:												\
		rt_stats.curmodel_changes |= STATS_CHANGE_TEXTURE0;	\
	xcase 1:												\
		rt_stats.curmodel_changes |= STATS_CHANGE_TEXTURE1;	\
	xcase 2:												\
		rt_stats.curmodel_changes |= STATS_CHANGE_TEXTURE2;	\
	xcase 3:												\
		rt_stats.curmodel_changes |= STATS_CHANGE_TEXTURE3;	\
	xcase 4:												\
		rt_stats.curmodel_changes |= STATS_CHANGE_TEXTURE4;	\
	xcase 5:												\
		rt_stats.curmodel_changes |= STATS_CHANGE_TEXTURE5;	\
	xcase 6:												\
		rt_stats.curmodel_changes |= STATS_CHANGE_TEXTURE6;	\
	xcase 7:												\
		rt_stats.curmodel_changes |= STATS_CHANGE_TEXTURE7;	\
	xcase 8:												\
		rt_stats.curmodel_changes |= STATS_CHANGE_TEXTURE8;	\
	}														\
}

#define RT_STAT_BLEND_CHANGE								\
{															\
	RT_STAT_INC(blendstate_changes);						\
	rt_stats.curmodel_changes |= STATS_CHANGE_BLENDMODE;	\
}

#define RT_STAT_DRAW_CHANGE									\
{															\
	RT_STAT_INC(drawstate_changes);							\
}

#define RT_STAT_VP_CHANGE									\
{															\
	RT_STAT_INC(vp_changes);								\
	rt_stats.curmodel_changes |= STATS_CHANGE_VP;			\
}

#define RT_STAT_DRAW_TRIS(tricount)							\
{															\
	rdrStatFinishModel(tricount);							\
	RT_STAT_ADD(tri_count,tricount);						\
	rt_stats.curmodel_changes = 0;							\
	RT_STAT_INC(drawcalls);									\
}

void rdrStatFinishModel(int triCount);

void rdrStatsFrameStart();
void rdrStatsClear(int cards_wanted);
void rdrStatsGet(RTStats *dest);

#else

#define RT_STAT_ADD(s,d)
#define RT_STAT_INC(s)
#define RT_STAT_ADDMODEL(vbo)
#define RT_STAT_ADDSPRITE(num_tris)
#define RT_STAT_BUFFER_CHANGE
#define RT_STAT_TEXBIND_CHANGE
#define RT_STAT_BLEND_CHANGE
#define RT_STAT_DRAW_CHANGE
#define RT_STAT_VP_CHANGE
#define RT_STAT_FINISH_MODEL
#define RT_STAT_DRAW_TRIS(tricount)

#define rdrStatsFrameStart()
#define rdrStatsClear(cards_wanted)
#define rdrStatsGet(dest)

#endif

#endif //_RT_STATS_H
