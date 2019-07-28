#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "stdtypes.h"
#include "mathutil.h"

#include "Wind.h"

//////////////////////////////////////////////////////////////////////////////
// WIND
// A sample "World" or "Ambient" wind generator.
// All wind values are in the range [0,1].
// This code randomly increases/decreases the wind speed within a tunable
//   range each frame. The virtual Min and Max values can be set to <0 or >1.
//   The actual magnitude will be clamed to [0,1]. This will produce periods
//   where the wind sits at its min or max value creating pauses or gusts.
// The Direction of the wind also varies from frame to frame, which is
//   accomplished by adding a vector within a tunable range perpendicular
//   to the default direction.
//
//////////////////////////////////////////////////////////////////////////////

// These represent the Max wind magnitude and the "Average" wind direction,
//   from which the "Current" Wind force is calculated
Vec3 WindDir = { .707f, 0.f, .707f };	// Normalized
F32 WindMagnitude = 0.2f;				// [0, 1]

// These store local values that are used to generate the current result
//F32 WindScale = 0.f;					// Calculated each frame
//F32 WindDirScale = 0.f;					// Calculated each frame

// These are tunable paramaters affecting the Wind magnitude
F32 WindDeltaScale = 0.25f;				// Max amt of change per frame of Magnitude
F32 WindScaleMin = -.5f;				// Values < 0 will clamp to 0 (creating pauses)
F32 WindScaleMax = 1.5f;				// Values > 1 will clamp to 1 (creating gusts)

// These are tunable paramaters affecting the Wind direction
F32 WindDirDeltaScale = 0.1f;			// Max amount of change per frame of tangent
F32 WindDirDeltaMax = 1.0f;				// = tangent(max angle from WindDir) 1.0 = +/- 45 deg

void ClothWindSetDir(Vec3 dir, F32 magnitude)
{
	if (dir)
	{
		copyVec3(dir, WindDir);
		normalVec3(WindDir);
	}
	WindMagnitude = magnitude;
}

F32 ClothWindGetRandom(Vec3 wind, F32 *windScale, F32 *windDirScale)
{
	F32 frand,scale,mag;
	Vec3 dir,adddir;
	frand = rand() * (1.0f/(F32)RAND_MAX); // [0, 1]
	frand = -1.0f + (frand*2.0f); // [-1, 1]
	*windScale += WindDeltaScale * frand;
	*windScale = MINMAX(*windScale, WindScaleMin, WindScaleMax);

	frand = rand() * (1.0f/(F32)RAND_MAX); // [0, 1]
	frand = -1.0f + (frand*2.0f); // [-1, 1]
	*windDirScale += WindDirDeltaScale * frand;
	*windDirScale = MINMAX(*windDirScale, -WindDirDeltaMax, WindDirDeltaMax);
	
	scale = MINMAX(*windScale, 0.f, 1.f);
	mag = WindMagnitude * scale;

	// add random perpendicular component
	// dir = WindDir + (WindDir x (0,1,0) * *windDirScale)
	setVec3(adddir, -WindDir[2]* *windDirScale, 0.f, WindDir[0]* *windDirScale);
	scaleAddVec3(adddir, *windDirScale, WindDir, dir);
	normalVec3(dir);
	
	scaleVec3(dir, mag, wind);
	return mag;
}

//////////////////////////////////////////////////////////////////////////////

