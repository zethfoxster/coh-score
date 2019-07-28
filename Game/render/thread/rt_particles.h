#ifndef _RT_PARTICLES_H
#define _RT_PARTICLES_H

#include "rt_tex.h"

typedef struct
{
	int		texid;
	Vec2	textranslate;
	Vec2	texscale;
} RdrParticleTex;

typedef struct
{
	Mat4	systemMatCamSpace;
	F32		alpha;
	int		particle_count;
	U32		blend_mode : 3;
	U32		has_systemMatCamSpace : 1;
	U32		whiteParticles : 1;
	int		*tris;
	RdrParticleTex	tex[2];
} RdrParticlePkg;

#ifdef RT_PRIVATE
void rdrCleanUpAfterRenderingParticleSystemsDirect();
void rdrPrepareToRenderParticleSystemsDirect(F32 **texarray);
void modelDrawParticleSysDirect(RdrParticlePkg *pkg );
#endif

#endif
