#ifndef _FXBHVR_H
#define _FXBHVR_H

#include "stdtypes.h"
#include "fxutil.h"
#include "splat.h"
#include "animtrackanimate.h"
#include "NwWrapper.h"

//Plots the behavior of an fxgeo.  Read in from a .bhvr file 
typedef struct FxBhvr
{
	struct		FxBhvr * next; //development only for listing non binary loaded bhvrs
	struct		FxBhvr * prev;

	char*		name;
	U8			initialized;	// have we been initialized by fxInitFxBhvr yet?
	int			fileAge;		//development only

	//Read in from file
	F32			gravity;
	F32			drag;

	Vec3		startjitter;
	Vec3		initialvelocity;
	Vec3		initialvelocityjitter;
	F32			initialvelocityangle;
  
	U8			physicsEnabled;
	F32 		physRadius;
	F32			physGravity;
	F32			physRestitution;
	F32			physStaticFriction;
	F32			physDynamicFriction;
	F32			physKillBelowSpeed;
	F32			physDensity;
	F32			physForceRadius;
	F32			physForcePower;
	F32			physForcePowerJitter;
	F32			physForceCentripetal;
	F32			physForceSpeedScaleMax;
	F32			physJointDrag;
	Vec3		physScale;
#if NOVODEX
	eNxPhysDebrisType	physDebris;
	eNxJointDOF physJointDOFs;
	Vec3		physJointAnchor;

	// joint options
	F32			physJointAngLimit;
	F32			physJointLinLimit;
	F32			physJointBreakTorque;
	F32			physJointBreakForce;
	F32			physJointLinSpring;
	F32			physJointLinSpringDamp;
	F32			physJointAngSpring;
	F32			physJointAngSpringDamp;
	bool		physJointCollidesWorld;

	eNxForceType physForceType;
#endif

	Vec3		initspin;
	Vec3		spinjitter;

	F32			fadeinlength;
	F32			fadeoutlength;
	U8			alpha;

	ColorNavPoint colornavpoint[MAX_COLOR_NAV_POINTS];	//Read in from bhvr file
	ColorPath	colorpath;
	bool		bInheritGroupTint;
	bool		bTintGeom;

	Vec3		scale;
	Vec3		scalerate;
	Vec3		scaleTime;
	Vec3		endscale;
	U8			stretch;			//Whether to stretch toward the lookat param
	U8			collides;

	Vec3		pyrRotate; 
	Vec3		pyrRotateJitter;
	Vec3		positionOffset;
	bool		useShieldOffset;

	F32			trackrate;			//How fast to move toward the target
	U8			trackmethod;		//MAGNETIC, ZOOM
	int			lifespan;

	F32			animscale;

	F32			shake;
	F32			shakeFallOff;
	F32			shakeRadius;

	F32			blur;
	F32			blurFallOff;
	F32			blurRadius;

	F32			hueShift;
	F32			hueShiftJitter;

	//F32			sound_inner;	//sound bhvr
	//F32			sound_outer;
	//int			volume;

	//Splat params
	eSplatFlags	splatFlags;
	StAnim		**stAnim;
	eSplatFalloffType  splatFalloffType;
	F32			splatNormalFade;
	F32			splatSetBack;		//Percent back to set the splat from the node start
	F32			splatFadeCenter;	//fade from: 0 = fully projection start 1 = fully projection end 0.5 the middle

	//Calculated based on read in stuff
	F32			fadeinrate;
	F32			fadeoutrate;

	F32			pulsePeakTime;
	F32			pulseBrightness;
	F32			pulseClamp;
	U32*		paramBitfield;
	int			paramBitSize;


} FxBhvr;

FxBhvr * fxGetFxBhvr(char *bhvr_name);
void fxPreloadBhvrInfo();
int fxInitFxBhvr(FxBhvr* fxbhvr);

#endif
