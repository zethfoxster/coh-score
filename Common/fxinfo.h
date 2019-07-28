#ifndef _FXINFO_H
#define _FXINFO_H

#include "stdtypes.h"
#include "seq.h"
#include "textparser.h"

#if CLIENT
#include "NwWrapper.h"
#endif

typedef struct FxBhvr FxBhvr;


#define FXSTRLEN			40  //That is Event and Input names, and Condition on names
#define MAX_PSYSPERFXGEO	20	//Maximum num of psystems controled by an event/fxgeo
#define FX_NOSTRETCH		0
#define FX_STRETCH			1
#define FX_STRETCHANDTILE	2

enum
{
	MIN_ALLOWABLE_POWER = 0,
	MAX_ALLOWABLE_POWER = 1
};

typedef enum FxObjectFlags
{
	FX_STATE_TIME			= 1 << 1,
	FX_STATE_PRIMEDIED		= 1 << 2,
	FX_STATE_PRIMEHIT		= 1 << 3,
	FX_STATE_PRIMEBEAMHIT	= 1 << 4,
	FX_STATE_CYCLE			= 1 << 5,
	FX_STATE_PRIME1HIT		= 1 << 6,
	FX_STATE_PRIME2HIT		= 1 << 7,
	FX_STATE_PRIME3HIT		= 1 << 8,
	FX_STATE_PRIME4HIT		= 1 << 9,
	FX_STATE_PRIME5HIT		= 1 << 10,
	FX_STATE_TRIGGERBITS	= 1 << 11,
	FX_STATE_DEATH			= 1 << 12,
	FX_STATE_DEFAULTSURFACE	= 1 << 13,
	FX_STATE_PRIME6HIT		= 1 << 14,
	FX_STATE_PRIME7HIT		= 1 << 15,
	FX_STATE_PRIME8HIT		= 1 << 16,
	FX_STATE_PRIME9HIT		= 1 << 17,
	FX_STATE_PRIME10HIT		= 1 << 18,
	FX_STATE_COLLISION		= 1 << 19,
	FX_STATE_FORCE			= 1 << 20,
	FX_STATE_TIMEOFDAY		= 1 << 21,
} FxObjectFlags;

//Fxgeo->flags
typedef enum FxGeoFlags {
	FXGEO_LOOKAT_CAMERA			= 1 << 0,
	FXGEO_AT_CAMERA				= 1 << 1,
} FxGeoFlags;

typedef enum FxEventFlags {
	FXGEO_ADOPT_PARENT_ENTITY	= 1 << 0,
	FXGEO_NO_RETICLE			= 1 << 1,
	FXGEO_POWER_LOOPING_SOUND	= 1 << 2,
	FXGEO_ONE_AT_A_TIME			= 1 << 3,
} FxEventFlags;

typedef enum FxCollisionEventTransformFlags
{
	FX_CEVENT_USE_COLLISION_NORM		= ( 0 ),
	FX_CEVENT_USE_WORLD_UP		= ( 1 ),
} FxCollisionEventTransformFlags;

typedef enum FxTransformFlags
{
	FX_NONE					=	0,
	FX_POSITION				=	1 << 0,
	FX_ROTATION				=	1 << 1,
	FX_SCALE				=	1 << 2,
	FX_ROTATE_ALL			=	1 << 3,
    FX_POS_AND_ROT          =   FX_POSITION | FX_ROTATION,
	FX_ALL					=	FX_POSITION | FX_ROTATION | FX_SCALE,
} FxTransformFlags;

typedef enum FxType
{
	FxTypeNone,
	FxTypeCreate,
	FxTypeDestroy,
	FxTypeLocal,
	FxTypeStart,
	FxTypePosit,
	FxTypeStartPositOnly,
	FxTypePositOnly,
	FxTypeSetState,
	FxTypeIncludeFx,
	FxTypeSetLight,
} FxType;

typedef enum FxInfoFlags {
	FX_SOUND_ONLY			=  1 << 0,
	FX_INHERIT_ALPHA		=  1 << 1,
	FX_IS_ARMOR				=  1 << 2,	/* no longer used */
	FX_INHERIT_ANIM_SCALE	=  1 << 3,
	FX_DONT_INHERIT_BITS	=  1 << 4,
	FX_DONT_SUPPRESS		=  1 << 5,
	FX_DONT_INHERIT_TEX_FROM_COSTUME =  1 << 6,
	FX_USE_SKIN_TINT		=  1 << 7,
	FX_IS_SHIELD			=  1 << 8,		// fx spawns shield geo (used in conjunction with "HideShield" Move flag
	FX_IS_WEAPON			=  1 << 9,		// fx spawns weapon (used in conjunction with "HideWeapon" Move flag -- only need this if geo on bone other than WepR/WepL
    FX_NOT_COMPATIBLE_WITH_ANIMAL_HEAD =  1 << 10, // dont spawn this fx on character's using animal head.  Yes, this is a hack.
	FX_INHERIT_GEO_SCALE	=  1 << 11,		// scale fx geo by character's "physique" scale value
	FX_USE_TARGET_SEQ		=  1 << 12,		// fx updates using the target sequencer instead of the parent (for HitFX + Triggerbits)
} FxInfoFlags;

typedef enum FxInfoLighting
{
	FX_PARENT_LIGHTING  = 0, //parent or none
	FX_DYNAMIC_LIGHTING = 1,
	//FX_STATIC_LIGHTING = 2, //performance thing to do
} FxInfoLighting;


typedef struct splatParams
{
	char * texture1;
	char * texture2;
} SplatEvent;

typedef struct SoundEvent
{
	char *soundName;
	F32 radius;
	F32 fade;
	F32 volume;
	U32 isLoopingSample:1;
} SoundEvent;

typedef struct FxEvent
{
	char		*name;	
	FxType		type;
	FxTransformFlags	inherit;
	FxTransformFlags	update;
	char		*at;
	char		*bhvr;
	FxBhvr**	bhvrOverride;
	char		*jevent;
	char		*cevent;
	F32			cthresh;
	U8			cdestroy;
	U8			jdestroy;
	bool		bHardwareOnly;
	bool		bSoftwareOnly;
	bool		bPhysicsOnly;
	bool		bCameraSpace;
	char		*atRayFx;
	F32			fRayLength;
	FxCollisionEventTransformFlags			crotation;
	F32			parentVelFraction;
	TokenizerParams ** capeFiles;
	TokenizerParams ** geom;
	int			altpiv;
	char		*animpiv;
	char		*anim;
	char		*newstate;
	TokenizerParams ** part;
	char		*childfx;
	char		*lookat;
	char		*magnet;
	char		*pmagnet;
	char		*pother;
	SplatEvent	** splat;
	//char		*sound;
	//TokenizerParams ** sound;
	SoundEvent  **sound;
	int			soundNoRepeat;
	char		* worldGroup;
	U8			power[2];	//power range at which this event is actually triggered
	F32			lifespan;
	F32			lifespanjitter;
	FxEventFlags fxevent_flags;
#if CLIENT
	int			*untilstate;
	int			*whilestate;
	int			*soundHistory; // Indices of last few sounds played.
#endif
	TokenizerParams ** untilbits;  //condition to kill this guy
	TokenizerParams ** whilebits;  //condition to kill this guy

} FxEvent;

typedef struct FxInput
{
	char		*name;
} FxInput;

typedef struct FxCondition
{
	char		*on;
	F32			time;
	F32			dayStart;
	F32			dayEnd;
	F32			dist;
	F32			chance;
	U8			domany;
	U8			repeat;
	U8			repeatJitter;
	U8			randomEvent;
	FxEvent		**events;
	F32			forceThreshold;
	
	int			trigger;		//on is translated into this that IDs it

#if CLIENT
	int			**triggerstates;
#endif
	TokenizerParams ** triggerbits;

} FxCondition;

#if CLIENT
	typedef struct PerfInfoStaticData PerfInfoStaticData;
	typedef struct PerformanceInfo PerformanceInfo;
#endif

typedef struct FxInfo
{
	struct FxInfo * next; //development only for listing non binary loaded fx
	struct FxInfo * prev;

	char		*name;
	int			hasEventsOnDeath; //kinda funky, get rid of someday.
	U8			initialized;	// have we been init'd by fxInitFxInfo?
	int			fileAge;		// development only
	
	#if CLIENT
		#if USE_NEW_TIMING_CODE
			PerfInfoStaticData*	perfInfo;
		#else
			PerformanceInfo*	perfInfo;
		#endif
	#endif

	char 		*inputsReal[128];
	int			inputsRealCount;

	//Performance info
	FxInfoFlags	fxinfo_flags;   //FX_SOUND_ONLY 
	F32			performanceRadius;

	//Read in
	
	int			lifespan;
	FxInfoLighting	lighting;   //PARENT/NONE, STATIC, DYNAMIC, PARENT
	FxInput		**inputs;
	FxCondition **conditions;

	F32			animScale; // From inheritAnimScale or hardcoded
	F32			onForceRadius;

	F32			effectiveHue;
	U8			effectiveHueCalculated			: 1;
	U8			hasForceNode					: 1;

	Vec3		clampMinScale;
	Vec3		clampMaxScale;

	bool		bDontInheritTexturesFromCostumePart;

} FxInfo;

FxInfo * fxGetFxInfo(const char fx_name[]);
void fxPreloadFxInfo(void);
void fxBuildFxStringHandles(void);
void fxInfoCalcEffectiveHue(FxInfo *fxinfo);
void fxPreloadGeometry(void);
bool fxInfoHasFlags(FxInfo *fxinfo, FxInfoFlags flags, bool bIncludeChildFx);

#endif
