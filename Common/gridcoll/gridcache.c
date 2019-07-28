#include <string.h>
#include "mathutil.h"
#include "group.h"
#include "grid.h"
#include "gridcoll.h"
#include "gridcache.h"
#include "StashTable.h"
#include "HashFunctions.h"

static int coll_cache_bits;
#define COLL_CACHE_SIZE (size_t)(1 << coll_cache_bits)
#define COLL_CACHE_MASK (COLL_CACHE_SIZE -1)

static int obj_cache_bits;
#define OBJ_CACHE_SIZE (size_t)(1 << obj_cache_bits)
#define OBJ_CACHE_MASK (OBJ_CACHE_SIZE -1)

static int grid_cache_disabled;

typedef struct
{
	U32			flags;
	U32			valid_id;
	Vec3		start;
	F32			end;
	DefTracker	*trackers[64-7];
	int			count;
} ObjCache;

ObjCache	*obj_cache;
CollInfo	*coll_cache;
CollParams	*param_cache;

extern Grid obj_grid;
int grid_cache_find,objcache_cant_fit;

int gridSetCacheDisabled(int disabled){
	int old = grid_cache_disabled;
	
	grid_cache_disabled = disabled;
	
	return old;
}

int gridCacheFind(CollParams *params,CollInfo *collp)
{
	U32			hash;
	CollInfo	*ccoll;

	if (grid_cache_disabled || !coll_cache_bits)
		return 0;
	hash = hashCalc((void*)params, sizeof(*params), 0xa74d94e3);

	if (memcmp(&param_cache[hash & COLL_CACHE_MASK],params,sizeof(*params))!=0)
		return 0;
	ccoll = &coll_cache[hash & COLL_CACHE_MASK];

	if (ccoll->valid_id == obj_grid.valid_id)
	{
		*collp = *ccoll;
		grid_cache_find++;
		return 1;
	}
	return 0;
}

void gridCacheSet(CollParams *params,CollInfo *coll)
{
	U32			hash;

	if (grid_cache_disabled || !coll_cache_bits)
		return;
	hash = hashCalc((void*)params, sizeof(*params), 0xa74d94e3);
	memcpy(&param_cache[hash & COLL_CACHE_MASK],params,sizeof(*params));
	memcpy(&coll_cache[hash & COLL_CACHE_MASK],coll,sizeof(*coll));
}

int objCacheFitBlock(Vec3 start,Vec3 end,F32 *radius)
{
	if (*radius > 1.f || start[0] != end[0] || start[2] != end[2] || fabs(start[1] - end[1]) > 6.f)
		return 0;
	if (!*radius)
	{
		end[0] = start[0] = qtrunc(start[0]) & ~7;
		end[2] = start[2] = qtrunc(start[2]) & ~7;
		start[1] += 2;
		end[1] -= 2;
		*radius = 8;
	}
	else
	{
		end[0] = start[0] = qtrunc(start[0]);
		end[2] = start[2] = qtrunc(start[2]);
		start[1] += 1;
		end[1] -= 5;
		*radius = 2;
	}
	return 1;
}

ObjCache *findObj(const Vec3 start,U32 flags,F32 hashkey[3])
{
	U32			hash;

	if (flags & COLL_FINDINSIDE)
	{
		hashkey[0] = qtrunc(start[0]) & ~7;
		hashkey[1] = qtrunc(start[2]) & ~7;
	}
	else
	{
		hashkey[0] = qtrunc(start[0]);
		hashkey[1] = qtrunc(start[2]);
	}
	hashkey[2] = *((F32*)&flags);
	hash = hashCalc((void*)hashkey, sizeof(F32)*3, 0xa74d94e3);
	return &obj_cache[hash & OBJ_CACHE_MASK];
}

void objCacheSet(Vec3 start,Vec3 end,U32 flags,DefTracker **trackers,int count)
{
	ObjCache	*obj;
	F32			hashkey[3];

	if (!obj_cache_bits || count > ARRAY_SIZE(obj->trackers))
	{
		objcache_cant_fit++;
		return;
	}
	flags &= COLL_FINDINSIDE;
	obj = findObj(start,flags,hashkey);
	copyVec3(start,obj->start);
	obj->end = end[1];
	//copyVec3(end,obj->end);
	obj->count = count;
	obj->flags = flags;
	obj->valid_id = obj_grid.valid_id;
	memcpy(obj->trackers,trackers,count*sizeof(DefTracker*));
}

int find_inside_count,find_inside_cache_count;

int objCacheFind(const Vec3 start,const Vec3 end,F32 radius,U32 flags,DefTracker **trackers,int *count)
{
	ObjCache	*obj;
	F32			hashkey[3];

	if (grid_cache_disabled || !obj_cache_bits)
		return 0;
	if (radius > 1.f || start[0] != end[0] || start[2] != end[2] || fabs(start[1] - end[1]) > 6.f)
		return 0;
	flags &= COLL_FINDINSIDE;
	obj = findObj(start,flags,hashkey);
	if (flags && obj)
		find_inside_count++;
	if (hashkey[0] != obj->start[0] || hashkey[1] != obj->start[2]
		|| !(start[1] <= obj->start[1] && end[1] >= obj->end)
		|| flags != obj->flags
		|| 	obj->valid_id != obj_grid.valid_id)
		return 0;
	if (flags && obj)
		find_inside_cache_count++;
	*count = obj->count;
	memcpy(trackers,obj->trackers,obj->count*sizeof(DefTracker*));
	return 1;
}

void objCacheSetSize(int bits)
{
	SAFE_FREE(obj_cache);
	obj_cache_bits = bits;
	if (!bits)
		return;
	obj_cache = calloc(sizeof(*obj_cache),OBJ_CACHE_SIZE);
}

void gridCacheSetSize(int bits)
{
	objCacheSetSize(bits);
	SAFE_FREE(coll_cache);
	SAFE_FREE(param_cache);
	coll_cache_bits = bits;
	if (!bits)
		return;
	coll_cache = calloc(sizeof(*coll_cache),COLL_CACHE_SIZE);
	param_cache = calloc(sizeof(*param_cache),COLL_CACHE_SIZE);
}

static void gridCacheClearMemory()
{
	ZeroStructs(coll_cache, COLL_CACHE_SIZE);
	ZeroStructs(param_cache, COLL_CACHE_SIZE);
	ZeroStructs(obj_cache, OBJ_CACHE_SIZE);
}

void gridCacheReset()
{
	if (!coll_cache)
		return;
	gridCacheClearMemory();
	gridCacheInvalidate();
}

void gridCacheInvalidate()
{
	obj_grid.valid_id++;

	// Reset after wrap-around, just in case.
	
	if(!obj_grid.valid_id){
		gridCacheClearMemory();
	}
}
