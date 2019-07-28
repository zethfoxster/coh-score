#ifndef _VIEW_CACHE_H
#define _VIEW_CACHE_H

#include "cmdgame.h"
#include "gfx.h"
#include "groupdraw.h"

#define VIEWCACHEDISABLED 1

// Forward declarations
typedef struct DefTracker DefTracker;
typedef struct GroupEnt GroupEnt;
typedef enum VisType VisType;

typedef struct ViewCacheEntry
{
	// parameters used for quick visibility test
	int			view_cache_parent_index;
	Vec3		mid;
	F32			radius;
	bool		has_model;
	bool		has_tracker;
	bool		visible;
	int			child_count;

	DrawParams	draw;
	F32			d;
	DefTracker	tracker;
	VisType		vis;
	Mat4		parent_mat;
	int			uid;
	GroupEnt	groupEnt;

} ViewCacheEntry;

typedef bool (*ViewCacheVisibilityCallback)(ViewCacheEntry *entry);

#if VIEWCACHEDISABLED
static bool INLINEDBG ViewCache_IsCaching() { return false; }
static bool INLINEDBG ViewCache_InUse() { return false; }
static bool INLINEDBG ViewCache_Update(int renderPass) { return true; }
static void INLINEDBG ViewCache_Use(ViewCacheVisibilityCallback callback, int renderPass)	{}
static int INLINEDBG ViewCache_Add(DrawParams *draw,F32 d,Vec3 mid,DefTracker *tracker,VisType vis,Mat4 parent_mat,int uid,GroupEnt *groupEnt)	{ assert(0); return -1; }
static void INLINEDBG ViewCache_UpdateChildCount(int parent_index) {}

#else
static INLINEDBG bool ViewCache_InUse()
{
	return gfx_state.viewCacheInUse;
}

static INLINEDBG bool ViewCache_IsCaching()
{
	return gfx_state.viewCacheCaching;
}

bool ViewCache_Update(int renderPass);
void ViewCache_Use(ViewCacheVisibilityCallback callback, int renderPass);

int ViewCache_Add(DrawParams *draw,F32 d,Vec3 mid,DefTracker *tracker,VisType vis,Mat4 parent_mat,int uid,GroupEnt *groupEnt);

void ViewCache_UpdateChildCount(int parent_index);
#endif

#endif
