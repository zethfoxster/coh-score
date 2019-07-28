#ifndef _PARTICLE_H
#define _PARTICLE_H

#include "mathutil.h"
#include "fxutil.h"

#define MAX_PARTICLES		50000	 //unneeded hard limit, should be limited by number updated or number drawn
#define MAX_PARTSPERSYS		1500     //goofy hard limit that should be much higher but I'm afraid to remove it this close to production
#define MAX_SYSTEMS			7500	 //another pointless hard limit I should ditch
//#define MAX_PARTSFORLITTLESYS 1500 //unused right now
//#define MAX_SYSTEMINFOS		10000 //unused right now
 
#define PARTICLE_NOTERRAIN		0
#define PARTICLE_STICKTOTERRAIN 1

#define PARTICLE_RANDOMBURBLE	0
#define PARTICLE_REGULARBURBLE	1

#define PARTICLE_FRONTFACING	0
#define PARTICLE_LOCALFACING	1		

#define PARTICLE_WORLDPOS		0	
#define PARTICLE_LOCALPOS		1

#define PARTICLE_POINT			0
#define PARTICLE_LINE			1
#define PARTICLE_CYLINDER		2
#define PARTICLE_TRAIL			3
#define PARTICLE_SPHERE			4
#define	PARTICLE_LINE_EVEN		5

#define PARTICLE_COLORSTOP		0
#define PARTICLE_COLORLOOP		1

#define PARTICLE_EXPANDFOREVER  0
#define PARTICLE_EXPANDANDSTOP  1 
#define PARTICLE_PINGPONG		2

typedef enum ParticleBlendMode {
	PARTICLE_NORMAL,
	PARTICLE_ADDITIVE,
	PARTICLE_SUBTRACTIVE,
	PARTICLE_PREMULTIPLIED_ALPHA,
	PARTICLE_MULTIPLY,
	PARTICLE_SUBTRACTIVE_INVERSE,
} ParticleBlendMode;

#define PARTICLE_ACTIVE			0
#define PARTICLE_SUSPENDED		1

#define PARTICLE_NOSTREAK		0  
#define PARTICLE_VELOCITY		1
#define PARTICLE_GRAVITY		2
#define PARTICLE_ORIGIN			3
#define PARTICLE_MAGNET			4
#define PARTICLE_OTHER			5
#define PARTICLE_CHAIN			7
#define PARTICLE_PARENT_VELOCITY	8

#define PARTICLE_NOORIENT		0
#define PARTICLE_ORIENT			1

#define PARTICLE_PULL			0
#define PARTICLE_PUSH			1

#define PARTICLE_ANIM_LOOP		0
#define PARTICLE_ANIM_ONCE		1
#define PARTICLE_ANIM_PINGPONG	2

#define PARTICLE_NOKICKSTART	0
#define PARTICLE_KICKSTART		1

#define PARTICLE_POWER_RANGE	10.f

#define PARTICLE_DISTFADELEN	20  //Distance past sys->visdist to be at zero alpha
#define PARTICLE_ALPHACUTOFF	5	//Alpha below which, the system isn't worth drawing

#define PARTICLE_UPDATE_AND_DRAW_ME			0
#define PARTICLE_UPDATE_BUT_DONT_DRAW_ME	1
#define PARTICLE_DONT_UPDATE_OR_DRAW_ME		2

#define PARTICLE_INUSE 1 //debug

enum
{
	PART_ALWAYS_DRAW				= (1 << 0),
	PART_FADE_IMMEDIATELY_ON_DEATH	= (1 << 1),
	PART_RIBBON						= (1 << 2),
	PART_IGNORE_FX_TINT				= (1 << 3)
};


typedef struct GfxNode GfxNode;

typedef struct Particle
{
	F32		age;
	F32		size;
	Vec3	position;
	Vec3	velocity;
	U8		alpha;
	U8      theta;
	S8		spin;
	U8		pulsedir:1;
	U8		thetaAccum:7;
} Particle;

typedef struct PartTex
{
	U8		texturename[64];	//converted to a name for the loaded file
	Vec3    texscroll;
	Vec3	texscrolljitter;
	F32		animframes;
	F32		animpace;
	int		animtype;

	Vec2	texscale;
	int		framewidth;
	BasicTexture * bind;
} PartTex;

typedef struct ParticleSystemInfo
{
	//List Bookkeeping
	struct  ParticleSystemInfo * next;
	struct  ParticleSystemInfo * prev;

	U32		has_velocity_jitter : 1;

	F32		fade_in_by;			//age by which it needs to be all here
	F32     fade_out_start;	//age at which the particle starts to fade 
	F32     fade_out_by;     //age by which it needs to be gone
	Vec3    velocity_jitter;    //jitter every frame in the velocity
	F32		expandrate;			//how fast to get big
	F32     tighten_up;			//attempt to pull particles toward camera 

	F32		emission_life_span; // age at which emitter stops
	F32		emission_life_span_jitter; // jitter on emission_life_span

	int		initialized;		// have we been initialized with partInitSystemInfo yet?
	int		fileAge;		//development only

	int		worldorlocalposition;
	int		frontorlocalfacing;

	F32		timetofull;
	int		kickstart;

	F32	 *	new_per_frame;		//number of new particles emitted every frame
	int  *  burst;				//initial number of particles 
	F32		movescale;			//scale the new per frame by the amount the emission point moved

	int		burbletype;
	F32		burblefrequency;
	F32	 *	burbleamplitude;
	F32		burblethreshold;

	int		emission_type;		//shape of emitted particles
	F32  *  emission_start_jitter;
	F32		emission_radius;
	F32		emission_height;

	int     tex_spin;			//per frame rotation of textures
	int     tex_spin_jitter;	//jitter in per frame tex spin;
	int     spin;				//per frame rotation of particles
	int     spin_jitter;		//jitter in per frame spin;
	int     orientation_jitter;	//jitter in per frame spin;

	F32		magnetism;			//rate at which to be attracted
	F32		gravity;			//gravity for these particles		
	int		kill_on_zero;       //kill this system when particle count=0 & no progess is being made?
	int		terrain;			//experimental, whether to stick to the terrain

	F32  *  initial_velocity;	//velocity of new particles, calculated every frame
	F32  *  initial_velocity_jitter;  //jitter in initial velovity of particles
	F32		sortBias;
	F32		drag;
	F32		stickiness;

	Vec3		colorOffset;
	Vec3		colorOffsetJitter;

	int	 *	alpha;
	int		colorchangetype;
	
	ColorNavPoint colornavpoint[MAX_COLOR_NAV_POINTS];
	ColorPath	colorPathDefault;

	ParticleBlendMode	blend_mode;			//blend mode:  Normal, additive, subtractive, etc
	
	F32	 *	startsize;		//for all particles
	F32		startsizejitter;
	F32	 *	endsize;			//how big to get	
	int		expandtype;			//how to react when the particle reaches max or min size

	int		streaktype;			//Velocity, Magnetism, Origin, Magnet, etc, etc...
	int		streakorient;
	int		streakdirection;
	F32		streakscale;		//scale factor on velocity to get streak only used for some types

	PartTex parttex[2];			//Textures with scrolling and animation info

	char*	name;

	U8		dielikethis[128];   //system to go to when the system dies...
	U8		deathagetozero;		//whether or not to reset the particle's age

	int		flags;
	//Calculated after this sysInfo is loaded   

	F32		visradius;
	F32		visdist;
	int		curralpha;
	
} ParticleSystemInfo;

typedef struct PartPerformance
{
	Vec3	boxMax;				
	Vec3	boxMin;
	F32		biggestParticleSquared;
	F32		totalSize;      //total size of all particles in the system
	F32		totalAlpha;		//total alpha of all particles in the system
} PartPerformance;

typedef struct ParticleSystem
{
	//List Bookkeeping
	struct  ParticleSystem * next;
	struct  ParticleSystem * prev;
	U8      inuse;
	F32		sortVal;
	int     unique_id;          //Used by FX system to check on it's pointer

	//Most Info
	ParticleSystemInfo * sysInfo;

	// Various Mats and points used by system to xform particles 
	Mat4	facingmat;			//current matrix to xform particles by
	Vec3	emission_start;		//point particles emit from
	Vec3	emission_start_prev;//last start location, for tracking, and emitting along a line
	Vec3    other;				//other point used in determining emission point...
	Vec3	magnet;				//location to pull particles toward
	Mat4    drawmat;
	Mat4	lastdrawmat;

	// Assorted System State variables:
	int		particle_count;		//currently
	F32     age;				//in TIMESTEP
	int		parentFxDrawState;	//FX_UPDATE_AND_DRAW_ME , FX_UPDATE_BUT_DONT_DRAW_ME, or FX_DONT_UPDATE_OR_DRAW_ME
	int		currdrawstate;		//PART_UPDATE_AND_DRAW_ME , PART_UPDATE_BUT_DONT_DRAW_ME, or PART_DONT_UPDATE_OR_DRAW_ME
	F32     new_buffer;			//part of num new left over from last frame
	char	kickstart;			//can be set by the sysInfo or the fx making it.
	int		dying;
	F32		new_per_frame;		//number of new particles emitted every frame
	F32		burbleamplitude;	//alternate number of new particles emitted every frame
	int		kill_on_zero;       //kill this system when particle count=0 & no progess is being made?
	int		burst;
	Vec3	emission_start_jitter;
	Vec3	initial_velocity_jitter;
	Vec3	initial_velocity;
	U8		alpha;
	U8		teleported;
	F32		startsize;
	F32		endsize;
	int		power;				//# between 1 and 10
	Vec3	stickinessVec;		//remove if you decide not to have stretchiness changed as a result of stickiness
	F32		timeSinceLastUpdate;//Used by performance thing only
	F32		animScale;			// From InheritAnimScale
	
	F32		camDistSqr;			//Figure sort order
	F32		emission_life_span; // age at which emitter stops

	// Visibility determination
	PartPerformance perf;

	//Texture Animation state
	Vec2    textranslate[2];	//xy xlate for part's textures 
	F32		framecurr[2];		// where is particle texture animation in animation path?
	int		animpong;			// is texture animation pinging or ponging?

	//Storage Arrays:
	F32		* verts;			//particle system verts in the form ul, ur, ll, lr.
	U8		* rgbas;			//particle system rgbas in the form ul, ur, ll, lr.
	F32		* texarray;			//ptr to array that goes texcoord[0],1,2,1,3,2  ...
	int		* tris;				//ptr to array that goes 0,1,2,1,3,2,6,7,8,7,9,8

	//TRICKY2
	int	myVertArrayId;

	//New Alloc
	int maxParticles;
	int firstParticle;
	int firstEmptyParticleSlot;

	U8 inheritedAlpha;

	U32 advanceAllParticlesToFadeOut:1; //flag that tells system to set the ages of all particles to at least the fade out time
	U32 fxtype:2;	// FX_POWERFX, etc

	ColorPath	colorPathCopy; // for color path offsets...
	ColorPath	* effectiveColorPath; // The color path to use (possibly after color tinting)

	GfxNode		* gfxnode; // Cape drawing hack

	int		geo_handle;
			
	//Particles Themselves
	Particle * particles;		//ptr to head of particle list

} ParticleSystem; 

#define MAX_PARTSPERTINYSYS 40
#define TINY_PART_VERT_BUFFER_COUNT 200

#define MAX_PARTSPERSMALLSYS 300
#define SMALL_PART_VERT_BUFFER_COUNT 20

#define MAX_PARTSPERLARGESYS 1500
#define LARGE_PART_VERT_BUFFER_COUNT 5


typedef struct ParticleEngine
{
	U8                 systems_on;
	int				   system_unique_ids;           //pool to give ids to particle systems

	//Vertex Buffer Object arrays
	int					tri_array_id;	// this is the only non-per-vertex data, needs to go in it's own object
	int               * tris;

	int					tex_array_id;
	F32				  * texarray;

	F32				  * verts;
	U8				  * rgbas;

	int				  tinyVertArrayId[TINY_PART_VERT_BUFFER_COUNT];
	int				  smallVertArrayId[SMALL_PART_VERT_BUFFER_COUNT];
	int				  largeVertArrayId[LARGE_PART_VERT_BUFFER_COUNT];

	int				  tinyVertArrayIdCurrentIdx;
	int				  smallVertArrayIdCurrentIdx;
	int				  largeVertArrayIdCurrentIdx;

	U8				  * rgbasTiny;
	U8				  * rgbasSmall;
	U8				  * rgbasLarge;

	void *				bigBuffer;  //TRICKY2 

	//Performance Numbers from Last Frame.
	int partTotalRendered;
	int partTotalUpdated;
	int particleTotal;
	F32 particleSize;

	//List Management for Particle Systems
	ParticleSystem   * systems;						//list of systems in use
	U16				   num_systems;					//number of systems in use
	ParticleSystem   * system_head;                   //head of unused list
	ParticleSystem     particle_systems[MAX_SYSTEMS]; //in this array
  
	//List Management for Particle System Infos
	ParticleSystemInfo * systeminfos;
	int				   num_systeminfos;

} ParticleEngine;

#define PART_UPDATE_AND_DRAW_ME			0
#define PART_UPDATE_BUT_DONT_DRAW_ME	1
#define PART_DONT_UPDATE_OR_DRAW_ME		2

#define PART_DEFAULT_TIMETOFULL			10.0 

// Generated by mkproto
int partGetID( ParticleSystem * system );
char * partGetName( ParticleSystem * system );
void partToggleDrawing( ParticleSystem * system, int id, int toggle );
ParticleSystem * partConfirmValid( ParticleSystem * system, int id );
ParticleSystemInfo * partGetSystemInfo( char system_name[] );
ParticleSystemInfo * partGiveNewParameters( ParticleSystem * system, int id, char * sysinfoname );
int partSetOriginMat( ParticleSystem * system, int id, Mat4 mat );
ParticleSystem * partCreateSystem(char system_name_param[], int * id, int kickstart, int power, int fxtype, F32 animScale );
void partKillSystem(ParticleSystem * system, int id);
void partSoftKillSystem(ParticleSystem * system, int id);
void partEngineInitialize();
void partPreloadParticles();
void partRunEngine(bool nostep);
void partRunSystem(ParticleSystem * system);
void partRunStartup();
void partRunShutdown();
void partSetMagnetPoint( ParticleSystem * system, int id, Vec3 point );
void partSetOtherPoint( ParticleSystem * system, int id, Vec3 point );
void partBuildParticleArray( ParticleSystem * system, F32 total_alpha, F32 * vertarray, U8 * rgbaarray, F32 clampedTimestep );
void partSetTeleported( ParticleSystem * system, int id, int teleported );
void partSetInheritedAlpha( ParticleSystem * system, int id, U8 inheritedAlpha );
void particleSystemSetDisableMode(int mode);
int particleSystemGetDisableMode();
void partKickStartAll();
// End mkproto
#endif
