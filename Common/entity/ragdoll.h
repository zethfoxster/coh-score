#ifndef RAGDOLL_H__
#define RAGDOLL_H__

#include "bones.h"

#if (defined CLIENT || defined SERVER) && !defined TEST_CLIENT && !BEACONIZER
#define RAGDOLL 1
#endif

#define RAGDOLL_HIST_MAX 64

#define NUM_BITS_PER_RAGDOLL_QUAT_ELEM 10

typedef struct Ragdoll
{
	struct Ragdoll* child;
	struct Ragdoll* next;
	struct Ragdoll* parent;
	BoneId			boneId;
	void* pActor;
	void* pJointToParent;
	U8			numBones;
	Quat	qCurrentRot;
//	Vec3		vCurrentPos;
	U8			hist_latest;
	Quat	qRotHist[RAGDOLL_HIST_MAX];
	U32			absTimeHist[RAGDOLL_HIST_MAX];
	F32			fRecursiveMass;
	int			iSceneNum;
} Ragdoll;

typedef struct SeqInst SeqInst;
typedef struct GfxNode GfxNode;

void compressRagdoll( Ragdoll* ragdoll, int* iQuatCompressed );

void decompressRagdoll( Ragdoll* ragdoll, int* iQuatCompressed, U32 uiTimeStamp );

void initNonRagdollBoneRecord(Quat** qRecord, SeqInst* seq, GfxNode* gfxnode );

int interpolateNonRagdollSkeleton(F32 fRatio, SeqInst* seq, GfxNode* gfxnode, Quat* qRec);

int setSkeletonIdentity(SeqInst* seq, GfxNode* gfxnode);

void setSkeletonFromRagdoll( Ragdoll* newRagdoll, SeqInst* seq, GfxNode* gfxnode  );

bool isRagdollBone(BoneId boneId);

bool interpBetweenTwoRagdolls( Ragdoll* a, Ragdoll* b, Ragdoll* c, F32 fRatio);

void interpRagdoll( Ragdoll* ragdoll, U32 uiClientTime );

U32 firstValidRagdollTimestamp( Ragdoll* ragdoll );

U32 secondValidRagdollTimestamp( Ragdoll* ragdoll );

F32 calculateRecursiveMass( Ragdoll* ragdoll );


bool clientHasCaughtUpToRagdollStart( Ragdoll* ragdoll, U32 uiClientTime );

Ragdoll* findNthRagdollBone( Ragdoll* root, U8* uiN );

Ragdoll* findRagdollBoneFromBoneId( Ragdoll* root, BoneId boneId );

#ifdef SERVER
void drawRagdoll( Ragdoll* ragdoll, Mat4 mRoot);
#endif


#endif
