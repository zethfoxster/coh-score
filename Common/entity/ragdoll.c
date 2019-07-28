#include "ragdoll.h"
#include "netcomp.h"
#include "mathutil.h"
#include "gfxtree.h"
#include "seq.h"
#include "NwWrapper.h"
#include "Quat.h"

void compressOrDecompressRagdoll( Ragdoll* ragdoll, int** iQuatCompressed, bool bCompress, U32 uiTimeStamp)
{
	// Recurse through tree
	Ragdoll* childRagdoll = ragdoll->child;
	int j;
	while (childRagdoll)
	{
		compressOrDecompressRagdoll(childRagdoll, iQuatCompressed, bCompress, uiTimeStamp);
		childRagdoll = childRagdoll->next;
	}

//	if ( !ragdoll->pActor )
//		return;
	
	if (bCompress)
	{

		quatForceWPositive(ragdoll->qCurrentRot);

		// Do the actual compression
		for(j=0;j<3;j++)
		{
			**iQuatCompressed = packQuatElem(ragdoll->qCurrentRot[j],NUM_BITS_PER_RAGDOLL_QUAT_ELEM);
			(*iQuatCompressed)++;
		}
	}
	else // decompress
	{
		// Advance the circular array index
		ragdoll->hist_latest = (ragdoll->hist_latest+1)%ARRAY_SIZE(ragdoll->absTimeHist);
		for(j=0;j<3;j++)
		{
			ragdoll->qRotHist[ragdoll->hist_latest][j] = unpackQuatElem(**iQuatCompressed, NUM_BITS_PER_RAGDOLL_QUAT_ELEM);
			(*iQuatCompressed)++;
		}

		ragdoll->absTimeHist[ragdoll->hist_latest] = uiTimeStamp;

		quatCalcWFromXYZ(ragdoll->qRotHist[ragdoll->hist_latest]);
		// shouldn't be necessary
		quatNormalize(ragdoll->qRotHist[ragdoll->hist_latest]);

	}
}

void compressRagdoll( Ragdoll* ragdoll, int* iQuatCompressed )
{
	compressOrDecompressRagdoll(ragdoll, &iQuatCompressed, 1, 0 );
}


void decompressRagdoll( Ragdoll* ragdoll, int* iQuatCompressed, U32 uiTimeStamp )
{
	compressOrDecompressRagdoll(ragdoll, &iQuatCompressed, 0, uiTimeStamp);
}

void drawRagdoll( Ragdoll* ragdoll, Mat4 mRoot )
{
	Ragdoll* child = ragdoll->child;
	
	while (child)
	{
		child = child->next;
	}
}

// This find's the nth bone we come along while recursing the tree
Ragdoll* findNthRagdollBone( Ragdoll* root, U8* uiN )
{

	// Recurse through tree
	Ragdoll* childRagdoll = root->child;

	if ( *uiN == 0)
		return root;

	(*uiN)--;

	while (childRagdoll)
	{
		Ragdoll* childSearch = NULL;
		childSearch = findNthRagdollBone(childRagdoll, uiN);
		if (childSearch)
			return childSearch;
		childRagdoll = childRagdoll->next;
	}
	return NULL;
}

Ragdoll* findRagdollBoneFromBoneId( Ragdoll* root, BoneId boneId )
{
	// Recurse through tree
	Ragdoll* childRagdoll = root->child;

	if (boneId == root->boneId)
		return root;
	while (childRagdoll)
	{
		Ragdoll* childSearch = NULL;
		childSearch = findRagdollBoneFromBoneId(childRagdoll, boneId);
		if (childSearch)
			return childSearch;
		childRagdoll = childRagdoll->next;
	}
	return NULL;
}

void getIdMat3ForBone(BoneId boneId, Mat3 mID)
{
	copyMat3( unitmat, mID );
	switch (boneId)
	{
		xcase BONEID_F1_R:
		case  BONEID_F2_R:
			rollMat3(RAD(90.0f), mID);

		xcase BONEID_F1_L:
		case  BONEID_F2_L:
			rollMat3(RAD(-90.0f), mID);

		xcase BONEID_T1_R:
		case  BONEID_T1_L:
			pitchMat3(RAD(-90.0f), mID);

		xcase BONEID_T2_R:
		case  BONEID_T2_L:
			// do nothing

		xcase BONEID_T3_R:
			pitchMat3(RAD(-140.0f), mID);
			yawMat3(RAD(40.0f), mID);

		xcase BONEID_T3_L:
			pitchMat3(RAD(-140.0f), mID);
			yawMat3(RAD(-40.0f), mID);
	};
}

void initNonRagdollBoneRecord(Quat** qRecord, SeqInst* seq, GfxNode* gfxnode )
{
	Quat* qRec;
	GfxNode * node; 
	if ( !(*qRecord) )
		*qRecord = calloc(256, sizeof(Quat));
	qRec = *qRecord;
	for(node = gfxnode ; node ; node = node->next)       
	{  
		if( seq->handle == node->seqHandle && !isRagdollBone(node->anim_id))
		{
			Mat4 mID;
			getIdMat3ForBone(node->anim_id, mID);
			mat3ToQuat(mID, qRec[node->anim_id]);
		}
		if ( node->child )
			initNonRagdollBoneRecord(qRecord, seq, node->child);
	}
}

int interpolateNonRagdollSkeleton(F32 fRatio, SeqInst* seq, GfxNode* gfxnode, Quat* qRec)
{
	GfxNode * node; 
	int iTotal=0;
	assert(qRec);
	for(node = gfxnode ; node ; node = node->next)       
	{  
		if( seq->handle == node->seqHandle && !isRagdollBone(node->anim_id)) //Dont try to animate things that aren't part of this anim, or aren't used by anything right now
		{
            // Convert target
			Quat qCurRot, qNewRot;
			quatNormalize(qRec[node->anim_id]);
			mat3ToQuat(node->mat, qCurRot);

			// interp from old to target
			quatInterp(fRatio, qRec[node->anim_id], qCurRot, qNewRot );
//			copyQuat(qRec[node->anim_id], qNewRot);

			// change node
			quatToMat(qNewRot, node->mat);

			// copy result to old
			copyQuat(qNewRot, qRec[node->anim_id]);
			iTotal++;
		}
		if ( node->child )
			iTotal += interpolateNonRagdollSkeleton(fRatio, seq, node->child, qRec);
	}

	return iTotal;
}


int setSkeletonIdentity(SeqInst* seq, GfxNode* gfxnode)
{
	GfxNode * node; 
	int iTotal=0;
	for(node = gfxnode ; node ; node = node->next)       
	{  
		if( seq->handle == node->seqHandle && !isRagdollBone(node->anim_id)) //Dont try to animate things that aren't part of this anim, or aren't used by anything right now
		{  
			Mat4 mID;
			getIdMat3ForBone(node->anim_id, mID);
			copyMat3(mID, node->mat );
			iTotal++;
		}
		if ( node->child )
			iTotal += setSkeletonIdentity(seq, node->child);
	}
	return iTotal;
}


void setSkeletonFromRagdoll( Ragdoll* newRagdoll, SeqInst* seq, GfxNode* gfxnode )
{
	GfxNode * node; 

	for(node = gfxnode ; node ; node = node->next)       
	{  
		if( seq->handle == node->seqHandle ) //Dont try to animate things that aren't part of this anim, or aren't used by anything right now
		{  
			Ragdoll* thisBoneRagdoll;

			thisBoneRagdoll = findRagdollBoneFromBoneId( newRagdoll, node->anim_id );

			if (thisBoneRagdoll && isRagdollBone(node->anim_id) )
			{
// 				if(node->anim_id == BONEID_HIPS)
// 				{
// 					copyMat4(unitmat, node->mat);
// 				}
// 				else
				{
					quatToMat(thisBoneRagdoll->qCurrentRot, node->mat);
					// Snakeman hack! (bone BONEID_FORE_FOOTL)
					if(node->anim_id == BONEID_HIPS)//|| node->anim_id == BONEID_FORE_FOOTL)
						zeroVec3(node->mat[3]);
					//copyVec3(thisBoneRagdoll->vCurrentPos, node->mat[3]);
				}
			}

			if(node->child)
				setSkeletonFromRagdoll(newRagdoll, seq, node->child );
		}
	}
}


bool isRagdollBone(BoneId boneId)
{
	switch (boneId)
	{
	case BONEID_HIPS:
		return true;
	case BONEID_WAIST:
		return false;
	case BONEID_CHEST:
		return true;
	case BONEID_HEAD:
		return true;
	case BONEID_UARMR:
	case BONEID_UARML:
	case BONEID_LARMR:
	case BONEID_LARML:
		return true;
	case BONEID_ULEGR:
	case BONEID_ULEGL:
	case BONEID_LLEGR:
	case BONEID_LLEGL:
		return true;
	case BONEID_HANDR:
	case BONEID_HANDL:
	case BONEID_FOOTR:
	case BONEID_FOOTL:
		return false;
	//snakeman
	case BONEID_HIND_ULEGL:
	case BONEID_HIND_LLEGL:
	case BONEID_HIND_FOOTL:
	case BONEID_HIND_TOEL:
	case BONEID_FORE_ULEGL:
	case BONEID_FORE_FOOTL:
	case BONEID_FORE_ULEGR:
	case BONEID_FORE_LLEGR:
		return false;
	default:
		return false;
	}
}

bool interpBetweenTwoRagdolls( Ragdoll* a, Ragdoll* b, Ragdoll* c, F32 fRatio)
{
	// walk down ragdoll a, using it's boneids to find corresponding bones in b and c
	Ragdoll* childRagdoll = a->child;
	while (childRagdoll)
	{
		if(!interpBetweenTwoRagdolls(childRagdoll, b, c, fRatio))
			return false;

		childRagdoll = childRagdoll->next;
	}

	{
		Ragdoll* bCorrespondingBone = findRagdollBoneFromBoneId(b, a->boneId);
		Ragdoll* cCorrespondingBone = findRagdollBoneFromBoneId(c, a->boneId);

		if(!(bCorrespondingBone && cCorrespondingBone))
			return false;

		quatInterp(fRatio, a->qCurrentRot, bCorrespondingBone->qCurrentRot, cCorrespondingBone->qCurrentRot);
	}
	return true;
}


void interpRagdoll( Ragdoll* ragdoll, U32 uiClientTime )
{
	Ragdoll* childRagdoll = ragdoll->child;
	while (childRagdoll)
	{
		interpRagdoll(childRagdoll, uiClientTime);
		childRagdoll = childRagdoll->next;
	}

	// First, find the indices corresponding to the client time, if they exist
	{
		U8 uiInterpEndIndex;
		U8 uiInterpBeginIndex = ragdoll->hist_latest;
		while (ragdoll->absTimeHist[uiInterpBeginIndex] > uiClientTime)
		{
			uiInterpBeginIndex = (uiInterpBeginIndex-1)%ARRAY_SIZE(ragdoll->absTimeHist);
			if ( uiInterpBeginIndex == ragdoll->hist_latest )
			{
				break; // we've looped
			}
		}

		if ( ragdoll->absTimeHist[uiInterpBeginIndex] == 0 )
		{
			assert(0); // we should have tested this before calling this function
			// client is ahead of history, so just use last value, should never happen
			copyQuat(ragdoll->qRotHist[ragdoll->hist_latest], ragdoll->qCurrentRot);
			return;
		}
		uiInterpEndIndex = (uiInterpBeginIndex+1)%ARRAY_SIZE(ragdoll->absTimeHist);

		if ( ragdoll->absTimeHist[uiInterpEndIndex] > ragdoll->absTimeHist[uiInterpBeginIndex] )
		{
			float fTimeStep, fRatio;
			fTimeStep = ragdoll->absTimeHist[uiInterpEndIndex] - ragdoll->absTimeHist[uiInterpBeginIndex];
			fRatio = (uiClientTime - ragdoll->absTimeHist[uiInterpBeginIndex] ) / fTimeStep;
			quatInterp(fRatio, ragdoll->qRotHist[uiInterpBeginIndex], ragdoll->qRotHist[uiInterpEndIndex], ragdoll->qCurrentRot);
		}
		else
		{
			copyQuat(ragdoll->qRotHist[uiInterpBeginIndex], ragdoll->qCurrentRot);
		}
	}
}

U32 firstValidRagdollTimestamp( Ragdoll* ragdoll )
{
	U8 uiInterpIndex = ragdoll->hist_latest;
	while (ragdoll->absTimeHist[uiInterpIndex] > 0)
	{
		uiInterpIndex = (uiInterpIndex-1)%ARRAY_SIZE(ragdoll->absTimeHist);
		if ( uiInterpIndex == ragdoll->hist_latest )
		{
			break; // we've looped around
		}
	}

	// We've found the last invalid timestamp, or we've wrapped around to the last valid timestamp
	// Either way, the first valid timestamp is the next element in the circular array
	uiInterpIndex = (uiInterpIndex+1)%ARRAY_SIZE(ragdoll->absTimeHist);

//	assert( ragdoll->absTimeHist[uiInterpIndex] != 0 );
	return ragdoll->absTimeHist[uiInterpIndex];
}

U32 secondValidRagdollTimestamp( Ragdoll* ragdoll )
{
	U8 uiInterpIndex = ragdoll->hist_latest;
	while (ragdoll->absTimeHist[uiInterpIndex] > 0)
	{
		uiInterpIndex = (uiInterpIndex-1)%ARRAY_SIZE(ragdoll->absTimeHist);
		if ( uiInterpIndex == ragdoll->hist_latest )
		{
			break; // we've looped around
		}
	}

	// We've found the last invalid timestamp, or we've wrapped around to the last valid timestamp
	// Either way, the second valid timestamp is the second element past this one in the circular array
	uiInterpIndex = (uiInterpIndex+2)%ARRAY_SIZE(ragdoll->absTimeHist);

	assert( ragdoll->absTimeHist[uiInterpIndex] != 0 );
	return ragdoll->absTimeHist[uiInterpIndex];
}

/*
F32 calculateRecursiveMass( Ragdoll* ragdoll )
{
	Ragdoll* childRagdoll = ragdoll->child;
	F32 fTotalMass = 0.0f;
	while (childRagdoll)
	{
		fTotalMass += calculateRecursiveMass(childRagdoll);
		childRagdoll = childRagdoll->next;
	}

	if ( ragdoll->pActor )
	{
		fTotalMass += nwGetActorMass(ragdoll->pActor);
	}

	ragdoll->fRecursiveMass = fTotalMass;
	return fTotalMass;
}
*/


bool clientHasCaughtUpToRagdollStart( Ragdoll* ragdoll, U32 uiClientTime )
{
	U32 uiFirstValidRagdollTimeStamp = firstValidRagdollTimestamp( ragdoll );

	if ( uiFirstValidRagdollTimeStamp < uiClientTime )
		return true;
	return false;
}








