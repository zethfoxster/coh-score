#define RT_PRIVATE
#include "wininclude.h"
#include "camera.h" //mm temp
#include "particle.h"
#include "renderparticles.h"
#include "gfxwindow.h"
#include "cmdgame.h"
#include "timing.h"
#include "assert.h"
#include "utils.h"
#include "font.h"
#include "model_cache.h"
#include "rt_queue.h"
#include "tex.h"

int particleCount[160];

extern CameraInfo cam_info;
extern ParticleEngine particle_engine;
extern Vec2 fog_dist;

extern int particle_rgba_type;
extern int particle_vbos;

void rdrPrepareToRenderParticleSystems()
{
	particle_engine.tinyVertArrayIdCurrentIdx = 0;
	particle_engine.smallVertArrayIdCurrentIdx = 0;
	particle_engine.largeVertArrayIdCurrentIdx = 0;
	rdrQueue(DRAWCMD_PARTICLESETUP,&particle_engine.texarray,sizeof(F32 *));
}

void rdrCleanUpAfterRenderingParticleSystems()
{
	rdrQueueCmd(DRAWCMD_PARTICLEFINISH);
}


int modelDrawParticleSys(ParticleSystem * system, F32 alpha, int VBOBuffer, Mat4 systemMatCamSpace )
{
	int					i,vert_size,rgba_size;
	ParticleSystemInfo	*info = system->sysInfo;
	RdrParticlePkg		*pkg;
	int					tex_ids[2];

	PERFINFO_AUTO_START("partBuildParticleArray",1);
		if(!(game_state.fxdebug & FX_DISABLE_PART_ARRAY_WRITE))
			partBuildParticleArray(system, alpha, system->verts, system->rgbas, TIMESTEP);
	PERFINFO_AUTO_STOP();
	if(game_state.fxdebug & FX_DISABLE_PART_RENDERING )
		return system->particle_count;

	for(i=0;i<2;i++)
		tex_ids[i] = texDemandLoad(info->parttex[i].bind);
	vert_size = system->particle_count * 4 * sizeof(Vec3);
	rgba_size = system->particle_count * 4 * sizeof(system->rgbas[0])*4;
	pkg = rdrQueueAlloc(DRAWCMD_PARTICLES,sizeof(RdrParticlePkg) + vert_size + rgba_size);
	if (systemMatCamSpace)
	{
		copyMat4(systemMatCamSpace,pkg->systemMatCamSpace);
		pkg->has_systemMatCamSpace = 1;
	}
	else
		pkg->has_systemMatCamSpace = 0;
	pkg->alpha			= alpha;
	pkg->particle_count = system->particle_count;
	pkg->blend_mode = info->blend_mode;
	pkg->whiteParticles = game_state.whiteParticles;
	pkg->tris			= system->tris;
	for(i=0;i<2;i++)
	{
		pkg->tex[i].texid = tex_ids[i];
		copyVec2(system->textranslate[i],pkg->tex[i].textranslate);
		copyVec2(info->parttex[i].texscale,pkg->tex[i].texscale);
		
	}
	memcpy(pkg + 1,system->verts,vert_size);
	memcpy(((U8*)(pkg + 1))+vert_size,system->rgbas,rgba_size);

	rdrQueueSend();
	return system->particle_count;
}

#ifdef NOVODEX_FLUIDS
void drawFluid(FxFluidEmitter* emitter)
{
	int					i,vert_size,rgba_size;
	RdrParticlePkg		*pkg;
	int					tex_ids[2];
	U8*					rgbas;
	U32					numParticles = emitter->currentNumParticles;
	FxFluid*			fluid = emitter->fluid;
	U32					numVerts = numParticles * 4;
	F32					alpha = 1.0f;
	Vec3*				verts;
	U32					uiParticle;
	F32					fHalfScale;
	F32*				vertarray;
	U8*					rgbaarray;
	U8*					rgb;

	rgb = fluid->colorPathDefault.path;


	// Allocate rgbas
	vert_size = numVerts * sizeof(Vec3);
	rgba_size = numVerts * sizeof(rgbas[0])*4;

	verts = malloc( vert_size );
	vertarray = (F32*)verts;
	rgbas = malloc( rgba_size );
	rgbaarray = rgbas;
	// Set colors to white for now
	memset(rgbas, 0xFF, rgba_size );

	fHalfScale = fluid->startSize * 0.5f;

	// Calculate verts from particle positions
	for (uiParticle=0; uiParticle<numParticles; ++uiParticle)
	{

		Vec3 part_pos_view;
		//copyVec3(emitter->particlePositions[uiParticle], part_pos_view);
		mulVecMat4(emitter->particlePositions[uiParticle], cam_info.viewmat, part_pos_view);
		
		*vertarray++ = part_pos_view[0] - fHalfScale;
		*vertarray++ = part_pos_view[1] + fHalfScale + fHalfScale; 
		*vertarray++ = part_pos_view[2];

		//upper right
		*vertarray++ = part_pos_view[0] + fHalfScale ; 
		*vertarray++ = part_pos_view[1] + fHalfScale + fHalfScale ;
		*vertarray++ = part_pos_view[2];

		//lower left
		*vertarray++ = part_pos_view[0] - fHalfScale;
		*vertarray++ = part_pos_view[1];
		*vertarray++ = part_pos_view[2];

		//lower right
		*vertarray++ = part_pos_view[0] + fHalfScale;
		*vertarray++ = part_pos_view[1];
		*vertarray++ = part_pos_view[2];
		
		if( fluid->colorPathDefault.length > 1)
		{
			int idx;
			//if(sysInfo->colorchangetype == PARTICLE_COLORSTOP)
				idx = MIN((round(emitter->particleDensities[uiParticle])), fluid->colorPathDefault.length - 1);
			//else //sysInfo->colorchangetype == PARTICLE_COLORLOOP
			//	idx = (round(particle->age-timestep)) % system->effectiveColorPath->length;
			rgb = &(fluid->colorPathDefault.path[idx * 3]);

		}
		*rgbaarray++ = rgb[0];
		*rgbaarray++ = rgb[1];
		*rgbaarray++ = rgb[2];
		*rgbaarray++ = fluid->alpha; 

		*rgbaarray++ = rgb[0];
		*rgbaarray++ = rgb[1];
		*rgbaarray++ = rgb[2];
		*rgbaarray++ = fluid->alpha;

		*rgbaarray++ = rgb[0];
		*rgbaarray++ = rgb[1];
		*rgbaarray++ = rgb[2];
		*rgbaarray++ = fluid->alpha;

		*rgbaarray++ = rgb[0];
		*rgbaarray++ = rgb[1];
		*rgbaarray++ = rgb[2];
		*rgbaarray++ = fluid->alpha;


	}

	// Load textures
	for(i=0;i<2;i++)
		tex_ids[i] = texDemandLoad(fluid->parttex[i].bind);

	pkg = rdrQueueAlloc(DRAWCMD_PARTICLES,sizeof(RdrParticlePkg) + vert_size + rgba_size);
	/*
	if (systemMatCamSpace)
	{
		copyMat4(systemMatCamSpace,pkg->systemMatCamSpace);
		pkg->has_systemMatCamSpace = 1;
	}
	else
	*/
		pkg->has_systemMatCamSpace = 0;
	pkg->alpha			= alpha;
	pkg->particle_count = numParticles;
	/*
	if (info->blend_mode == PARTICLE_ADDITIVE)
		pkg->additive = 1;
	else
		pkg->additive = 0;
		*/
	pkg->blend_mode = PARTICLE_ADDITIVE;

	pkg->whiteParticles = game_state.whiteParticles;
	pkg->tris			= particle_engine.tris;
	for(i=0;i<2;i++)
	{
		pkg->tex[i].texid = tex_ids[i];
		copyVec2(zerovec3,pkg->tex[i].textranslate);
		copyVec2(onevec3,pkg->tex[i].texscale);

	}
	memcpy(pkg + 1,verts,vert_size);
	memcpy(((U8*)(pkg + 1))+vert_size,rgbas,rgba_size);

	rdrQueueSend();
	free(verts);
	free(rgbas);

}
#endif


