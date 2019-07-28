#ifndef _LIGHT_H
#define _LIGHT_H

#include "mathutil.h"

#define COLOR_SCALEUB(x) (x >> 2)

typedef struct EntLight EntLight;
typedef struct GroupDef GroupDef;
typedef struct DefTracker DefTracker;
typedef struct Model Model;

void lightTracker(DefTracker *tracker);
void lightAll(void);
void lightModel(Model *model,Mat4 mat,U8 *rgb,DefTracker *light_trkr, F32 brightScale);
DefTracker *findLightsRecur(DefTracker *tracker,Vec3 pos,F32 *rad);
DefTracker *findLights(Vec3 pos);
void lightEnt(EntLight *light,Vec3 pos, F32 min_ambient, F32 max_ambient);
void lightGiveLightTrackerToMyDoor(DefTracker * tracker, DefTracker * lighttracker);
void drawLightDirLines(Mat4 cam);
void lightClearLightLines(void);
void lightReapplyColorSwaps();
void getDefColorVec3(GroupDef *def,int which,Vec3 rgb);
void getDefColorU8(GroupDef *def,int which,U8 color[3]);
void lightCheckReapplyColorSwaps();

#endif
