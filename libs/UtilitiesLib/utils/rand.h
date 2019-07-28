#pragma once

#include "mathutil.h"



// Here's the basic way this rand lib works. You can specify which type of PRNG to use, 
// which are listed and explained in the RandType enum
// You can also specify a seed, which will be used instead of the global seed. The global seed is per-RandType
// The Mersenne Twister is not easily reseeded, so do not use it with a seed unless you intend to generate a large number of
// numbers. The default choice is LCG with no seed, so the simplest functions call through to those settings.





typedef enum RandType
{
	RandType_LCG, // Linear Congruential Generator: Fastest, least well distributed
	RandType_Mersenne, // Mersenne Twister: Relatively slow, every 624th call must reinit table, but is otherwise fast. Extremely good distribution
	RandType_BLORN, // Big List of Random Numbers... generated from mersenne, but is just a table lookup so always fast. Worst periodicity, but otherwise very well distributed.
} RandType;

void testRandomNumberGenerators();
void generateUntemperedMersenneTable( void );

void generateBLORN();

void initRand();




// this function sets the original seed and forces a recomputation of the
// mersenne twister table. If you need a string of predictable numbers, 
// call this function once before using the randomMersenne functions.
void seedMersenneGenerator( U32 seed );

U32 randomStepSeed(U32* uiSeed, RandType eRandType);

// returns from 0 to 2^32-1
static INLINEDBG U32 randomU32Seeded(U32* uiSeed, RandType eRandType)
{
	return randomStepSeed(uiSeed, eRandType);
}

// returns from -2^31 to 2^31-1
static INLINEDBG int randomIntSeeded(U32* uiSeed, RandType eRandType)
{
	return (int)randomStepSeed(uiSeed, eRandType);
}

// returns between -1.0 and 1.0
static INLINEDBG F32 randomF32Seeded(U32* uiSeed, RandType eRandType)
{
	return -1.f + (randomStepSeed(uiSeed, eRandType) * 1.f / (F32)0x7fffffffUL);
}

// returns number in [0.0, 1.0)
static INLINEDBG F32 randomPositiveF32Seeded(U32* uiSeed, RandType eRandType)
{
	return (randomStepSeed(uiSeed, eRandType) * 0.999999f / (F32)0xffffffffUL);
}


// returns between 0.0 and 99.9999999
static INLINEDBG F32 randomPctSeeded(U32* uiSeed, RandType eRandType)
{
	return (randomStepSeed(uiSeed, eRandType) * 1.f / (F32)0x28f5c29UL);
}

// Returns a uniform distribution w.r.t. volume of a cube from -1,-1,-1 to 1,1,1
static INLINEDBG void randomVec3Seeded(U32* uiSeed, RandType eRandType, Vec3 vResult)
{
	vResult[0] = randomF32Seeded(uiSeed, eRandType);
	vResult[1] = randomF32Seeded(uiSeed, eRandType);
	vResult[2] = randomF32Seeded(uiSeed, eRandType);
}


// returns true or false equally likely
static INLINEDBG bool randomBoolSeeded(U32* uiSeed, RandType eRandType)
{
	return (randomStepSeed(uiSeed, eRandType) & 0x80) != 0; // I chose this bit to get away from the LSB's, while remaining 1 byte (for 1 byte bools)
}

// Returns a uniform distribution w.r.t. volume of a spherical slice defined by 2 angles, theta and phi, and a radius R
// theta ranges from 0 to 2pi and phi from 0 to pi. If you pass in 2pi and pi, it will be uniformly distributed in the volume
// of the sphere. Another example is passing in pi and pi, which will create a hemisphere.
// Imagine an arc defined by +/- theta swept about the y-axis by phi


// Does not slice up the sphere or sphere shell
static INLINEDBG void randomSphereSeeded(U32* uiSeed, RandType eRandType, F32 fRadius, Vec3 vResult)
{
	sphericalCoordsToVec3(
		vResult,
		randomPositiveF32Seeded(uiSeed, eRandType) * TWOPI,
		acosf(randomF32Seeded(uiSeed, eRandType)), // Note the arccos, this is to correct for bunching at the poles, since spherical coordinates are not uniform across the sphere
		randomPositiveF32Seeded(uiSeed, eRandType) * fRadius
		);
}
static INLINEDBG void randomSphereShellSeeded(U32* uiSeed, RandType eRandType, F32 fRadius, Vec3 vResult)
{
	sphericalCoordsToVec3(
		vResult,
		randomPositiveF32Seeded(uiSeed, eRandType) * TWOPI,
		acosf(randomF32Seeded(uiSeed, eRandType) * PI), // Note the arccos, this is to correct for bunching at the poles, since spherical coordinates are not uniform across the sphere
		fRadius
		);
}

// The same, but the radius is not random
static INLINEDBG void randomSphereShellSliceSeeded(U32* uiSeed, RandType eRandType, F32 fTheta, F32 fPhi, F32 fRadius, Vec3 vResult)
{
	sphericalCoordsToVec3(
		vResult,
		randomF32Seeded(uiSeed, eRandType) * fTheta,
		acosf(randomF32Seeded(uiSeed, eRandType) * fPhi * ONEOVERPI), // Note the arccos, this is to correct for bunching at the poles, since spherical coordinates are not uniform across the sphere
		fRadius
		);
}

static INLINEDBG void randomSphereSliceSeeded(U32* uiSeed, RandType eRandType, F32 fTheta, F32 fPhi, F32 fRadius, Vec3 vResult)
{
	sphericalCoordsToVec3(
		vResult,
		randomF32Seeded(uiSeed, eRandType) * fTheta,
		acosf(randomF32Seeded(uiSeed, eRandType) * fPhi), // Note the arccos, this is to correct for bunching at the poles, since spherical coordinates are not uniform across the sphere
		powf(randomPositiveF32Seeded(uiSeed, eRandType), 0.33333333f) * fRadius
		);
}



///
/// The functions below use the global seed and assume LCG, for simplicity
///
// returns from 0 to 2^32-1
static INLINEDBG U32 randomU32()
{
	return randomU32Seeded(NULL, RandType_LCG);
}

// returns from -2^31 to 2^31-1
static INLINEDBG int randomInt()
{
	return randomIntSeeded(NULL, RandType_LCG);
}

// returns between -1.0 and 1.0
static INLINEDBG F32 randomF32()
{
	return randomF32Seeded(NULL, RandType_LCG);
}

// returns between [0.0, 1.0)
static INLINEDBG F32 randomPositiveF32()
{
	return randomPositiveF32Seeded(NULL, RandType_LCG);
}


// returns between 0.0 and 99.999999
static INLINEDBG F32 randomPct()
{
	return randomPctSeeded(NULL, RandType_LCG);
}

// Returns a uniform distribution w.r.t. volume of a cube from -1,-1,-1 to 1,1,1
static INLINEDBG void randomVec3(Vec3 vResult)
{
	randomVec3Seeded(NULL, RandType_LCG, vResult);
}

// returns true or false equally likely
static INLINEDBG bool randomBool()
{
	return randomBoolSeeded(NULL, RandType_LCG);
}

// Returns uniformly distributed (w.r.t. volume) numbers in a sphere
static INLINEDBG void randomSphere(F32 fRadius, Vec3 vResult)
{
	randomSphereSeeded(NULL, RandType_LCG, fRadius, vResult);
}

// Return uniformly distributed (w.r.t. area) numbers on a spherical shell
static INLINEDBG void randomSphereShell(F32 fRadius, Vec3 vResult)
{
	randomSphereShellSeeded(NULL, RandType_LCG, fRadius, vResult);
}

// Returns uniformly distributed (w.r.t. volume) numbers in a sphere slice
static INLINEDBG void randomSphereSlice(F32 fTheta, F32 fPhi, F32 fRadius, Vec3 vResult)
{
	randomSphereSliceSeeded(NULL, RandType_LCG, fTheta, fPhi, fRadius, vResult);
}

// Return uniformly distributed (w.r.t. area) numbers on a spherical shell slice
static INLINEDBG void randomSphereShellSlice(F32 fTheta, F32 fPhi, F32 fRadius, Vec3 vResult)
{
	randomSphereShellSliceSeeded(NULL, RandType_LCG, fTheta, fPhi, fRadius, vResult);
}


///
/// The functions below use the global seed and assume Mersenna
///
// returns from 0 to 2^32-1
static INLINEDBG U32 randomMersenneU32()
{
	return randomU32Seeded(NULL, RandType_Mersenne);
}

// returns from -2^31 to 2^31-1
static INLINEDBG int randomMersenneInt()
{
	return randomIntSeeded(NULL, RandType_Mersenne);
}

// returns between -1.0 and 1.0
static INLINEDBG F32 randomMersenneF32()
{
	return randomF32Seeded(NULL, RandType_Mersenne);
}

// returns between [0.0, 1.0)
static INLINEDBG F32 randomMersennePositiveF32()
{
	return randomPositiveF32Seeded(NULL, RandType_Mersenne);
}


// returns between 0.0 and 99.999999
static INLINEDBG F32 randomMersennePct()
{
	return randomPctSeeded(NULL, RandType_Mersenne);
}

// Returns a uniform distribution w.r.t. volume of a cube from -1,-1,-1 to 1,1,1
static INLINEDBG void randomMersenneVec3(Vec3 vResult)
{
	randomVec3Seeded(NULL, RandType_Mersenne, vResult);
}

// returns true or false equally likely
static INLINEDBG bool randomMersenneBool()
{
	return randomBoolSeeded(NULL, RandType_Mersenne);
}

// Returns uniformly distributed (w.r.t. volume) numbers in a sphere
static INLINEDBG void randomMersenneSphere(F32 fRadius, Vec3 vResult)
{
	randomSphereSeeded(NULL, RandType_Mersenne, fRadius, vResult);
}

// Return uniformly distributed (w.r.t. area) numbers on a spherical shell
static INLINEDBG void randomMersenneSphereShell(F32 fRadius, Vec3 vResult)
{
	randomSphereShellSeeded(NULL, RandType_Mersenne, fRadius, vResult);
}

// Returns uniformly distributed (w.r.t. volume) numbers in a sphere slice
static INLINEDBG void randomMersenneSphereSlice(F32 fTheta, F32 fPhi, F32 fRadius, Vec3 vResult)
{
	randomSphereSliceSeeded(NULL, RandType_Mersenne, fTheta, fPhi, fRadius, vResult);
}

// Return uniformly distributed (w.r.t. area) numbers on a spherical shell slice
static INLINEDBG void randomMersenneSphereShellSlice(F32 fTheta, F32 fPhi, F32 fRadius, Vec3 vResult)
{
	randomSphereShellSliceSeeded(NULL, RandType_Mersenne, fTheta, fPhi, fRadius, vResult);
}
















// Implementation of improved perlin noise
// noise has range -1 to 1

// get noise at a single frequency band
F32 noiseGetBand2D( F32 x, F32 y, F32 wavelength );
F32 noiseGetBand3D( F32 x, F32 y, F32 z, F32 wavelength );

// get a combination of wavelength bands from min_wavelength to max_wavelength
// amplitude of each frequency is 1 / f^falloff_rate, so falloff_rate = 0 is white noise
F32 noiseGetFullSpectrum2D( F32 x, F32 y, F32 min_wavelength, F32 max_wavelength, F32 falloff_rate );
F32 noiseGetFullSpectrum3D( F32 x, F32 y, F32 z, F32 min_wavelength, F32 max_wavelength, F32 falloff_rate );

