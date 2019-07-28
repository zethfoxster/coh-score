#ifndef DEBUGLOCATION_H
#define DEBUGLOCATION_H

#include "stdtypes.h"

typedef struct Packet Packet;

typedef struct DebugLocation
{
	Vec3	position;
	U32		uniqueId;
	Vec3	hsv;
	Vec3	rgb;
	int		alpha;
	int		vModifier;
} DebugLocation;

U32 DebugLocationCreate(Vec3 pos, U32 col);
void DebugLocationReceive(Packet* pak);
int DebugLocationDestroy(U32 id);
void DebugLocationDisplayAll();
void DebugLocationShutdown();
int DebugLocationCount();

#endif
