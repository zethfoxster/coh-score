#ifndef _GRIDCACHE_H
#define _GRIDCACHE_H

#include "grid.h"
#include "gridcoll.h"
#include "ctri.h"

typedef struct
{
	Vec3	start,end;
	F32		radius;
	int		flags;
	int		find_type;
} CollParams;

int gridSetCacheDisabled(int disabled);
int gridCacheFind(CollParams *params,CollInfo *collp);
void gridCacheSet(CollParams *params,CollInfo *coll);
void gridCacheSetSize(int bits);

int objCacheFind(const Vec3 start,const Vec3 end,F32 radius,U32 flags,DefTracker **trackers,int *count);
void objCacheSet(Vec3 start,Vec3 end,U32 flags,DefTracker **trackers,int count);
int objCacheFitBlock(Vec3 start,Vec3 end,F32 *radius);
void objCacheSetSize(int bits);
void gridCacheReset();
void gridCacheInvalidate();

#endif
