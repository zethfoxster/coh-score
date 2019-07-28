
#include "fxdebris.h"
#include "fxgeo.h"
#include "gridfind.h"
#include "timing.h"
#include "groupnovodex.h"
#include "entity.h"
#include "light.h"
#include "player.h"
#include "fx.h"
#include "fxlists.h"

#if NOVODEX

FxDebrisManager fx_debris_manager;

#define MAX_FADE_OUT_DEBRIS_COUNT 1000

static void fxCleanUpGfxNode(GfxNode* pGfxNode, int iGfxNodeId)
{
	// remove gfx node
	if ( pGfxNode && pGfxNode->seqGfxData )
		free(pGfxNode->seqGfxData);
	if( gfxTreeNodeIsValid(pGfxNode, iGfxNodeId) )
	{
		gfxTreeDelete(pGfxNode);
	}
}

static void fxFinishFadingOutFadeOutDebris(FxFadeOutDebris* pFadeOut)
{
	fxCleanUpGfxNode(pFadeOut->pGfxNode, pFadeOut->iGfxNodeId);

	mpFree(fx_debris_manager.mpFadeOutDebris, pFadeOut);
}

static void fxMoveGfxNodeIntoFadeOutList(GfxNode* pGfxNode, int iGfxNodeId)
{
	FxFadeOutDebris* pFade = mpAlloc(fx_debris_manager.mpFadeOutDebris);
	pFade->pGfxNode = pGfxNode;
	pFade->iGfxNodeId = iGfxNodeId;

	// insert into list
	pFade->pNext = fx_debris_manager.pFadeOutList;
	fx_debris_manager.pFadeOutList = pFade;
}

static bool fxUpdateFadeOutDebris(FxFadeOutDebris* pFadeOut)
{
	if ( pFadeOut->pGfxNode->alpha > 2)
		pFadeOut->pGfxNode->alpha -= 2;
	else
		return false;

	return true;
}

static void fxUpdateFadeOutDebrisManager()
{
	FxFadeOutDebris* pDebris = fx_debris_manager.pFadeOutList;
	FxFadeOutDebris* pPrevDebris = NULL;
	U32 uiFadeOutCount = 0;
	while (pDebris)
	{
		FxFadeOutDebris* pNext = pDebris->pNext;
		bool bStillValid;
		if ( uiFadeOutCount > MAX_FADE_OUT_DEBRIS_COUNT)
			bStillValid = false;
		else
			bStillValid = fxUpdateFadeOutDebris(pDebris);
		if ( !bStillValid )
		{
			// Remove it
			if ( pPrevDebris )
				pPrevDebris->pNext = pDebris->pNext;
			else
				fx_debris_manager.pFadeOutList = pDebris->pNext;

			fxFinishFadingOutFadeOutDebris(pDebris);
		}
		else
			pPrevDebris = pDebris;
		pDebris = pNext;
		uiFadeOutCount++;
	}
	//printf("Fade out count = %d\n", uiFadeOutCount);
}

static void fxRemoveAllFadeOutDebris()
{
	FxFadeOutDebris* pDebris = fx_debris_manager.pFadeOutList;
	while (pDebris)
	{
		FxFadeOutDebris* pNext = pDebris->pNext;
		fxFinishFadingOutFadeOutDebris(pDebris);
		pDebris = pNext;
	}
	fx_debris_manager.pFadeOutList = NULL;
}


static void fxRemoveDebrisFromGrid(FxDebris* pDebris)
{
	if ( pDebris->pGridIdxList )
		gridClearAndFreeIdxList(fx_debris_manager.gridDebris, &pDebris->pGridIdxList);
}

static void fxInsertDebrisIntoGrid(FxDebris* pDebris)
{
	Vec3 vPos;
	PERFINFO_AUTO_START("insert into grid", 1);
	copyVec3(pDebris->pGfxNode->mat[3], vPos);

	fxRemoveDebrisFromGrid(pDebris);

	// I have to shift up 2 bits because it turns out the
	// grid code assumes that what is passed in is a 4-byte aligned pointer... aw yeah
	gridAdd(fx_debris_manager.gridDebris, (void*)(pDebris->nxEmissary<<2), vPos, vPos, 0, &pDebris->pGridIdxList);
	PERFINFO_AUTO_STOP();
}

static void fxKillDebris(FxDebris* pDebris, bool bFast)
{
	// Remove novodex presence
	if ( pDebris->nxEmissary && nwEnabled() )
	{
		nwDeleteActor(nwGetActorFromEmissaryGuid(pDebris->nxEmissary), NX_CLIENT_SCENE);
		pDebris->nxEmissary = 0;
		fx_debris_manager.uiDebrisCount--;
	}

	fxRemoveDebrisFromGrid(pDebris);

	if ( bFast )
	{
		fxCleanUpGfxNode(pDebris->pGfxNode, pDebris->iGfxNodeId);
		pDebris->pGfxNode = NULL;
	}
	else if ( pDebris->pGfxNode )
		fxMoveGfxNodeIntoFadeOutList(pDebris->pGfxNode, pDebris->iGfxNodeId);
	

}

static void fxRemoveDebris(FxDebris* pDebris, bool bFast)
{
	fxKillDebris(pDebris, bFast);

	// Free it
	mpFree(fx_debris_manager.mpDebris, pDebris);
}

static void fxUpdateDebris(FxDebris* pDebris)
{
	Vec3 vDiff;
	const bool bDoLengthTesting = false;

	PERFINFO_AUTO_START("update matrix", 1);
	if ( pDebris->nxEmissary )
	{
		NwEmissaryData* pData = nwGetNwEmissaryDataFromGuid(pDebris->nxEmissary);
		if ( pData )
		{
			void* pActor = pData->pActor;
			if ( pDebris->iFxID && pData->bCurrentlyJointed && !hdlGetPtrFromHandle(pDebris->iFxID) )
			{
				// Our fx owner is gone, yet we are jointed. destroy us
				fxKillDebris(pDebris, true);
				return;
			}
			if ( pActor )
			{
				nwGetActorMat4(pActor, (float*)pDebris->pGfxNode->mat);
				scaleMat3Vec3(pDebris->pGfxNode->mat, pDebris->vScale);
			}
		}
	}


	{
		int t, i, iPos[3];
		bool bMoveGrid = false;
		PERFINFO_AUTO_STOP_START("checkgrid", 1);
		qtruncVec3(pDebris->pGfxNode->mat[3], iPos);
		for(i=0;i<3;i++)
		{
			t = iPos[i] >> 3;
			if (pDebris->iLastPos[i] != t)
				bMoveGrid = true;
			pDebris->iLastPos[i] = t;
		}

		if ( bMoveGrid )
		{
			PERFINFO_AUTO_STOP_START("check bounds", 1);
			if ( playerPtr() )
			{
				F32 fLengthToCheck = getActorSporeRadius() * 1.5f;
				subVec3( pDebris->pGfxNode->mat[3], ENTPOS(playerPtr()), vDiff );
				if ( lengthVec3Squared(vDiff) > fLengthToCheck*fLengthToCheck )
				{
					fxKillDebris(pDebris, true);
					PERFINFO_AUTO_STOP();
					return;
				}
			}
			/*
			// Could be performance heavy, so let's not do this
			PERFINFO_AUTO_STOP_START("update lighting", 1);
			if ( pDebris->pGfxNode->seqGfxData )
				lightEnt( &pDebris->pGfxNode->seqGfxData->light, pDebris->pGfxNode->mat[3], 0.0, 1.0 );
				*/
			PERFINFO_AUTO_STOP_START("reinsert into grid", 1);

			fxInsertDebrisIntoGrid(pDebris);
		}
	}

	PERFINFO_AUTO_STOP();
}




void fxInitDebrisManager()
{
	// make sure everything is cleared out
	fxDeinitDebrisManager();

	if ( fx_debris_manager.uiMaxDebris == 0 )
		return;

	// debris max not inited, set it to default
	/* - now a gfx option
	if ( nx_state.hardware )
		fx_debris_manager.uiMaxDebris = 750;
	else
		fx_debris_manager.uiMaxDebris = 200;
		*/

	fx_debris_manager.uiDebrisCount = 0;

	fx_debris_manager.debrisQueue = createQueue();
	qSetMaxSizeLimit( fx_debris_manager.debrisQueue, fx_debris_manager.uiMaxDebris );
	initQueue( fx_debris_manager.debrisQueue, fx_debris_manager.uiMaxDebris );

	fx_debris_manager.mpDebris = createMemoryPoolNamed("FxDebris", __FILE__, __LINE__);
	initMemoryPool( fx_debris_manager.mpDebris, sizeof(FxDebris), fx_debris_manager.uiMaxDebris + 1); // the plus one is for when we are adding to an already full queue, we need the storage temporarily

	fx_debris_manager.mpFadeOutDebris = createMemoryPoolNamed("FxFadeOutDebris", __FILE__, __LINE__);
	initMemoryPool( fx_debris_manager.mpFadeOutDebris, sizeof(FxFadeOutDebris), fx_debris_manager.uiMaxDebris ); 

	fx_debris_manager.gridDebris = createGrid(fx_debris_manager.uiMaxDebris);
}

void fxDeinitDebrisManager()
{

	if ( fx_debris_manager.debrisQueue )
	{
		QueueIterator iter;
		FxDebris* pDebris;

		qGetIterator(fx_debris_manager.debrisQueue, &iter);
		while (qGetNextElement(&iter, &pDebris))
			fxRemoveDebris(pDebris, true);

		destroyQueue(fx_debris_manager.debrisQueue);
	}

	fx_debris_manager.debrisQueue = NULL;

	if ( fx_debris_manager.gridDebris )
		destroyGrid(fx_debris_manager.gridDebris);
	fx_debris_manager.gridDebris = NULL;

	if ( fx_debris_manager.mpDebris )
		destroyMemoryPool(fx_debris_manager.mpDebris);
	fx_debris_manager.mpDebris = NULL;

	fxRemoveAllFadeOutDebris();
	if ( fx_debris_manager.mpFadeOutDebris )
		destroyMemoryPool(fx_debris_manager.mpFadeOutDebris);
	fx_debris_manager.mpFadeOutDebris = NULL;

}

bool fxAddDebris(FxGeo* pGeo, Vec3 vScale, int iFxId)
{
	FxDebris* pNewDebris;

	// not yet init'd
	if ( !fx_debris_manager.uiMaxDebris )
		return false;

	// allocate
	pNewDebris = mpAlloc(fx_debris_manager.mpDebris);

	// If you hit this assert, you probably have failed to init the debris manager.
	assert( fx_debris_manager.debrisQueue );

	// copy the novodex emissary over
	pNewDebris->nxEmissary = pGeo->nxEmissary;

	// copy the gfx node
	pNewDebris->pGfxNode = pGeo->gfx_node;
	pNewDebris->pGfxNode->rgbs = NULL;
	pNewDebris->pGfxNode->alpha = 255;
	pNewDebris->iGfxNodeId = pGeo->gfx_node_id;
	pNewDebris->pGridIdxList = NULL;
	pNewDebris->iFxID = iFxId;
	//pNewDebris->bhvr = pGeo->bhvr;
	if ( pNewDebris->pGfxNode->seqGfxData )
	{
		pNewDebris->pGfxNode->seqGfxData = calloc(1, sizeof(SeqGfxData));
		lightEnt( &pNewDebris->pGfxNode->seqGfxData->light, pNewDebris->pGfxNode->mat[3], 0.0, 1.0 );
	//	pNewDebris->pGfxNode->seqGfxData = NULL;
	}

	copyVec3(vScale, pNewDebris->vScale);
//	nwSetActorSleepVelocities(pGeo->nxEmissary->pActor, 5.0f, 5.0f);

	// Free up a slot if necessary
	if ( qIsReallyFull(fx_debris_manager.debrisQueue) )
		fxRemoveDebris(qDequeue(fx_debris_manager.debrisQueue), false);

	// add it to queue
	qEnqueue(fx_debris_manager.debrisQueue, pNewDebris);

	// add it to the grid
	fxInsertDebrisIntoGrid(pNewDebris);
	fx_debris_manager.uiDebrisCount++;

	return true;
}

void fxUpdateDebrisManager()
{
	QueueIterator iter;
	FxDebris* pDebris;

	if ( !fx_debris_manager.debrisQueue )
		return;
	qGetIterator(fx_debris_manager.debrisQueue, &iter);

	while (qGetNextElement(&iter, &pDebris))
	{
		void* pActor = nwGetActorFromEmissaryGuid(pDebris->nxEmissary);
		if ( pActor && !nwIsSleeping(pActor) )
			fxUpdateDebris(pDebris);
	}

	fxUpdateFadeOutDebrisManager();

}

void fxSetMaxDebrisCount()
{
	U32 uiMaxDebrisCount;
	switch (nx_state.physicsQuality)
	{
	xdefault:
	xcase ePhysicsQuality_None:
		uiMaxDebrisCount = 0;
	xcase ePhysicsQuality_Low:
		uiMaxDebrisCount = 150;
	xcase ePhysicsQuality_Medium:
		uiMaxDebrisCount = 350;
	xcase ePhysicsQuality_High:
		uiMaxDebrisCount = 500;
	xcase ePhysicsQuality_VeryHigh:
		uiMaxDebrisCount = 750;
	}

	if ( fx_debris_manager.uiMaxDebris == uiMaxDebrisCount )
		return;

	fx_debris_manager.uiMaxDebris = uiMaxDebrisCount;
	// reset everything for the new maxdebriscount
	fxInitDebrisManager();
}


void fxApplyForce(Vec3 vEpicenter, F32 fRadius, F32 fPower, F32 fPowerJitter, F32 fCentripetal, eNxForceType eForce, Mat3Ptr pmXform)
{
	const U32 cuiMaxResults = 256;
	NwEmissaryDataGuid* pEmissaries;
	U32 uiResultIndex, uiNumResults;

	if ( fx_debris_manager.uiMaxDebris == 0 )
		return;
	PERFINFO_AUTO_START("Stack allocate", 1);
	pEmissaries = _alloca(sizeof(NwEmissaryDataGuid) * cuiMaxResults);

	PERFINFO_AUTO_STOP_START("Gather shapes", 1);
	uiNumResults = gridFindObjectsInSphere(fx_debris_manager.gridDebris, (void**)pEmissaries, cuiMaxResults, vEpicenter, fRadius);

	PERFINFO_AUTO_STOP_START("Apply force", 1);
	for (uiResultIndex=0; uiResultIndex<uiNumResults;++uiResultIndex)
	{
		// I have to shift down 2 bits because it turns out the
		// grid code assumes that what is passed in is a 4-byte aligned pointer... aw yeah
		NwEmissaryData* pData = nwGetNwEmissaryDataFromGuid(pEmissaries[uiResultIndex]>>2);
		if ( pData )
			nwApplyForceToEmissary(pData, vEpicenter, fRadius, fPower, fPowerJitter, fCentripetal, eForce, pmXform, NX_CLIENT_SCENE);
	}
	PERFINFO_AUTO_STOP_CHECKED("Apply force");

	fxActivateForceFxNodes(vEpicenter, fRadius, fPower);
}

U32  fxGetDebrisObjectCount()
{
	return fx_debris_manager.uiDebrisCount;
}

U32  fxGetDebrisMaxObjectCount()
{
	return fx_debris_manager.uiMaxDebris;
}
#endif