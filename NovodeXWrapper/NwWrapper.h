// -------------------------------------------------------------------------------------------------------------------
// NOVODEX SDK C WRAPPER ('C' is for Cryptic)
// Written by Tom Lassanske, 03-01-05
// -------------------------------------------------------------------------------------------------------------------
#ifndef INCLUDED_NW_WRAPPER_H
#define INCLUDED_NW_WRAPPER_H

#if !BEACONIZER
#define NOVODEX 1
#endif

#if NOVODEX


#ifdef CLIENT 
//#define NOVODEX_FLUIDS 1
#endif


#define NX_CLIENT_SCENE 0

// should be multiple of 32, for bitfields
#ifdef CLIENT
#define MAX_NX_SCENES 1
#elif SERVER
#define MAX_NX_SCENES 32
#endif

#include "NwTypes.h"

// -------------------------------------------------------------------------------------------------------------------
// 'C' vs 'C++' PREPROCESSOR MANGLING                                               'C' vs 'C++' PREPROCESSOR MANGLING
// -------------------------------------------------------------------------------------------------------------------
#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus
#include "stdtypes.h"

typedef struct GroupDef GroupDef;

typedef enum ePhysicsQuality
{
	ePhysicsQuality_None,
	ePhysicsQuality_Low,
	ePhysicsQuality_Medium,
	ePhysicsQuality_High,
	ePhysicsQuality_VeryHigh,
} ePhysicsQuality;

typedef struct NxState
{
	U32		initialized;
	U32		initTick;
	U32		deinitTick;
	
	U32		debug;
	U32		threaded;
	int		staticActorCount;
	int		dynamicActorCount;
	int		maxDynamicActorCount;
	int		maxNewDynamicActors;
	int		meshShapeCount;
	int		sceneCount;
	int		actorSporeCount;
	int		gameInPhysicsPossibleState;
	int		needToReset;
	int		tainted;
	int		entCollision;
	ePhysicsQuality	physicsQuality;
#ifdef CLIENT
	int		fakeHardware;
	int		allowed;
	int		hardware;
	int		softwareOnly;
#else
	int		notAllowed;
#endif
} NxState;

typedef U32 NwEmissaryDataGuid;
typedef U32 NxActorGuid;

extern NxState nx_state;

typedef enum NxGroupType
{
	nxGroupStatic,
	nxGroupLargeDebris,
	nxGroupDebris,
	nxGroupKinematic,
	nxGroupRagdoll,
	nxGroupJointed,
	nxGroupLast
} NxGroupType;


typedef struct FxGeo FxGeo;
typedef struct FxFluidEmitter FxFluidEmitter;

typedef struct NwEmissaryData {
	void*					pEmissary;
	void*					pActor;
	NwEmissaryDataGuid		uiOurGuid;
	NxActorGuid				uiActorGuid;
	Vec3					vAccelToAdd;
	Vec3					vAccelToAddOnce;
	Vec3					vAngAccelToAddOnce;
	F32						fDrag;
	Vec3					vPhysicsHitPos;
	Vec3					vPhysicsHitNorm;
	Vec3					vJointAnchor;
	F32						fPhysicsHitForce;
	F32						fSurfaceAreaOverMass;
	F32						fJointDrag;
	U32						uiCreateTick;
	U32						uPhysicsHitObject:2;
	U32						bToAddForce:1;
	U32						bNotifyOnEndTouch:1;
	U32						bJointJustBroke:1;
	U32						bCurrentlyJointed:1;
	U32						bJointCollidesWorld:1;
} NwEmissaryData;

typedef enum eNxForceType {
	eNxForceType_None,
	eNxForceType_Out,
	eNxForceType_In,
	eNxForceType_Up,
	eNxForceType_Forward,
	eNxForceType_Side,
	eNxForceType_CWSwirl,
	eNxForceType_CCWSwirl,
	eNxForceType_Drag,
} eNxForceType;

typedef enum eNxJointDOF {
	eNxJointDOF_RotateX		=	1<<0,
	eNxJointDOF_RotateY 	= 	1<<1,
	eNxJointDOF_RotateZ 	= 	1<<2,
	eNxJointDOF_TranslateX 	= 	1<<3,
	eNxJointDOF_TranslateY 	= 	1<<4,
	eNxJointDOF_TranslateZ 	= 	1<<5,
} eNxJointDOF;

typedef enum eNxPhysDebrisType
{
	eNxPhysDebrisType_None,
	eNxPhysDebrisType_Small,
	eNxPhysDebrisType_Large,
	eNxPhysDebrisType_LargeIfHardware,
} eNxPhysDebrisType;

typedef enum eNxForceFalloff {
	eNxForceFalloff_Linear,
	eNxForceFalloff_None,
	eNxForceFalloff_Squared,
	eNxForceFalloff_InverseSquared,
} eNxForceFalloff;

// -------------------------------------------------------------------------------------------------------------------
// LOCAL UTILITY FUNCTIONS
// -------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------
// NOVODEX INIT/SHUTDOWN FUNCTIONS (immediate)
// -------------------------------------------------------------------------------------------------------------------
void  nwSetNovodexDefaults(void);
U8  nwInitializeNovodex(void);
U8  nwDeinitializeNovodex(void);
void  destroyConvexShapeCache(void);
void  nwCreateThreadQueues(void);
void  nwDeleteThreadQueues(void);
void  nwCreatePhysicsSDK(void); // Creates the NovodeX singleton instance.
void  nwDeletePhysicsSDK(void); // Deletes the NovodeX singleton instance, cleaning up all memory. 
U8  nwEnabled(void);
void  nwDeleteScene( int iScene );
int   nwCreateScene(void);      // Creates a single NovodeX scene, releasing any existing one.
U8	  nwHardwarePresent();
void nwCoreDump();
void nwConnectToPhysXDebug();
void nwSetPhysicsQuality( ePhysicsQuality pq);
void nwSetRagdollCount( int count );

// -------------------------------------------------------------------------------------------------------------------
// NOVODEX SCENE STEPPING
// -------------------------------------------------------------------------------------------------------------------
void  nwStepScenes(double stepSize); // Steps the scene if 0.016 seconds has elapsed since the last simulation step.
void  nwEndThread(int iSceneNum);

// -------------------------------------------------------------------------------------------------------------------
// NOVODEX GLOBAL READ (immediate)
// -------------------------------------------------------------------------------------------------------------------
float  nwGetDefaultGravity();



// -------------------------------------------------------------------------------------------------------------------
// MESH COOKING (delayed)
// -------------------------------------------------------------------------------------------------------------------

U8 nwCookMeshShape(const char* name, const void* vertArray, int vertCount, int vertStride, const void* triArray, int triCount, int triStride, char** streamOutput, int* streamSize);
void* nwLoadMeshShape(const char* pcStream, int iStreamSize);
void* nwCookAndLoadMeshShape(const char* name, const void* vertArray, int vertCount, int vertStride, const void* triArray, int triCount, int triStride );
void nwFreeMeshShape(void* shapeParam);
void nwFreeStreamBuffer();
void nwCookAndStoreConvexShape( const char* name, const float* verts, int vertcount, NxGroupType group, const FxGeo* geoOwner, int iSceneNum );
U8 nwFindConvexShapeDesc( const char* name, void** convexShapeDesc );
/*
void* nwLoadConvexShape(const float* verts, int vertcount);
void  nwFreeConvexShape( void* shape );
*/


// -------------------------------------------------------------------------------------------------------------------
// ACTOR CREATION/DELETION (delayed)
// -------------------------------------------------------------------------------------------------------------------
U8  nwCanAddDynamicActor(void);
void* nwCreateBoxActor(Mat4 mWorldSpaceMat, Vec3 vSize, NxGroupType group, float density, int iSceneNum );
void* nwCreateSphereActor(Mat4 mWorldSpaceMat, float fRadius, NxGroupType group, float fDensity, int iSceneNum );
NwEmissaryDataGuid  nwPushSphereActor(const float* position, const float* vel, const float* angVel, float density, float radius, NxGroupType group, const FxGeo* geoOwner, bool bDisableCollision, int iSceneNum);
NwEmissaryDataGuid  nwPushConvexActor(const char* name, Mat4 convexTransform, const Vec3 velocity, const Vec3 angVelocity, float fDensity, const FxGeo* geoOwner, int iSceneNum);
NwEmissaryDataGuid nwPushCapsuleActor(const Mat4 mCapsuleTransform, const float fRadius, const float fHeight, NxGroupType group, float fDensity, int iSceneNum);
void* nwCreateMeshActor(Mat4 meshTransform, GroupDef* def, void* shape, NxGroupType group, int iSceneNum);
void* nwCreateConvexActor(const char* name, Mat4 convexTransform, const float* verts, int vertcount, NxGroupType group, float fDensity, int iSceneNum);
void* nwCreateCapsuleActor(Mat4 mCapsuleTransform, const float fRadius, const float fHeight, NxGroupType group, float fDensity, int iSceneNum);
void  nwDeleteActor(void* actorParam, int iSceneNum);
void  nwAddJoint(NwEmissaryData* nwEmissaryData, eNxJointDOF jointDOF, const Vec3 vLocalAnchor, const Mat4 mLocalMat, F32 fLinLimit, F32 fAngLimit, F32 fMaxForce, F32 fMaxTorque, F32 fLinSpring, F32 fLinSpringDamp, F32 fAngSpring, F32 fAngSpringDamp);
NwEmissaryData* nwGetNwEmissaryDataFromGuid(NwEmissaryDataGuid uiDataGuid);
void* nwGetActorFromEmissaryGuid(NwEmissaryDataGuid uiDataGuid);


// -------------------------------------------------------------------------------------------------------------------
// ACTOR READ FUNCTIONS (immediate)
// -------------------------------------------------------------------------------------------------------------------
void  nwGetActorMat4( void* actor, float* mat4 );
void  nwGetActorLinearVelocity(void* actor, float* velocity);
void  nwGetActorAngularVelocity(void* actor, float* velocity);
F32   nwGetActorMass(void* actor);
U8  nwIsSleeping(void* actor);


// -------------------------------------------------------------------------------------------------------------------
// ACTOR WRITE FUNCTIONS (delayed)
// -------------------------------------------------------------------------------------------------------------------
void  nwSetActorMat4(NxActorGuid guidActor , float* mat4);
void  nwSetActorLinearVelocity(void* actor, const float* velocity);
void  nwSetActorAngularVelocity(void* actor, const float* velocity);
void  nwActorApplyAcceleration(void* actor, const float* acceleration, bool bPermanent);
void  nwSetActorDamping( void* actor, F32 fAngular, F32 fLinear );
void  nwSetActorSleepVelocities(void* actor, F32 fAngular, F32 fLinear);
void  nwSetActorMass(void* actor, F32 fMass );
void  nwActorSetDrag(void* actor, F32 fDrag );
void  nwAddCCDSkeletonForActor(void* actor);
void  nwRemoveCCDSkeletonForActor(void* actor);

// -------------------------------------------------------------------------------------------------------------------
// PLAYER INTERACTION
// -------------------------------------------------------------------------------------------------------------------
//void  nwUpdatePlayerPosition(Entity* pPlayerEnt);
void  nwApplyForceToEmissary(NwEmissaryData* pEmissaryData, Vec3 vEpicenter, F32 fRadius, F32 fPower, F32 fPowerJitter, F32 fCentripetal, eNxForceType eForce, Mat3Ptr pmXform, int iSceneNum);
U32  nwGatherEmissariesInSphere(Vec3 vEpicenter, F32 fRadius, NwEmissaryData** pEmissaryResults, U32 uiMaxResults, int iSceneNum);

// -------------------------------------------------------------------------------------------------------------------
// NOVODEX RAYCAST
// -------------------------------------------------------------------------------------------------------------------
int   nwRaycast(Vec3 p0, Vec3 p1, Vec3 out);




// -------------------------------------------------------------------------------------------------------------------
// NOVODEX DEBUGGING
// -------------------------------------------------------------------------------------------------------------------
void  nwPrintInfoToConsole(int toConsole);


// -------------------------------------------------------------------------------------------------------------------
// NOVODEX FLUIDS
// -------------------------------------------------------------------------------------------------------------------
#ifdef NOVODEX_FLUIDS
void* nwCreateFluidEmitter( FxFluidEmitter* fxEmitter, int iSceneNum );
void  nwUpdateFluidEmitterTransform( void* fluid, Mat4 fluidTransform );
void  nwDeleteFluid( void* fluid, int iSceneNum );
#endif

// -------------------------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
// -------------------------------------------------------------------------------------------------------------------
#endif // NOVODEX
#endif // INCLUDED_NW_WRAPPER_H
