/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BASEDRAW_H__
#define BASEDRAW_H__

#ifndef COLOR_H
#include "color.h"
#endif

typedef struct GroupDef GroupDef;
typedef struct DefTracker DefTracker;
typedef struct BaseRoom BaseRoom;
typedef struct Detail Detail;
typedef struct RoomDetail RoomDetail;

typedef struct TintedGroupDef
{
	GroupDef *pDef;
	Color tint[2];
} TintedGroupDef;


int rejectByCameraHeight(GroupDef *def,int *which);
void baseHackeryFreeCachedRgbs(void);
GroupDef *baseHackery(GroupDef *def,Mat4 mat,Vec3 mid,int *alpha,DefTracker *light_tracker,DefTracker **tracker_p);
void baseDrawFadeCheck(void);
void baseStartDraw(void);
void drawScaledBox(Mat4 worldMat,Vec3 size,GroupDef *box,Color tint[2]);

void drawDefSimple(GroupDef *def,Mat4 worldMat,Color tint[2]);

void drawScaledObject(TintedGroupDef *box, Mat4 worldMat,Vec3 size);
void drawDetailBox(RoomDetail *detail, Mat4 worldMat, Mat4 surface, TintedGroupDef *box, TintedGroupDef *box_volume, bool editor);
void drawBoxExtents(Vec3 pos,Vec3 min,Vec3 max,TintedGroupDef *box);


#endif /* #ifndef BASEDRAW_H__ */

/* End of File */

