#ifndef _CUBEMAP_H
#define _CUBEMAP_H

#include "stdtypes.h"
#include "viewport.h"
#include "tex.h"

#define NUM_CUBEMAP_FACES	6

typedef enum cubemapFlags
{
	kGenerateStatic		=1,
	kReloadStaticOnGen	=2,
	kSaveFacesOnGen		=4,
	kCubemapStaticGen_RenderAlpha		= 0x0080,	// when generating static cubemaps render alpha pass geometry (e.g. War Walls)
	kCubemapStaticGen_RenderParticles	= 0x0100,	// when generating static cubemaps render particles?
} cubemapFlags;

typedef struct CubemapFaceData
{
	ViewportInfo	viewport;
	BasicTexture	texture;
	char			name[32];
} CubemapFaceData;

void				cubemap_Update();
int					cubemap_GetTextureID(bool bForTerrain, Vec3 modelLoc, bool bForceDynamic);
F32					cubemap_CalculateAttenuation(bool bForTerrain, const Vec3 modelLoc, F32 modelRadius, bool bForceDynamic);
CubemapFaceData*	cubemapGetFace( int nFaceIndex );	// range: 0...(NUM_CUBEMAP_FACES-1)

void				cubemap_UseStaticCubemap();
void				cubemap_UseDynamicCubemap();

#endif // _CUBEMAP_H
