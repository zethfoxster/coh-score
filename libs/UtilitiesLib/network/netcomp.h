#ifndef _NETCOMP_H
#define _NETCOMP_H

#include "stdtypes.h"
#include "network\netio.h"

typedef struct
{
	Vec3	pyr;
	Vec3	scale;
	Vec3	pos;
	char	has_pyr,has_scale,has_pos;
} PackMat;


#define POS_SCALE (256.f/4.f)
#define POS_BITS 24

U32 packQuatElem(F32 qelem,int numbits);
F32 unpackQuatElem(U32 val,int numbits);
U32 packQuatElemQuarterPi(F32 qelem,int numbits);
F32 unpackQuatElemQuarterPi(U32 val,int numbits);
U32 packEuler(F32 ang,int numbits);
F32 unpackEuler(U32 val,int numbits);
U32 packPos(F32 pos);
F32 unpackPos(U32 ut);
F32 quantizePos(F32 pos);
void unitZeroMat(Mat4 mat,int *unit,int *zero);
int packMat(Mat4 mat_in,PackMat *pm);
void unpackMat(PackMat *pm,Mat4 mat);

#endif
