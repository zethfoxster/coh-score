#ifndef _RT_CLOTH_H
#define _RT_CLOTH_H

#include "rt_model.h"
#include "texEnums.h"
#include "Cloth.h"

typedef struct TrickNode	TrickNode;
typedef struct ClothMesh	ClothMesh;
typedef struct Cloth		Cloth;
typedef struct ClothObject	ClothObject;

typedef struct ClothStrip
{
	S32 Type; // 0=strip, 1=fan
	S32 NumIndices;
	S16 *IndicesCCW;		// allocated
	S32 MinIndex;
	S32 MaxIndex;
} ClothStrip;

enum
{
	CLOTHMESH_TRISTRIP = 0,
	CLOTHMESH_TRIFAN = 1,
	CLOTHMESH_TRILIST = 2
};

typedef struct BasicTexture BasicTexture;
typedef struct TexBind TexBind;

typedef struct
{
	Vec3		PositionOffset;
	int			Flags;
	F32			alpha;
	Mat4		viewspace;
	TrickNode	trick;
	Vec3		ambient;
	Vec3		diffuse;
	Vec3		lightdir;
	BlendModeType blend_mode;
	U32			has_trick:1;
	ClothRenderData renderData;
	RdrTexList	texlist[2];
	F32			cubemap_attenuation;

	// only used by cape debug viewer. not thread safe
	Cloth		*cloth_debug;
	ClothObject	*clothobj_debug;
} RdrCloth;

void freeClothNodeDirect(ClothObject *clothobj);

#endif
