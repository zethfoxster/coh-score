#ifndef RENDERWATER_H
#define RENDERWATER_H

#include "stdtypes.h"

typedef enum EPlanarReflectModes
{
	kReflectNearby,
	kReflectEntitiesOnly,
	kReflectAll,
} EPlanarReflectModes;

typedef enum EPlanarReflectProjClip
{
	kReflectProjClip_None,
	kReflectProjClip_EntitiesOnly,
	kReflectProjClip_All,
} EPlanarReflectProjClip;

void rdrWaterGrabBuffer(bool doReflection);

typedef struct PBuffer PBuffer;
// Debug
void rdrFrameGrab(void);
void rdrFrameGrabShow(F32 alpha);
void rdrShowPBuffer(F32 alpha, PBuffer * pbuf, F32 scale);

int waterTextureSize(int x);

int waterHandle(void);

void requestReflection(void);
#endif
