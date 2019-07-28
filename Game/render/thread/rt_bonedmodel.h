#ifndef _RT_BONEDMODEL_H
#define _RT_BONEDMODEL_H

#include "rt_model_cache.h"
#include "rt_model.h"

typedef struct Model Model;

typedef struct
{
	Vec4		ambient;
	Vec4		diffuse;
	Vec4		lightdir;
	VBO			*vbo;
	U8			rgba[2][4];
	U16			flags;
	U8			bone_count;
	U8			has_trick;
	S16			tex_index;
//	U8			blend_mode;
	F32			cubemap_attenuation;
	Model		*debug_model_backpointer;
	Mat4		bone_mats[1]; // MUST BE LAST
} SkinModel;

#define SKINMODEL_MAXBONE_MEM	(2 + (sizeof(TrickNode) + sizeof(Mat4) * MAX_BONES) / sizeof(SkinModel))	// enough memory for all the bones that could be in one skin

void DeformObject(Mat4 bones[], VBO *vbo, int vert_count, Vec3 *weightedverts, Vec3 *weightednorms, Vec3 *weightedtangents);
void modelDrawBonedNodeDirect( SkinModel *draw );

#endif
