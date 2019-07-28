#ifndef _CLOTH_H
#define _CLOTH_H

#include "stdtypes.h"
#include "mathutil.h"
#include "ClothDefs.h"

//============================================================================
// DEFINITIONS
//============================================================================

#define CLOTH_SUB_LOD_NUM 4

enum {
	CLOTH_FLAG_DEBUG_COLLISION = 1,
	CLOTH_FLAG_DEBUG_POINTS = 2,
	CLOTH_FLAG_DEBUG_IM_POINTS = 4,
	CLOTH_FLAG_DEBUG_TANGENTS = 8,
	CLOTH_FLAG_DEBUG_AND_RENDER = 16,
	CLOTH_FLAG_DEBUG_NORMALS = 32,
	CLOTH_FLAG_DEBUG_HARNESS = 64,
	CLOTH_FLAG_DEBUG = 0xff,
	CLOTH_FLAG_RIPPLE_VERTICAL = 0x100,	// Produces nice ripples for flags.
	CLOTH_FLAG_MASK = 0xffff
};

//============================================================================
// RENDER SUPPORT DEFINITIONS
//============================================================================

#define CLOTH_NORMALIZE_NORMALS 1
#define CLOTH_CALC_BINORMALS 1
#define CLOTH_NORMALIZE_BINORMALS 0
#define CLOTH_CALC_TANGENTS 1
#define CLOTH_NORMALIZE_TANGENTS 0

#define CLOTH_SCALE_NORMALS 0

//============================================================================
// PROTOTYPES
//============================================================================

// Defined in Cloth.h
typedef struct Cloth Cloth;
typedef struct ClothAttachment ClothAttachment;
typedef struct ClothLengthConstraint ClothLengthConstraint;
typedef struct ClothLOD ClothLOD;
typedef struct ClothObject ClothObject;
// Defined in ClothCol.h
typedef struct ClothCol ClothCol;
// Defined in ClothMesh.h
typedef struct ClothMesh ClothMesh;
typedef struct ClothStrip ClothStrip;

typedef struct ClothNodeData ClothNodeData;

//============================================================================
// CLOTH
//============================================================================

typedef struct ClothCommonData
{
	// Data shared between main/threaded data
	//   Pointers should be to things that don't chang after init
	//   Other values are copied per frame

	S32 Width, Height;
	S32 NumParticles; 	// = Width * Height
	S32 SubLOD;
	const ClothLengthConstraint *LengthConstraints; // Const pointer to make sure no one changes this after init

	// ----------------------------------------
	// Attachment (Hook / Eyelet) Data
	S32 NumAttachments;
	S32 MaxAttachments; // Largest index into hooks that is used
	ClothAttachment *Attachments;

	F32 *ApproxNormalLens;	// Used for fast normal calculation

} ClothCommonData;

typedef struct ClothRenderData
{
	ClothCommonData commonData;

	// Data used only by rendering

	// One element per Rendered Particle
	Vec3 *RenderPos;		// Buffer for positions to be used by the renderer (copied from CurPos)
	Vec2 *TexCoords; 		// Array of texture coords

	// One element per rendered particle
	Vec3 *Normals;	// Array of normals
#if CLOTH_CALC_BINORMALS
	Vec3 *BiNormals;	// Array of binormals (for bumpmapping)
#endif
#if CLOTH_CALC_TANGENTS
	Vec3 *Tangents;		// Array of tangents (for bumpmapping)
#endif
	F32 *NormalScale; // - = relative min, + = relative max, 1 / particle

	Vec3 *hookNormals; // Filled in per-frame and sent to renderer

	ClothMesh *currentMesh;
	S32 CurSubLOD;

	Cloth *debug_cloth_backpointer; // For debugging, shouldn't be used in code

} ClothRenderData;

struct Cloth
{
	S32 Flags;			// Mostly debug, also RIPPLE_VERTICAL

	// ----------------------------------------
	// Constants (set on creation)
	ClothCommonData commonData;
	S32 MinSubLOD;		// Determines how many positions to allocate
	F32 PointColRad;	// Collision radius of Modeled Particles
	S32 NumIterations[CLOTH_SUB_LOD_NUM];	// How many times to iterate at each sublod

	// One element per Modeled Particle
	Vec3 *OrigPos; 			// Origional Positions
	F32 *InvMasses; 		// Array of 1/mass
	Vec3 *PosBuffer1; 		// CurPos or OldPos points to this
	Vec3 *PosBuffer2; 		// OldPos or CurPos points to this
	
	ClothRenderData renderData;

	// ----------------------------------------
	// Calculated Each Frame
	Vec3 MaxPos, MinPos;
	Vec3 CenterPos, DeltaPos;

	// One element per Modeled Particle
	F32 *ColAmt; 			// Collision amount each frame

	// ----------------------------------------
	// Pointers
	Vec3 *CurPos; // Points to PosBuffer1 or PosBuffer2
	Vec3 *OldPos; // Points to PosBuffer2 or PosBuffer1

	// ----------------------------------------
	// Constraint Data
	S32 NumLengthConstraints[CLOTH_SUB_LOD_NUM];
	S32 NumConnections;
	ClothLengthConstraint *LengthConstraintsInit; // Version only written two during initialization
	// in commonData: const ClothLengthConstraint *LengthConstraints; // Const pointer to make sure no one changes this after init

	// ----------------------------------------
	// Collision Data
	S32 NumCollidables;
	ClothCol *Collidables;

	// ----------------------------------------
	// Private data
	// in commonData: S32 SubLOD;
	S32 SkipUpdatePhysics; // set when movement during a frame is too severe
	
	// ----------------------------------------
	// Physics
	Mat4 Matrix;
	Vec4 mQuat; // From matrix
	Mat4 EntMatrix;
	Vec3 Vel;
	F32 Gravity;
	F32 Drag;
	F32 Time;
	// Wind
	Vec3 WindDir;			// [-1, 1]
	F32 WindMag;			// [ 0, 2] (normally [0, 1])
	F32 WindScaleFactor;	// ~= 64.0 ft/s (scales [-1, 1] -> ft/s; 64 ~= 2x gravity)
	F32 WindRippleAmt;		// [-1, 1]
	F32 WindRippleRepeat;	// number of waves across cloak
	F32 WindRipplePeriod;	// seconds

	Vec3 PositionOffset;	// Offset to all verts from actual world coordinates, add this to a Vec3 to get to world space, subtract it from a Vec3 to get to Cloth space
};

// Create
extern Cloth *ClothCreate(int minsublod, int width, int height);
extern Cloth *ClothCreateEmpty(int minsublod);
extern Cloth *ClothCreateFromPoints(int minsublod, int width, int height, Vec3 *points, F32 *masses);
extern Cloth *ClothCreateFromGrid(int minsublod, int width, int height, Vec3 dxyz, Vec3 origin, F32 xscale, int arc);
extern void ClothCalcLengthConstraints(Cloth *cloth, S32 flags, int nfracscales, F32 *in_fracscales);
extern void ClothSetParticle(Cloth *cloth, int pidx, Vec3 pos, F32 mass);
extern int  ClothAddAttachment(Cloth *cloth, int pidx, int hook1, int hook2, F32 frac);
extern void ClothDelete(Cloth * cloth);

// Build (ClothBuild.c)
extern void ClothBuildGrid(Cloth *cloth, int width, int height, Vec3 dxyz, Vec3 origin, F32 xscale, int arc);
extern int ClothCreateParticles(Cloth *cloth, int width, int height);
extern void ClothSetPointsMasses(Cloth *cloth, Vec3 *points, F32 *masses);
extern int ClothBuildGridFromTriList(Cloth *cloth, int npoints, Vec3 *points, Vec2 *texcoords, F32 *masses, F32 mass_y_scale, F32 stiffness, int ntris, int *tris, Vec3 scale);
extern int ClothBuildLOD(Cloth *newcloth, Cloth *incloth, int lod, F32 point_y_scale, F32 stiffness);
extern int ClothBuildAttachHarness(Cloth *cloth, int hooknum, Vec3 *hooks);

// Physics
extern void ClothSetGravity(Cloth *cloth, F32 g);
extern void ClothSetDrag(Cloth *cloth, F32 drag);
extern void ClothSetWind(Cloth *cloth, Vec3 dir, F32 speed, F32 scale);
extern void ClothSetWindRipple(Cloth *cloth, F32 amt, F32 repeat, F32 period);

// Collisions
extern void ClothSetColRad(Cloth *cloth, F32 r);
extern int  ClothAddCollidable(Cloth *cloth);
extern ClothCol *ClothGetCollidable(Cloth *cloth, int idx);

// Update
extern int ClothCheckMaxMotion(Cloth *cloth, F32 dt, Mat4 newmat, int freeze);
extern void ClothUpdateAttachments(Cloth *cloth, Vec3 *hooks, F32 ffrac, int freeze);
extern void ClothUpdatePosition(Cloth *cloth, F32 dt, Mat4 newmat, Vec3 *hooks, int freeze);
extern void ClothUpdatePhysics(Cloth *cloth, F32 dt);
extern void ClothUpdateDraw(ClothRenderData *cloth);
extern void ClothOffsetInternalPosition(Cloth *cloth, const Vec3 newPositionOffset);

// Error Handling
extern S32  ClothError; // 0 = no error
extern char ClothErrorMessage[];


//============================================================================
// CLOTH ATTACHMENT (EYELET)
//============================================================================

struct ClothAttachment
{
	S32 PIdx;	// Particle Idx
	F32 Frac;	// pos = Hooks[Hook1] * Frac + Hooks[Hook2] * (1-Frac)
	S32 Hook1;	// 1st Index into Hooks
	S32 Hook2;	// 2nd Index into Hooks
};

//============================================================================
// CLOTH LENGTH CONSTRAINT
//============================================================================

struct ClothLengthConstraint
{
	S16 P1;
	S16 P2;
	F32 RestLength; // if < 0, -RestLength = minimum distance
	F32 InvLength; // 1/dist(P1,P2)
	F32 MassTot2; // 2 / (P1->InvMass + P2->InvMass)
};

//============================================================================
// CLOTH LOD
//============================================================================
// A ClothLOD consists of a Cloth structure
// and one more more meshes for each sub-lod

struct ClothLOD
{
	Cloth *mCloth;
	ClothMesh *Meshes[CLOTH_SUB_LOD_NUM];
	S32 MaxSubLOD;
};

//============================================================================
// CLOTH OBJECT
//============================================================================
// A cloth object consists of one or more ClothLOD's and some state info

struct ClothObject
{
	S32 NumLODs;
	S32 CurLOD;
	S32 CurSubLOD;
	ClothLOD **LODs;
	ClothNodeData *GameData;	// implementation hook
	Vec3 *debugHookPos;
	Vec3 *debugHookNormal;
};

extern ClothObject *ClothObjectCreate();
extern void ClothObjectDelete(ClothObject *obj);

extern int ClothObjectAddLOD(ClothObject *obj, int minsublod, int maxsublod);
extern int ClothObjectCreateLODs(ClothObject *obj, int maxnum, F32 point_y_scale, F32 stiffness);
extern Cloth *ClothObjGetLODCloth(ClothObject *obj, int lod);
extern ClothMesh *ClothObjGetLODMesh(ClothObject *obj, int lod, int sublod);
extern int ClothObjectSetLOD(ClothObject *obj, int lod);
extern int ClothObjectSetSubLOD(ClothObject *obj, int sub);
extern ClothMesh * ClothObjectGetLODMesh(ClothObject *obj, int lod, int sublod);
extern int ClothObjectDetailNum(ClothObject *obj);
extern int ClothObjectSetDetail(ClothObject *obj, int detail);

extern int ClothObjectParseText(ClothObject *obj, const char *intext);
extern int ClothObjectCreateMeshes(ClothObject *obj);

extern void ClothObjectSetColRad(ClothObject *obj, F32 colrad);
extern void ClothObjectSetGravity(ClothObject *obj, F32 g);
extern void ClothObjectSetDrag(ClothObject *obj, F32 drag);
extern void ClothObjectSetWind(ClothObject *obj, Vec3 dir, F32 speed, F32 maxspeed);
extern void ClothObjectSetWindRipple(ClothObject *obj, F32 amt, F32 repeat, F32 period);

extern int  ClothCheckMaxMotion(Cloth *cloth, F32 dt, Mat4 newmat, int freeze);
extern void ClothObjectUpdatePosition(ClothObject *clothobj, F32 dt, Mat4 newmat, Vec3 *hooks, int freeze);
extern void ClothObjectUpdatePhysics(ClothObject *clothobj, F32 dt);
extern void ClothObjectUpdateDraw(ClothObject *clothobj);
extern void ClothObjectUpdate(ClothObject *clothobj, F32 dt);

//////////////////////////////////////////////////////////////////////////////

#endif // _CLOTH_H
