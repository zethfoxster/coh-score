#ifndef _SPLAT_H
#define _SPLAT_H

/* Copyright (C) Eric Lengyel, 2001. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Eric Lengyel, 2001"
 */

#include "mathutil.h"
#include "stdtypes.h"
#include "gridcoll.h"
#include "animtrackanimate.h"

typedef struct GfxNode GfxNode;
typedef struct BasicTexture BasicTexture;

typedef struct Triangle
{
	U32	index[3];
} Triangle;

#define MAX_SHADOW_TRIS 5000
#define splatEpsilon 0.25F

typedef enum
{
	SPLAT_ADDITIVE		= ( 1 << 0 ),
	SPLAT_SHADOWFADE	= ( 1 << 1 ),
	SPLAT_ADDBASE		= ( 1 << 2 ),
	SPLAT_ROUNDTEXTURE	= ( 1 << 3 ),
	SPLAT_SUBTRACTIVE	= ( 1 << 4 ),
	SPLAT_CAMERA		= ( 1 << 5 ),
} eSplatFlags;


typedef enum
{
	SPLAT_FALLOFF_NONE		= ( 0 ),
	SPLAT_FALLOFF_UP		= ( 1 ),
	SPLAT_FALLOFF_DOWN		= ( 2 ),
	SPLAT_FALLOFF_BOTH		= ( 3 ),
	SPLAT_FALLOFF_SHADOW	= ( 4 ),

} eSplatFalloffType;

typedef struct Splat
{	
	//The splat parameters
	Vec3		center;
	Vec3		normal;
	Vec3		tangent;
	Vec3		projectionEnd;
	Vec3		binormal;
	F32			height;
	F32			width;
	F32			depth;
	F32			realWidth;
	F32			realHeight;
	U8			currMaxAlpha; //last time this was calculated, this was the max alpha
	union
	{
		BasicTexture*texture1;
		int			texid1;
	};
	union
	{
		BasicTexture*texture2;
		int			texid2;
	};
	int			drawMe;		// set to gloal_frame_count when it should be rendered

	eSplatFlags	flags;
	StAnim		*stAnim;
	eSplatFalloffType  falloffType;
	F32			normalFade;
	F32			fadeCenter;

	//The resulting splat
	int			currMaxVerts;  //how much memory(/Vec3 size) is currently alloced in verts (and sts, sts2,and colors)
	int			currMaxTris;	//how much memory(/Triangle size) is currently alloced for tris
	int			vertCnt;
	int			triCnt;
	Vec3		*verts;
	Vec2		*sts;
	Vec2		*sts2;
	Triangle	*tris;
	U8			*colors;
	Vec3		previousStart;
	struct Splat * invertedSplat;

} Splat;


#define MIN_SHADOW_WORTH_DRAWING 0.05

#define SPLAT_LOW_DENSITY_REJECTION		0.02 //all ground except chunky rocks.
#define SPLAT_MEDIUM_DENSITY_REJECTION	0.2 //basically everything you might walk on, chunky rocks are around 0.1
#define SPLAT_HIGH_DENSITY_REJECTION	0.8  //pretty much everything; cars and benches are around 0.7	
#define SHADOW_BASE_DRAW_DIST 120.0 //This plus below is total fade out dist for shadow
#define SHADOW_FADE_OUT_DIST 40.0  
#define SEQ_DEFAULT_SHADOW_SIZE 3.6 
#define SHADOW_SPLAT_OFFSET 2.0

typedef struct SplatParams
{
	Vec3	projectionStart;	
	Vec3	projectionDirection;
	Vec3	shadowSize;
	F32		max_density;
	U8		maxAlpha;
	Mat4	mat;
	U8		rgb[3];
	eSplatFlags	flags;
	StAnim  * stAnim;
	eSplatFalloffType  falloffType;
	F32		normalFade;
	F32		fadeCenter;
	GfxNode * node;

} SplatParams;

Splat* createSplat();
void destroySplat(Splat* splat);
void updateASplat( SplatParams * splatParams );
GfxNode * initSplatNode( int * nodeId, const char * texture1, const char * texture2, int doInvertedSplat );
void updateSplatTextureAndColors(SplatParams* splatParams, F32 width, F32 height);

#endif
