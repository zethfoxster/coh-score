#ifndef _VISTRAY_H
#define _VISTRAY_H

#include "stdtypes.h"

typedef struct DefTracker DefTracker;
typedef struct DrawParams DrawParams;
typedef struct Grid Grid;

typedef struct VisTrayInfo
{
	int portal_count;
	int detail_count;
} VisTrayInfo;


void vistrayReset(void);
void vistrayResetUIDs(void);
void vistrayDraw(DrawParams *draw, Mat4 parent_mat);
void vistrayDrawDetails(DefTracker *tracker_parent, DrawParams *draw, Mat4 parent_mat, int vis);
void vistraySetUid(DefTracker *tray_tracker, int uid);
void vistraySetDetailUid(DefTracker *detail, int uid);

void vistrayOpenPortals(DefTracker *tracker);
void vistrayClosePortals(DefTracker *tracker);

// returns NULL if the tray is not open
// returns a pointer to a shared variable
VisTrayInfo * vistrayGetInfo(DefTracker *tracker);

// visible trays
void vistrayClearVisibleList(void);
void vistraySortVisibleList(void);
int vistrayIsVisible(DefTracker *tray_tracker);

// outside trays
void vistrayClearOutsideTrayList(void);
void vistrayAddOutsideTray(DefTracker *tray_tracker);
void vistrayDrawOutsideTrays(DrawParams *draw, Mat4 parent_mat);

// detail groups
void vistrayDetailTrays(void);
void vistrayFreeTrayDetails(DefTracker *tray_tracker);

void vistrayAddDetail(DefTracker *detail);
void vistrayFreeDetail(DefTracker *detail);

void vistrayDetailColorTrackerDel(DefTracker *tray_tracker);
void vistrayDetailLightsToGrid(DefTracker *tray_tracker, Grid *light_grid);

void vistrayNotifyDetailClosed(DefTracker *detail);

// Calculate whether p1 is reachable from p0 by making tray transitions through portals
bool vistrayIsSoundReachable(const Vec3 p0, const Vec3 p1);
#endif

