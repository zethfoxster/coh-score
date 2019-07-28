#include "groupnovodex.h"
#include "group.h"
#include "NwWrapper.h"
#include "anim.h"
#include "tricks.h"
#include "MemoryPool.h"
#include "utils.h"
#include "mathutil.h"
#include "gridcoll.h"
#include "file.h"
#include "strings_opt.h"
#include "gridfind.h"
#include "timing.h"
#include "grouptrack.h"
#include "groupfileload.h"
#include "grouputil.h"
#include "entity.h"
#include "gridcache.h"
#include "NwRagdoll.h"
#include "StashTable.h"
#include "Quat.h"

#ifdef CLIENT 
#include "player.h"
#include "gfxLoadScreens.h"
#endif
#ifdef SERVER 
#include "svr_player.h"
#include "entai.h"
#endif

#if NOVODEX


#ifdef SERVER 
#define MAX_SPORES_PER_GRID_LOOKUP 2048
#elif CLIENT 
#define MAX_SPORES_PER_GRID_LOOKUP 8192
#endif


F32 getActorSporeRadius()
{
#ifdef SERVER 
	return 10.0f;
#elif CLIENT 
	return 75.0f;
#endif
}

//
// Actor Spores, for creating NxActors on the fly
//




typedef struct NxActorSpore
{
	GridIdxList* gridIdxList;
	Vec3 vWorldPos;
	Vec3 vWorldCompQuat;
	GroupDef*	pDef;
	bool bExpanded;
} NxActorSpore;

static Grid* nxActorSporeGrid;

static NxActorSpore **nxActorSporeList;

typedef struct NxGrownSpore
{
	struct NxGrownSpore* pNext;
	NxActorSpore* pSpore;
	void* nxGrownActor;
} NxGrownSpore;


// AN influence sphere corresponds to a nxScene in novodex
typedef struct NxInfluenceSphere
{
	Entity* ent;
	NxGrownSpore* pActiveSporeList;
} NxInfluenceSphere;

static NxInfluenceSphere influenceSphere[MAX_NX_SCENES];




// SHAPE CREATION FUNCTIONS
static void createGroupDefNxShape(GroupDef* def, char* nxFilename, int iCtriCount)
{
	Vec3* verts;
	int* tris;
	Model* model = def->model;
	U32 uiTotalAllocSize = 0;
	bool bAllocOnTheStack = true;

	assert(def->nxShape == NULL && model);

	// unpack verts and tris, create the nxstream
	if (!model->gld->geo_data)
	{
		geoLoadData(model->gld);
	}

	assert(model->gld->geo_data);
	uiTotalAllocSize = model->vert_count * sizeof(Vec3) + model->tri_count * 3 * sizeof(int) * 2;

	// don't allocate more than 100k on the stack
	if ( uiTotalAllocSize > 131072)
	{
		bAllocOnTheStack = false;
	}

	if ( bAllocOnTheStack )
	{
		verts = _alloca(model->vert_count * sizeof(Vec3));
		tris = _alloca(model->tri_count * 3 * sizeof(int));
	}
	else
	{
		verts = malloc(model->vert_count * sizeof(Vec3));
		tris = malloc(model->tri_count * 3 * sizeof(int));
	}

	modelGetTris(tris, model);
	modelGetVerts(verts, model);

	{
		int* nxCtris;
		bool nxCtriMallocd = false;
		int nxCtriCount = 0;

		if ( model->ctriflags_setonall & COLL_NORMALTRI)
		{
			assert(iCtriCount == model->tri_count);
			// Just use the all the tris
			nxCtris = tris;
			nxCtriCount = model->tri_count;
		}
		else
		{
			int iCopiedCtriCount = 0;
			int i;
			// Only some of the ctris are necessary...
			assert(iCtriCount < model->tri_count && iCtriCount > 0);
			if ( bAllocOnTheStack )
			{
				nxCtris = _alloca(iCtriCount * 3 * sizeof(int));
			}
			else
			{
				nxCtris = malloc(iCtriCount * 3 * sizeof(int));
				nxCtriMallocd = true;
			}
			nxCtriCount = iCtriCount;
			// Now loop through and copy only the ctri tris...
			for(i=0;i<model->tri_count;i++)
			{
				if ( model->ctris[i].flags & COLL_NORMALTRI )
				{
					memcpy( nxCtris + 3*iCopiedCtriCount, &tris[i*3], sizeof(int)*3);
					iCopiedCtriCount++;
				}
			}
			assert(iCopiedCtriCount == iCtriCount);
		}

		// Now cook the stream
		def->nxShape = nwCookAndLoadMeshShape(def->model->name, verts, model->vert_count, sizeof(verts[0]), nxCtris, nxCtriCount, sizeof(nxCtris[0])*3 );
		if ( nxCtriMallocd )
			free( nxCtris );
	}

	assert(def->nxShape);

	if ( !bAllocOnTheStack )
	{
		free( tris );
		free( verts );
	}
}



static int createAllNxShapesForGroupDef(GroupDef* pDef)
{
	int iTotalShapes = 0;
	//if ( pDef->model && pDef->model->name && !strnicmp(pDef->model->name, "shapercave", 10) )
	//	return -1;

	if ( pDef->nxShape )
		return 0; // no need to repeat


	if ( pDef->model && pDef->model->ctriflags_setonsome & COLL_NORMALTRI)
	{
		int i;
		int iCtriCount = 0;
		if ( pDef->model->tri_count > 0)
			modelCreateCtris(pDef->model);
		for(i=0;i<pDef->model->tri_count;i++)
		{
			if ( pDef->model->ctris[i].flags & COLL_NORMALTRI)
				iCtriCount++;
		}

		if (iCtriCount > 0)
		{
			// Has ctris, make a shape!
			createGroupDefNxShape(pDef, pDef->model->name, iCtriCount);

#if CLIENT
			loadUpdate("Novodex shapes", 50 * iCtriCount);
#endif

			iTotalShapes++;
		}
	}


	// now try children
	{
		int iChild;
		for (iChild=0; iChild<pDef->count; ++iChild)
		{
			int iNewShapes = createAllNxShapesForGroupDef(pDef->entries[iChild].def);

			if ( iNewShapes == -1 )
				return -1;
			iTotalShapes += iNewShapes;
		}
	}


	return iTotalShapes;
}








int createNxWorldShapes()
{
	int i;
	int iTotalShapes = 0;

	loadstart_printf("loading novodex shapes...");


	// Create all of the shapes
	for(i = 0; i < group_info.ref_count; i++)
	{
		if(group_info.refs[i]->def)
		{
			int iResults = createAllNxShapesForGroupDef(group_info.refs[i]->def);

			if ( iResults == -1 )
			{
				iTotalShapes = iResults;
				break;
			}
			else
			{
				iTotalShapes += iResults;
			}
		}
	}

	// Free the stream buffer
	nwFreeStreamBuffer();

	// Now, go through all of the trackers

	if ( iTotalShapes == -1 )
	{
		loadend_printf("failed: invalid shape found." );
		return 0;
	}
	else
		loadend_printf("done: %d shapes created.", iTotalShapes);

	return iTotalShapes;
}






// SHAPE DESTRUCTION FUNCTIONS
static int iFreeCount;
static void destroyAllNxShapesForGroupDef(GroupDef* pDef )
{
	int iChild;
	if ( pDef->nxShape )
		iFreeCount++;
	groupDefFreeNovodex(pDef);

	// Instead, try recursively w/ the children
	for (iChild=0; iChild<pDef->count; ++iChild)
	{
		destroyAllNxShapesForGroupDef(pDef->entries[iChild].def);
	}
}


static void destroyShapesFromTopTrackers(DefTracker* pTracker)
{
	GroupDef* pDef = pTracker->def;

	if ( !pDef )
		return;

	destroyAllNxShapesForGroupDef( pDef );

}






void destroyNxWorldShapes( )
{
	int i;
	iFreeCount = 0;

	loadstart_printf("freeing all novodex shapes...");

	// Create all of the shapes
	for(i = 0; i < group_info.ref_count; i++)
	{
		if(group_info.refs[i]->def)
		{
			destroyShapesFromTopTrackers(group_info.refs[i]);
		}
	}

	loadend_printf("done: freed %d shapes.", iFreeCount);
}









// ACTOR SPORE CREATION FUNCTIONS
MemoryPool actorSporeMp = 0;
MemoryPool grownSporeMp = 0;
#define ACTOR_SPORE_MEMPOOL_SIZE 3000
#define GROWN_SPORE_MEMPOOL_SIZE 1000

static void initActorSporeMemPool()
{
	if( actorSporeMp )
		destroyMemoryPoolGfxNode(actorSporeMp);
	actorSporeMp = createMemoryPoolNamed("NxActorSpore", __FILE__, __LINE__);
	mpSetMode(actorSporeMp, ZERO_MEMORY_BIT);
	initMemoryPool( actorSporeMp, sizeof(NxActorSpore), ACTOR_SPORE_MEMPOOL_SIZE );
}

static void initGrownSporeMemPool()
{
	if( grownSporeMp )
		destroyMemoryPoolGfxNode(grownSporeMp);
	grownSporeMp = createMemoryPoolNamed("NxGrownSpore", __FILE__, __LINE__);
	mpSetMode(grownSporeMp, ZERO_MEMORY_BIT);
	initMemoryPool( grownSporeMp, sizeof(NxGrownSpore), GROWN_SPORE_MEMPOOL_SIZE );
}


static int createGroupDefActorSpore(GroupDef* def, Mat4 mXform)
{
	NxActorSpore* pActorSpore;
	Quat qTemp;
	Vec3 vMin, vMax;

	if (def->no_coll)
		return 0;

	if ( def->nxShape )
	{
		nx_state.actorSporeCount++;
	}


	PERFINFO_AUTO_START("alloc actorspores", 1);


	if ( !actorSporeMp )
		initActorSporeMemPool();

	pActorSpore = mpAlloc(actorSporeMp);

	PERFINFO_AUTO_STOP_START("calc quat", 1);

	copyVec3(mXform[3], pActorSpore->vWorldPos);
	mat3ToQuat(mXform, qTemp);
	quatForceWPositive(qTemp);
	copyVec3(qTemp, pActorSpore->vWorldCompQuat);

	pActorSpore->pDef = def;

	PERFINFO_AUTO_STOP_START("add grid", 1);


	// Add the spore the grid and the list
	if ( !nxActorSporeGrid )
	{
		nxActorSporeGrid = createGrid(1024);
	}

	groupGetBoundsTransformed(def->min, def->max, mXform, vMin, vMax);

	gridAdd(nxActorSporeGrid, pActorSpore, vMin, vMax, 0, &pActorSpore->gridIdxList);

	PERFINFO_AUTO_STOP_START("add to earray", 1);

	// Keep a list for cleanup
	if ( !nxActorSporeList )
		eaCreate(&nxActorSporeList);

	eaPush(&nxActorSporeList, pActorSpore);

	PERFINFO_AUTO_STOP();


	return 0;
}


void initActorSporeGrid()
{
	loadstart_printf("initActorSporeGrid...");

	groupProcessDef(&group_info, createGroupDefActorSpore);

	loadend_printf("done.");
}

static void expandActorSpore( NxActorSpore* pActorSpore )
{
	Mat4 mWorldXForm;
	Quat qTemp;
	GroupEnt	*ent;
	int i;

	copyVec3(pActorSpore->vWorldCompQuat, qTemp);
	quatCalcWFromXYZ(qTemp);
	quatToMat(qTemp, mWorldXForm);
	copyVec3(pActorSpore->vWorldPos, mWorldXForm[3]);


	// create and add that actor
	for(ent = pActorSpore->pDef->entries,i=0;i<pActorSpore->pDef->count;i++,ent++)
	{
		Mat4 mat;
		mulMat4(mWorldXForm, ent->mat, mat);

		createGroupDefActorSpore(ent->def, mat );
	}
	pActorSpore->bExpanded = true;
}



// ACTOR SPORE DESTRUCTION FUNCTIONS
void deinitActorSporeGrid()
{
	NxActorSpore* pActorSpore;
	int iCount = 0;
	loadstart_printf("deiniting actor spore grid...");
	nx_state.actorSporeCount = 0;

	if ( nxActorSporeGrid )
	{
		gridFreeAll(nxActorSporeGrid);
		destroyGrid(nxActorSporeGrid);
		nxActorSporeGrid = NULL;
	}

	pActorSpore = eaPop(&nxActorSporeList);

	while (pActorSpore )
	{
		iCount++;
		mpFree(actorSporeMp, pActorSpore);
		pActorSpore = eaPop(&nxActorSporeList);
	}

	eaDestroy(&nxActorSporeList);
	nxActorSporeList = NULL;

	loadend_printf("done (%d).", iCount);
}


// GROWN SPORE EXPANSION AND CONTRACTION
static void addGrownSporeActor(NxGrownSpore* pGrownSpore, int iSceneNum)
{
	NxActorSpore* pActorSpore = pGrownSpore->pSpore;
	Mat4 mWorldXForm;
	Quat qTemp;
	copyVec3(pActorSpore->vWorldCompQuat, qTemp);
	quatCalcWFromXYZ(qTemp);
	quatToMat(qTemp, mWorldXForm);
	copyVec3(pActorSpore->vWorldPos, mWorldXForm[3]);

	pGrownSpore->pSpore->pDef->nxActorRefCount++;
	pGrownSpore->nxGrownActor = nwCreateMeshActor(mWorldXForm, pActorSpore->pDef, pActorSpore->pDef->nxShape, 0, iSceneNum);

}

static void removeGrownSporeActor( NxGrownSpore* pGrownSpore, int iSceneNum)
{
	if (!pGrownSpore->nxGrownActor)
		return;

	assert( pGrownSpore->pSpore );

	assert(pGrownSpore->pSpore->pDef->nxActorRefCount > 0);
    pGrownSpore->pSpore->pDef->nxActorRefCount--;

	nwDeleteActor(pGrownSpore->nxGrownActor, iSceneNum);
	pGrownSpore->nxGrownActor = NULL;
}





// INFLUENCE SPHERE MANAGEMENT
void addInfluenceSphere(int iSceneNum, Entity* ent)
{
	assert( influenceSphere[iSceneNum].ent == NULL);
#ifdef SERVER 
	assert(ent);
#endif

	influenceSphere[iSceneNum].ent = ent;
	influenceSphere[iSceneNum].pActiveSporeList = NULL;
}

void removeInfluenceSphere(int iSceneNum)
{
	NxGrownSpore* pGrownSporeToRemove = influenceSphere[iSceneNum].pActiveSporeList;

	while ( pGrownSporeToRemove )
	{
		NxGrownSpore* pNext = pGrownSporeToRemove->pNext;
		removeGrownSporeActor(pGrownSporeToRemove, iSceneNum);
		mpFree(grownSporeMp, pGrownSporeToRemove);
		pGrownSporeToRemove = pNext;
	}

	influenceSphere[iSceneNum].pActiveSporeList = NULL;
	influenceSphere[iSceneNum].ent = NULL;
}
void removeAllInfluenceSpheres()
{
	int i=0;

	for (i=0; i<MAX_NX_SCENES; ++i)
	{
		if ( influenceSphere[i].ent )
		{
			removeInfluenceSphere(i);
		}
	}
}


static NxActorSpore* pFoundActorSpores[MAX_SPORES_PER_GRID_LOOKUP];
static StashTable sporeStash;

// PER-TICK UPDATE FUNCTIONS
void updateNxActors( int iSceneNum )
{
	// for now, just around the main player
	Vec3 vSporeRadiusCenter;
	int iFoundCount, i;
	NxInfluenceSphere* iSphere = &influenceSphere[iSceneNum];

#ifdef SERVER 
	assert( iSphere->ent != NULL );
#elif CLIENT 
	iSphere->ent = playerPtr();
	if ( !iSphere->ent )
		return;
#endif

	if (!nxActorSporeGrid)
		return;

	if (!sporeStash)
		sporeStash = stashTableCreateAddress(MAX_SPORES_PER_GRID_LOOKUP);

	PERFINFO_AUTO_START("findObjectsAndExpandSporeTree", 1);
	{
		bool bExpandedSomething = false;
		copyVec3(ENTPOS(iSphere->ent), vSporeRadiusCenter);

		// Now find all the should be grown spores
		do
		{
			bExpandedSomething = false;
			PERFINFO_AUTO_START("gridFind",1);
			iFoundCount = gridFindObjectsInSphere(nxActorSporeGrid, pFoundActorSpores, MAX_SPORES_PER_GRID_LOOKUP, vSporeRadiusCenter, getActorSporeRadius());
			stashTableClear(sporeStash); // if we've changed the grid around (in the loop), we must re-add everything


			PERFINFO_AUTO_STOP_START("look for expandables",1);
			// Loop throught he found objects, and see if any need to be expanded
			for (i=0; i<iFoundCount; ++i)
			{
				NxActorSpore* pFoundActorSpore = pFoundActorSpores[i];

				stashAddressAddPointer(sporeStash, pFoundActorSpore, pFoundActorSpore, false);
				if ( !pFoundActorSpore->bExpanded )
				{
					PERFINFO_AUTO_START("expand",1);
					bExpandedSomething = true;

					// Expand it
					expandActorSpore(pFoundActorSpore);

					// Now, if it has a shape we must keep it in the tree, otherwise toss it
					if ( !pFoundActorSpore->pDef->nxShape )
					{
						gridClearAndFreeIdxList(nxActorSporeGrid, &pFoundActorSpore->gridIdxList);
						eaRemoveFast(&nxActorSporeList, eaFind(&nxActorSporeList, pFoundActorSpore) );
						mpFree(actorSporeMp, pFoundActorSpore);
					}
					PERFINFO_AUTO_STOP_CHECKED("expand");
				}
			}
			PERFINFO_AUTO_STOP_CHECKED("look for expandables");
		} while ( bExpandedSomething );
	}
	PERFINFO_AUTO_STOP_START("removeOldActors", 1);
	{
		// First, loop through the currently "grown" (meaning active in novodex)
		// spores, and remove those that weren't found
		// Then we will go through and add the unmarked found ones
		NxGrownSpore* pGrownSpore = iSphere->pActiveSporeList;
		NxGrownSpore* pPrevGrownSpore = NULL;
		while (pGrownSpore)
		{
			// First, see if it's in the found list, and if it is remove it
			bool bStillActive = stashAddressRemovePointer(sporeStash, pGrownSpore->pSpore, NULL);

			if ( bStillActive )
			{
				// nothing to do, we're synced up... advance
				pPrevGrownSpore = pGrownSpore;
				pGrownSpore = pGrownSpore->pNext;
			}
			else
			{
				// If we got here, we're not in the found list, 
				// yet we are grown: remove us
				removeGrownSporeActor(pGrownSpore, iSceneNum);

				// Remove from LL
				if ( pPrevGrownSpore )
				{
					pPrevGrownSpore->pNext = pGrownSpore->pNext;
				}
				else
				{
					iSphere->pActiveSporeList = pGrownSpore->pNext;
				}

				// free us
				{
					NxGrownSpore* pNextSpore = pGrownSpore->pNext;
					mpFree(grownSporeMp, pGrownSpore);
					pGrownSpore = pNextSpore;
				}
			}
		}
	}
	PERFINFO_AUTO_STOP_START("addNewActors", 1);
	// Ok, we've processed the grown list.. anything left in the found list
	// is new, and must be created in novodex
	{
		StashElement elem;
		StashTableIterator iter;
		stashGetIterator(sporeStash, &iter);
		while (stashGetNextElement(&iter, &elem))
		{
			NxActorSpore* pFoundActorSpore = (NxActorSpore*)stashElementGetPointer(elem);
			NxGrownSpore* pNewGrownSpore;
			// Make grown spore
			if (!grownSporeMp)
				initGrownSporeMemPool();
			pNewGrownSpore = mpAlloc(grownSporeMp);
			pNewGrownSpore->pSpore = pFoundActorSpore;

			// Create the novodex presence
			addGrownSporeActor(pNewGrownSpore, iSceneNum);

			// Add it to LL
			pNewGrownSpore->pNext = iSphere->pActiveSporeList;
			iSphere->pActiveSporeList = pNewGrownSpore;
		}
	}
	PERFINFO_AUTO_STOP();
}

#endif // NOVODEX

