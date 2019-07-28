#ifndef _UVUNWRAP_H_
#define _UVUNWRAP_H_

#include "stdtypes.h"
#include "mathutil.h"


typedef struct _Prim Prim;
Prim *primCreate(Vec3 p0, Vec3 p1, Vec3 p2, Vec2 st0, Vec2 st1, Vec2 st2);
void primGetTexCoords(Prim *p, Vec2 texcoord0, Vec2 texcoord1, Vec2 texcoord2);
void primDestroy(Prim *p);

float uvunwrap(Prim ***primitives);


#endif


