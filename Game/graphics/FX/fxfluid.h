#ifndef _FXFLUID_H
#define _FXFLUID_H

#include "NwWrapper.h"

#if NOVODEX
#include "stdtypes.h"

#include "particle.h"


typedef struct FxFluid 
{
	struct		FxFluid * next;//development only for listing non binary loaded fluids
	struct		FxFluid * prev;

	char*		name;
	int			fileAge;			//development only

	F32 restitution;				// Restitution with shapes
	F32 adhesion;					// Adhesion with shapes
	int maxParticles;				// How many particles this can have in existance at once
	F32 restParticlesPerMeter;		// How many particles per linear meter, max, at rest state
	F32 restDensity;				// How much mass per cubic meter the fluid has, assuming rest state.
	F32 stiffness;					// Compressibility
	F32 viscosity;					// Noun. The quality or state of being viscous. See viscous.
	F32 damping;					// Velocity damping, over time.

	F32 dimensionX;					// The x-axis radius of the emitter
	F32 dimensionY;					// The y-axis radius of the emitter
	F32 rate;						// emission rate, in particles per second
	F32 randomAngle;				// Angle deviation from particle emission
	F32 fluidVelocityMagnitude;		// how fast the fluid travels upon emission
	F32 particleLifetime;			// how long a particle lives for

	PartTex parttex[2];				// Particle texture info
	F32 startSize;					// The starting scale of the fluid particle, for rendering
	int		blend_mode;			//blend mode:  PARTICLE_NORMAL or PARTICLE_ADDITIVE
	int		alpha;

	ColorNavPoint colornavpoint[MAX_COLOR_NAV_POINTS];
	ColorPath	colorPathDefault;


	U8 initialized;
} FxFluid;

typedef struct FxFluidEmitter
{
	FxGeo* geoOwner;
	FxFluid* fluid;
	void* nxFluid;
	Vec3* particlePositions;
	Vec3* particleVelocities;
	F32* particleAges;
	F32* particleDensities;
	U32  currentNumParticles;
} FxFluidEmitter;


FxFluid * fxGetFxFluid(char fluid_name[]);
void fxPreloadFluidInfo();


#endif

#endif
