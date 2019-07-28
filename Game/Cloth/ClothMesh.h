#ifndef _CLOTHMESH_H
#define _CLOTHMESH_H

#include "stdtypes.h"
#include "mathutil.h"
#include "texEnums.h"

typedef struct ClothMesh ClothMesh;
typedef struct ClothStrip ClothStrip;

struct ClothMesh
{
	S32 Allocate;
	S32 NumStrips;
	ClothStrip *Strips;	// allocated
	S32 NumPoints;
	Vec3 *Points;		// pointer to pool
	Vec3 *Normals;		// pointer to pool
	Vec3 *BiNormals;	// pointer to pool
	Vec3 *Tangents;		// pointer to pool
	Vec2 *TexCoords;	// pointer to pool
	Vec3 Color;
	F32  Alpha;
	// Implementation specific data
	TextureOverride TextureData[2];
	U8   colors[4][4];
};

extern ClothMesh *ClothMeshCreate();
extern void ClothMeshDelete(ClothMesh *mesh);
extern void ClothMeshPrimitiveCreateBox(ClothMesh *mesh, F32 dx, F32 dy, F32 dz, int detail);
extern void ClothMeshPrimitiveCreatePlane(ClothMesh *mesh, F32 rad, int detail);
extern void ClothMeshPrimitiveCreateBalloon(ClothMesh *mesh, F32 rad, F32 hlen, int detail);
extern void ClothMeshPrimitiveCreateCylinder(ClothMesh *mesh, F32 rad, F32 hlen, int detail);
extern void ClothMeshPrimitiveCreateSphere(ClothMesh *mesh, F32 rad, int detail);

#include "rt_cloth.h"


#endif // _CLOTHMESH_H
