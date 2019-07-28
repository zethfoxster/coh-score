#ifndef _FXUTIL_H
#define _FXUTIL_H

#include "Color.h"
#include "seq.h"
#include "gfxtree.h"
#include "mathutil.h"

typedef struct ColorPair ColorPair;
typedef struct ParticleSystemInfo ParticleSystemInfo;

#define MAX_COLOR_NAV_POINTS	5

typedef enum 
{
	FX_VISIBLE		= (1 << 0),
	FX_NOTVISIBLE	= (1 << 1),
} eFxVisibility;

#define FX_UPDATE_AND_DRAW_ME		0
#define FX_UPDATE_BUT_DONT_DRAW_ME	1
#define FX_DONT_UPDATE_OR_DRAW_ME	2
#define FX_POWER_RANGE 10 //should always be same as particle power range I think

typedef struct ColorNavPoint
{
	U8	rgb[3];
	int	time;		//time to arrive at next ColorNavPoint
	F32 primaryTint;
	F32 secondaryTint;
} ColorNavPoint;

typedef struct ColorPath
{
	U8			* path;		//Calculated based on color nav points
	int			length;
} ColorPath;

#define FX_MAX_PATH 300

void fxFindWSMWithOffset(Mat4 result, GfxNode * node, Vec3 offset);
F32 fxLookAt(const Vec3 locVec, const Vec3 targetVec, Mat4 result);
void fxLookAt2(Vec3 locVec, Vec3 targetVec, Mat4 result );
F32 fxFaceTheCamera(Vec3 location, Mat4 result);
void partGetRandomPointOnLine(Vec3 start, Vec3 end, Vec3 result);
void initQuickTrig2();
F32 quickSin(int theta);
F32 quickCos(int theta);
F32 findAngle(Vec3 start, Vec3 end);
Color sampleColorPath(ColorNavPoint const* colorNavPoint, 
					  ColorPair const* customColors, F32 time);
void partBuildColorPath(ColorPath * colorPath, ColorNavPoint const navpoint[], 
						ColorPair const* customColors, bool invertTint);
void partCreateHueShiftedPath(ParticleSystemInfo *sysInfo, Vec3 hsvShift, 
							  ColorPath* colorPath);
void getRandomPointOnCylinder(Vec3 start_pos, F32 radius, F32 height, Vec3 final_pos );
void getRandomPointOnSphere(Vec3 initial_position, F32 radius, Vec3 final_position );
int fxHasAValue( char * string );
void * sort_linked_list(void *p, unsigned index,
  int (*compare)(void *, void *, void *), void *pointer, unsigned long *pcount);
U32 magnitudeToFxPower( F32 magnitude, F32 min_value, F32 max_value );
void partRadialJitter(Vec3 subject, F32 angle); // only works for vertical vectors at the moment

static INLINEDBG void partVectorJitter(const Vec3 subject, const Vec3 parameter, Vec3 output ) 
{
	output[0] = subject[0] + parameter[0] * qfrand();
	output[1] = subject[1] + parameter[1] * qfrand();
	output[2] = subject[2] + parameter[2] * qfrand();
}


static INLINEDBG void partRunMagnet(Vec3 velocity, const Vec3 position, const Vec3 magnet, F32 magnetism)
{
	if(1) 
	{
		if(position[0] < magnet[0]) 
			velocity[0] += magnetism;
		else 
			velocity[0] -= magnetism;

		if(position[1] < magnet[1]) 
			velocity[1] += magnetism; 
		else 
			velocity[1] -= magnetism;

		if(position[2] < magnet[2]) 
			velocity[2] += magnetism; 
		else 
			velocity[2] -= magnetism; 
	}  
	else
	{ //This is much worse
		Vec3 r;
		int temp[3];
		subVec3(magnet, position, r);
		temp[0] = ( *(int*)&magnetism ^ ( (*(int*)&r[0]) & 0x80000000 ) );
		temp[1] = ( *(int*)&magnetism ^ ( (*(int*)&r[1]) & 0x80000000 ) );
		temp[2] = ( *(int*)&magnetism ^ ( (*(int*)&r[2]) & 0x80000000 ) );
		velocity[0] += ( *(F32*)&temp[0] );
		velocity[1] += ( *(F32*)&temp[1] );
		velocity[2] += ( *(F32*)&temp[2] );
	}
}//*/

static INLINEDBG void partRunMagnet2(Vec3 velocity, const Vec3 position, const Vec3 magnet, F32 magnetism)
{
	velocity[0] += magnetism * (position[0] - magnet[0]); //mask all but the sign
	velocity[1] += magnetism * (position[1] - magnet[1]);
	velocity[2] += magnetism * (position[2] - magnet[2]);
}

#endif
