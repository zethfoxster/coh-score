#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "stdtypes.h"
#include "error.h"
#include "mathutil.h"
#include "group.h"
#include "grouptrack.h"
#include "gridcoll.h"
#include "grouputil.h"
#include "gridfind.h"
#include "assert.h"
#include "utils.h"
#include "anim.h"

static Grid			*tray_grid = 0;

static void trayGridAdd(Mat4 pmat,Vec3 pmin,Vec3 pmax,DefTracker *tracker)
{
	Vec3		min,max;
	GroupDef	*def = tracker->def;

	if (!tray_grid)
		tray_grid = createGrid(1024);

	groupGetBoundsTransformed(pmin, pmax, pmat, min, max);
	gridAdd(tray_grid,tracker,min,max,0,&tracker->traygrid_idxs);
}

int trayGridCheck(Vec3 center, F32 radius)
{
	void *ptr = 0;

	if (!tray_grid)
		return 0;

	return gridFindObjectsInSphere(tray_grid, &ptr, 1, center, radius);
}

void trayGridRefActivate(DefTracker *tracker)
{
	GroupDef *def = tracker->def;

	if (!def)
		return;

	trackerSetMid(tracker);

	if (def->is_tray)
	{
		// the outdoor mission tray is special: it is for mission purposes, not for drawing
		if (!def->model || (tracker->parent && tracker->parent->def && tracker->parent->def->has_volume) || !strstri(def->model->name, "_outdoor_mission_tray"))
			trayGridAdd(tracker->mat, def->min, def->max, tracker);
	}
	else if (def->child_tray)
	{
		// recursive step
		int i;
		trackerOpen(tracker);
		for (i = 0; i < tracker->count; i++)
			trayGridRefActivate(&(tracker->entries[i]));
	}
}

void trayGridFreeAll(void)
{
	if (!tray_grid)
		return;
	gridFreeAll(tray_grid);
	tray_grid = 0;
}

void trayGridDel(DefTracker *tracker)
{
	GroupDef *def = tracker->def;

	if (!def)
		return;

	if (def->is_tray)
	{
		gridClearAndFreeIdxList(tray_grid, &tracker->traygrid_idxs);
	}
	else if (def->child_tray)
	{
		// recursive step
		int i;
		trackerOpen(tracker);
		for (i = 0; i < tracker->count; i++)
			trayGridDel(&(tracker->entries[i]));
	}
}






