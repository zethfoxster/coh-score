#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "mathutil.h"
#include "error.h"
#include "file.h"
#include "cmdcommon.h"
#include "assert.h"
#include "gfxtree.h"
#include "camera.h" 
#include "font.h"
#include "gfxtree.h"
#include "model.h"
#include "tricks.h"
#include "anim.h"
#include "MemoryPool.h"

#if CLOTH_HACK
#include "clothnode.h"
#endif

#if CLIENT
	#include "splat.h"
	#include "cmdgame.h" 
	#include "fxinfo.h"
	#include "sun.h"
	#include "groupMiniTrackers.h"
#endif

#define GFXDEBUG 0

GfxNode	*gfx_tree_root,*gfx_tree_root_tail;
MemoryPool gfxnode_mp = 0;

#ifdef CLIENT
MemoryPool sky_gfxnode_mp = 0;
GfxNode	*sky_gfx_tree_root,*sky_gfx_tree_root_tail;
int sky_gfx_tree_inited=0;
#endif

static int gfx_tree_nodes_used = 0; //debug
static int strange_optimized_buid_thing = 0;

//###########################################################################
//1. Debug Stuff#############################################################

extern char * getBoneNameFromNumber( int num );  //(just here for debug)


//#######################################################################################
///2 Utilities that query the gfxtree but don't change it...


//Given A Gfx Node, return it's world space position.
void gfxTreeFindWorldSpaceMat(Mat4 result, GfxNode * node)
{
	Mat4 old_result;

	if(!node)
	{
		copyMat4(unitmat, result);
		return;
	}
	copyMat4(node->mat, result); 	
	while(node->parent && node->parent != ( GfxNode * )0xfafafafa )	//latter part is lf hack, approved by Mr. Woomer.  DEBUG!!
	{
		copyMat4(result, old_result); 
		mulMat4(node->parent->mat, old_result, result);
		node = node->parent;
	}
}

//Given a name, find the gfxnode with the named modelect 
//TO DO can this be made faster by using fewer string compares?
//TO DO add trick file elimination to this
GfxNode *gfxTreeFindRecur(char *name,GfxNode *node) 
{
GfxNode	*found;

	for(;node;node = node->next)
	{
		if (node->model && strstri(node->model->name,name)==0)
			return node;
		if (node->child)
		{
			found = gfxTreeFindRecur(name,node->child);
			if (found)
				return found;
		}
	}
	return 0;
}


/*Given a root bone return the node. Pass in gfx_root child, because
gfx_root doesn't have the seqHandle...*/
GfxNode * gfxTreeFindBoneInAnimation(BoneId bone, GfxNode *node, int seqHandle, int root)
{
GfxNode	*found;

	for(;node;node = node->next)
	{
		if(node->seqHandle == seqHandle)
		{
			if (node->anim_id == bone)
				return node;
			if (node->child)
			{
				found = gfxTreeFindBoneInAnimation(bone,node->child, seqHandle, 0);
				if (found)
					return found;
			}
		}
		//if(root) taken out as long as gfx_root->child is passed in cuz gfx_root has no SeqGfxData
		//	break;
	}
	return 0;
}

MP_DEFINE(TrickNode);

TrickNode * gfxTreeAssignTrick( GfxNode * node )
{
	assert( node );
	if( !node->tricks ) 
	{
		MP_CREATE(TrickNode, 100);
		node->tricks = MP_ALLOC(TrickNode);
		memset(node->tricks->trick_rgba, 255, sizeof(node->tricks->trick_rgba)); //I think this is unnecessary, but I don't want to be randomly changing stuff right before the milestone
		node->trick_from_where = TRICKFROM_UNKNOWN;
	}
	return node->tricks;
}

/* GfxNodes are always initialized to the tricks of the modelect attached to them;
the modelect's tricks also has a field of tricks to be added to the gfxnodes->flags.
(The node's tricks once initted are then modifiable and are used for several things...) 
Im not happy with this function here in gfxtree, but don't really know were else to put it.
*/
void gfxTreeInitGfxNodeWithObjectsTricks(GfxNode * node, TrickNode *trick)
{
	if (trick)
	{
		*(gfxTreeAssignTrick( node )) = *trick;
	}
}

//###############################################################################
//3.  Manage the GfxTree Structure #################################################

#ifdef CLIENT
void gfxTreeInitSkyTree()
{
	if( sky_gfxnode_mp )
		destroyMemoryPoolGfxNode(sky_gfxnode_mp);
	sky_gfxnode_mp	= createMemoryPool();
	initMemoryPool( sky_gfxnode_mp, sizeof(GfxNode), 16 );
	sky_gfx_tree_root = 0;
	sky_gfx_tree_root_tail = 0;
	sky_gfx_tree_inited = 0;
}
#endif

#if 0 // fpe for test
#include "fileutil.h"
#include "model_cache.h"
#include "tex.h"
#include "StashTable.h"

bool gbFoundZeroTangent, gbFoundZeroTangentInThisFile;
int numZeroTangentFiles, numZeroTangentModels, numTotalModels, numTotalTextures, numTotalFiles, numMissingTextures;
static StashTable missing_textures_ht=0;
static FileScanAction preloadSomeGeometryCallback(char* dir, struct _finddata32_t* data)
{
	if(dir[0] == '_') {
		return FSA_NO_EXPLORE_DIRECTORY;
	}

	if(!(data->attrib & _A_SUBDIR))
	{
		const char* name = data->name;
		if(	strEndsWith(name, ".geo") && name[0] != '_' )
		{
			char buffer[MAX_PATH];
			GeoLoadData* gld;
			
			sprintf(buffer, "%s/%s", dir, name);
			//printf("preloading geo file: \"%s\"\n", buffer);
			gld = geoLoad(buffer, LOAD_NOW, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE); // LOAD_HEADER
			if(gld)
			{
#if 0 // test code to find zero tangents
				int i, count = gld->modelheader.model_count;
				gbFoundZeroTangentInThisFile = false;
				numTotalFiles++;
				numTotalModels += count;
				for(i=0; i<count; i++)
				{
					Model* pModel = gld->modelheader.models[i];
					printf("\tmodel \"%s\"\n", pModel->name);

					// now test to see if this model has screwed up tangent space
					gbFoundZeroTangent = false;
					modelSetupVertexObject(pModel, VBOS_USE, BlendMode(BLENDMODE_BUMPMAP_COLORBLEND_DUAL,0)) ;

					if(pModel->vbo) {
						modelFreeVertexObject( pModel->vbo );
						pModel->vbo = NULL;
					}

					if(gbFoundZeroTangent)
					{
						numZeroTangentModels++;
						if(!gbFoundZeroTangentInThisFile)
						{
							gbFoundZeroTangentInThisFile = true;
							numZeroTangentFiles++;
						}
					}
				}
#else // test code to find all used textures
				int i, j;
				numTotalFiles++;
				numTotalModels += gld->modelheader.model_count;
				numTotalTextures += gld->texnames.count;
#if 0
				for (i=0; i<gld->texnames.count; i++) {
					const char * texname = gld->texnames.strings[i];
					if(texFind(texname) == NULL && texFindComposite(texname) == NULL) {
						printf("Texture %s is missing (file %s)!\n", texname, buffer);
						numMissingTextures++;
					}

					if(texFind(texname)) {
						assert(texFindComposite(texname) != NULL);
					}
				}
#else
				// look only at textures used by models
				printf("GEO: %s (%d textures)\n", buffer, gld->texnames.count);
				for (i=0; i<gld->texnames.count; i++)
					printf("  texture %d: %s\n", i+1, gld->texnames.strings[i]);
				for(j=0; j<gld->modelheader.model_count; j++) {
					Model * model = gld->modelheader.models[j];
					printf("\tmodel %d: %s\n", j+1, model->name);
					for(i=0;i<model->tex_count;i++)
					{
						char *texname = gld->texnames.strings[model->tex_idx[i].id];
						assert(model->gld == gld);
						printf("\t\ttexture: %s\n", texname);
						continue;

						if(texFind(texname) == NULL && texFindComposite(texname) == NULL) {
							StashElement element;
							assert(strstri(texname, ".tga") == NULL);
							if (!stashFindElement(missing_textures_ht, texname, &element)) {
								printf("Texture %s is missing from %s : %s\n", texname, buffer, model->name);
								numMissingTextures++;
								stashAddPointer(missing_textures_ht, texname, gld, false);
							}
						}
					}
				}
#endif
#endif
			}
		}
	}
	
	return FSA_EXPLORE_DIRECTORY;
}
#endif


void gfxTreeInitCharacterAndFxTree()
{
	if( gfxnode_mp )
		destroyMemoryPoolGfxNode(gfxnode_mp);
	gfxnode_mp		= createMemoryPool();
	initMemoryPool( gfxnode_mp, sizeof(GfxNode), 2000 );
	gfx_tree_root = 0;
	gfx_tree_root_tail = 0;
	
	modelFreeAllCache(GEO_USED_BY_GFXTREE);
	
#if CLIENT
	cam_info.simpleShadow = 0;
	cam_info.simpleShadowUniqueId = 0;
	skyNotifyGfxTreeDestroyed();
	fxPreloadGeometry();

#if 0
	// fpe for test, load all geo and process to determine which ones need reprocessing
	{
		const char* dirToScan = "player_library";
		missing_textures_ht = stashTableCreateWithStringKeys(100, StashDeepCopyKeys);
		fileScanAllDataDirs(dirToScan, preloadSomeGeometryCallback);
		//printf("Scanned dir \"%s\" for zero tangents:\n\t%d of %d models\n\t%d of %d files\n", dirToScan, numZeroTangentModels, numTotalModels, numZeroTangentFiles, numTotalFiles);
		printf("Scanned dir \"%s\" for textures:\n\t%d models\n\t%d files\n\t%d textures\n\t%d missing textures\n",
			dirToScan, numTotalModels, numTotalFiles, numTotalTextures, numMissingTextures);
	}
#endif
#endif
}

// A. Allocate the gfxtree
void gfxTreeInit()
{
	gfxTreeInitCharacterAndFxTree();
#ifdef CLIENT
	gfxTreeInitSkyTree();
#endif
#if GFXDEBUG
	gfx_tree_nodes_used = 0;
#endif
}


////////////////////////////////////////////////////////////////////////////
//General Tree Management:

//Gets a node from gfx_nodes and initializes it
static GfxNode *gfxTreeNewNode( MemoryPool mp  )
{
	GfxNode *node;
	static int nextGfxNodeId = 1; 

#if GFXDEBUG	
	gfx_tree_nodes_used++;
#endif

	node = mpAlloc(mp); //(assumed zeroed)
	assert(node);

	node->tricks = 0;
	node->alpha = 255;
	//node->scale = 1.f;
	//node->def_id = -1;
	if(nextGfxNodeId == -1) //skip -1 as that means not allocated, and 0 because that just feels wrong
		nextGfxNodeId+=2;
	if (nextGfxNodeId == 0xfafafafa)
		nextGfxNodeId++;
	node->unique_id = nextGfxNodeId++; 
	return node;
}

//Attaches given new node to the gfx_tree
//(In the case of reattachment, the node can have kids and younger brothers.)
static GfxNode * gfxTreeAttachNode(GfxNode * parent, GfxNode * curr, GfxNode ** root, GfxNode ** tail )
{
	GfxNode	*node;
	
	assert(curr && !curr->prev && !curr->parent && curr->unique_id != 0xfafafafa); //node must be oldest brother and have no parent
		
	if (parent)
	{		
		//put this node at the end of the list as the youngest node (it's younger bros are still attached) 
		if( parent->child )
		{
			node = parent->child;
			assert( node != (GfxNode*)0xfafafafa && node->unique_id != 0xfafafafa );

			while(node->next) //why? it's inefficient, why can't youngest bro be parent's child, not eldest... (would need to swithc a few things around in gfxtree for that to work
			{
				assert( node->next != (GfxNode*)0xfafafafa && node->next->unique_id != 0xfafafafa );
				node = node->next;
			}
			assert(node && ! node->next);
			node->next = curr;
			curr->prev = node;
		}
		else
		{
			parent->child = curr;
		}

		//give this node and all it's younger brothers the new parent
		node = curr;
		while(node) 
		{
			node->parent = parent;
			node = node->next; 
		}

	}
	else //if(!parent)
	{
		if (!*root ) //if empty tree
		{
			assert(!*tail);
			*root = curr;
		}
		else
		{
			assert(*tail && !(*tail)->next);
			curr->prev = *tail;
			(*tail)->next = curr;
		}

		node = curr;
		while(node->next)
		{
			assert( node->next != (GfxNode*)0xfafafafa && node->next->unique_id != 0xfafafafa );
			node = node->next;
		}
		*tail = node;
	}
	
	assert(!*tail || !(*tail)->next);
	return curr;
}


#if CLIENT
//extern int splatMemAlloced;
//C. Removing Nodes
Splat * freeSplatNode( Splat * splat )
{
	SAFE_FREE( splat->colors );
	SAFE_FREE( splat->sts );
	SAFE_FREE( splat->sts2 );
	SAFE_FREE( splat->verts );
	SAFE_FREE( splat->tris );

	//splatMemAlloced -= splat->currMaxVerts * sizeof( Vec3 );
	//splatMemAlloced -= splat->currMaxVerts * sizeof( Vec2 );
	//splatMemAlloced -= splat->currMaxVerts * sizeof( Vec2 );
	//splatMemAlloced -= splat->currMaxVerts * 4 * sizeof(U8);
	//splatMemAlloced -= splat->currMaxTris * sizeof( Triangle );

	if( splat->invertedSplat )
	{
		freeSplatNode( splat->invertedSplat );
	}

	destroySplat( splat );

	return 0;
}
#endif

static void removeSuspendedNode(GfxNode * node);

//Scrubs an unneeded node and sticks it back in gfx_nodes
static void gfxTreeFreeNode(GfxNode *node, MemoryPool mp )
{
#if GFXDEBUG
	gfx_tree_nodes_used--;
#endif
	MP_FREE(TrickNode, node->tricks);

	#if CLIENT
		if( node->splat )
		{
			node->splat = freeSplatNode( (Splat*)node->splat );
		}
		gfxNodeFreeMiniTracker(node);
	#endif
	
#if CLOTH_HACK
# if CLIENT
	if (node->clothobj) {
		freeClothNode( node->clothobj );
		node->clothobj = NULL;
	}
# else
	if (node->clothobj)
		assert( 0 == "GfxTree error.  Server should not be creating capes");
# endif
#endif
	// Check and see if this GfxNode was on the suspended list and clear it if so
	removeSuspendedNode(node);

	memset(node,0xfa,sizeof(GfxNode)); // Probably done in the memory pool anyway!
	mpFree( mp, node );
}

//Frees this node, and all it's kids, and all it's older siblings (should only be 
//called on node->child )
static void freeTreeNodes(GfxNode *node, MemoryPool mp)
{
	extern S32 collectPreviousModels;
	void animAddPreviousModel(GfxNode* node);
	
	GfxNode *next;

	for( ; node ; node = next )
	{
		if(collectPreviousModels)
		{
			animAddPreviousModel(node);
		}
		
		next = node->next;
		if (node->child)
			freeTreeNodes(node->child, mp);	
		gfxTreeFreeNode(node, mp);
	}
}


//Unlinks and returns a node and it's children from gfxtree. Nothing else is changed in the branch
static GfxNode * gfxTreeDetatchNode( GfxNode * node, GfxNode ** root, GfxNode ** tail )
{
	if (!node)
		return 0;
	
	assert(!*tail || !(*tail)->next);

	if (node->next)
	{
		node->next->prev = node->prev;
	}

	if (node->prev)
	{
		node->prev->next = node->next;	
	}
	else if (node->parent)
	{
		node->parent->child = node->next;
	}

	if (node == *root) //!parent or prev: this must be the gfx_tree_root //To Do check these two and more tightly check them
	{
		assert(!node->prev);
		*root = node->next; 
	}
	if (node == *tail)
	{
		assert(!node->next);
		*tail = node->prev;  //if !node->prev shouldn't root =0 too?
	}

	node->parent = 0;
	node->prev   = 0;
	node->next   = 0;

	return node;
}

///////////////////////////////////////////////////////////////////////////////////
// Sky GfxTree Only Functions
#ifdef CLIENT
//Adds one new gfx_node to the gfx_tree at the given spot
GfxNode *gfxTreeInsertSky(GfxNode *parent)
{
	GfxNode	* curr;
	curr = gfxTreeNewNode( sky_gfxnode_mp );
	gfxTreeAttachNode(parent, curr, &sky_gfx_tree_root, &sky_gfx_tree_root_tail );
	return curr;
} 

//Frees this node's children and this node. 
void gfxTreeDeleteSky(GfxNode *node)
{
	if (!node)
		return;
	//checkTree(node, 0);
	gfxTreeDetatchNode(node, &sky_gfx_tree_root, &sky_gfx_tree_root_tail);
	freeTreeNodes(node->child, sky_gfxnode_mp );
	gfxTreeFreeNode(node, sky_gfxnode_mp );
}
#endif

//////////////////////////////////////////////////////////////////////////////////
/// Main GfxTree ONly functions 

int gfxTreeCountNodes(GfxNode *node, int seqHandle)
{
	int ret=0;
	if (!node)
		return 0;
	for(;node;node = node->next)
	{
		if (node->seqHandle == seqHandle)
		{
			ret++;
		}
		if (node->child)
			ret += gfxTreeCountNodes(node->child, seqHandle);
	}
	return ret;
}


//Adds one new gfx_node to the gfx_tree at the given spot
GfxNode *gfxTreeInsert(GfxNode *parent)
{
	GfxNode	* curr;
//	assert(!parent || gfxTreeCountNodes(parent, parent->seqHandle) < 128);
	curr = gfxTreeNewNode( gfxnode_mp );
	gfxTreeAttachNode(parent, curr, &gfx_tree_root, &gfx_tree_root_tail );
	return curr;
}

//Frees this node's children and this node. 
void gfxTreeDelete(GfxNode *node)
{
	if (!node)
		return;
	//checkTree(node, 0);
	gfxTreeDetatchNode(node, &gfx_tree_root, &gfx_tree_root_tail);
	freeTreeNodes(node->child, gfxnode_mp );
	gfxTreeFreeNode(node, gfxnode_mp );
}



//assumes new bro has no siblings; only called on a node that's just been detatched
static void gfxTreeAttachBrother( GfxNode * oldbro, GfxNode * newbro )
{

	assert( !newbro->next && !newbro->prev && !oldbro->prev && !newbro->parent && !oldbro->parent );
	assert(!gfx_tree_root_tail || !gfx_tree_root_tail->next);

	if(oldbro->next)
		oldbro->next->prev = newbro;
	newbro->next = oldbro->next;
	oldbro->next = newbro;
	newbro->prev = oldbro;
	newbro->parent	= oldbro->parent;
	
	assert( (!newbro || newbro->next != newbro) && (!oldbro || oldbro->next != oldbro) );
}




//D. Crazy Unlink and relink nodes Scheme
//########################Unlink and Relink Animations ####################################
//This removes all nodes associated with a particular seqHandle, and places the children of
//those nodes in suspended animation.  The expectation is that the nodes will be replaced
//by similar nodes from another animation and relinked by gfxTree RelinkSuspendedNodes.

//This function takes a branch in the gfx tree (seq->gfx_root)
//and removes every contiguous gfx_node that shares the indicated seqHandle.
//children orphaned by this process are placed in the bone id position of their parent
//in a global limbo array, suspended_node.  Here they wait to be reassigned

//Right after TreeDeleteAnimation is called, TreeRelinkSuspendedNodes should be 
//called. It goes through a replacement animation, and looks in it's suspended_node 
//mailbox.  If a gfx_node is there, it is attached to that node.  

//Right now, it assumes all nodes should be successfully reassigned.  This might change.	
  
static GfxNode * suspended_node[BONEID_COUNT];
int suspended_count = 0; 


static void removeSuspendedNode(GfxNode * node)
{
	int i;

	if (suspended_count == 0)
		return;

	for (i = 0; i < BONEID_COUNT; i++)
	{
		if (suspended_node[i] == node)
		{
			suspended_node[i] = 0;
			suspended_count--;
			return;
		}
	}
}

static void detatchFx(GfxNode * node, int seqHandle)
{
	GfxNode * next;
	GfxNode * freed;

	assert(!node || !node->prev); //should always be the eldest
	for(;node;node = next)
	{	
		assert(node->parent);
		next = node->next;

		if( node->seqHandle == seqHandle )
		{
			detatchFx(node->child, seqHandle);
		}
		else
		{
			GfxNode* parent = node->parent;
			BoneId slot = parent->anim_id;
			assert(bone_IdIsValid(slot));
			freed	= gfxTreeDetatchNode(node, &gfx_tree_root, &gfx_tree_root_tail);
			if(suspended_node[slot])
			{
				gfxTreeAttachBrother( suspended_node[slot], freed );
				//printf("detatchFx: adding suspended node brother 0x%08x [anim_id %d, parent 0x%08x, parent anim_id %d]\n",
				//	freed, freed->anim_id, parent, slot);
			}
			else
			{	
				suspended_node[slot] = freed;
				suspended_count++;
				//printf("detatchFx: suspended node 0x%08x [anim_id %d, parent 0x%08x, parent anim_id %d], suspended_count = %d\n",
				//	freed, freed->anim_id, parent, slot, suspended_count);
			}
		}
	}
}

// look up the chain from current node to see if any parent node's are invisible
bool gfxTreeParentIsVisible(GfxNode * node)
{
	GfxNode * curNode = node;
	while(curNode) {
		if(curNode->flags & GFXNODE_HIDE)
			return false;
		curNode = curNode->parent;
	}
	return true;
}


//seq->gfx_root
void gfxTreeDeleteAnimation(GfxNode * node, int seqHandle)
{
	if(!node) 
		return;
	
	ZeroArray(suspended_node);
	suspended_count = 0;
	   //checkTree(node, 0);
	   //checkTree(gfx_tree_root, 0);
	   //gfxDrawDebugTree(gfx_tree_root);
	detatchFx(node->child, seqHandle);
	   //checkTree(node, 0);
	   //gfxDrawDebugTree(gfx_tree_root);
	freeTreeNodes(node->child, gfxnode_mp);
	node->child = 0;
	   //checkTree(gfx_tree_root, 0);
	  // gfxDrawDebugTree(gfx_tree_root);
}
/*
void gfxTreeMakeMemoryFriendly( )
{

}

void gfxTreeMakeMemoryFriendly()
{
	for(;node;node = node->next)
	{
		if(node->seqHandle == seqHandle && node->useFlags )
		{
			node->alpha = alpha;
			if (node->child)
				gfxNodeSetEntAlpha(node->child,alpha,seqHandle);
		}
	}
}*/

static void gfxTreeRelinkSuspendedRecur(GfxNode * node, int seqHandle)
{	
	for( ; node ; node = node->next)
	{
		assert(	(int)node != 0xfafafafa && (int)node->model != 0xfafafafa && (int)node->unique_id != 0xfafafafa);

		if(node->seqHandle == seqHandle)
		{
			if(node->child)
				gfxTreeRelinkSuspendedRecur(node->child, seqHandle);

			if( bone_IdIsValid(node->anim_id) && suspended_node[node->anim_id] ) 
			{					
				gfxTreeAttachNode(node, suspended_node[node->anim_id], &gfx_tree_root, &gfx_tree_root_tail );
				//printf("gfxTreeRelinkSuspendedRecur: node 0x%08x [anim_id %d, parent 0x%08x, parent anim_id %d]\n",
				//	suspended_node[node->anim_id], suspended_node[node->anim_id]->anim_id, node, node->anim_id);
				
				suspended_node[node->anim_id] = 0;
				suspended_count--;
			}
		}
	}
}


//seq->gfx_root
int gfxTreeRelinkSuspendedNodes(GfxNode * node, int seqHandle)
{
	BoneId i;

	   //checkTree(node, 0);
	   //gfxDrawDebugTree(gfx_tree_root);

	gfxTreeRelinkSuspendedRecur( node->child, seqHandle );

	   //gfxDrawDebugTree(gfx_tree_root);
	   //checkTree(node, 0);

	if(suspended_count)
	{	
		//checkTree(gfx_tree_root, 0);
		for(i = 0 ; i < ARRAY_SIZE(suspended_node) ; i++ )
		{
			if(suspended_node[i])
			{
				//checkTree(suspended_node[i], 0);
				gfxTreeDelete(suspended_node[i]);
#ifdef CLIENT
				//Maybe I could move this node in some amount (like to the hips, or subtract bones till I find one?  
				//then flag it as one to be moved back up when the time is right.  (No, just ditch rebuilding altogether))
				printToScreenLog(1, "GFXTREE: %s Bone missing from model! FX will die", bone_NameFromId( i ) );
#endif
			}
		}
		//checkTree(gfx_tree_root, 0);
		return 0;
	}
	return 1;
}

void gfxTreeResetTrickFlags( GfxNode * gfxnode )
{
	GfxNode *   node;

	for( node = gfxnode ; node ; node = node->next )
	{
		if (node->trick_from_where == TRICKFROM_MODEL) {
			if (node->model && node->model->trick) {
				gfxTreeInitGfxNodeWithObjectsTricks( node, node->model->trick); //Copy the struct
				node->trick_from_where = TRICKFROM_MODEL;
			} else {
				if (node->tricks && node->tricks->info) {
					// Remove trick
					node->tricks->flags64 &= ~node->tricks->info->tnode.flags64;
					if (node->tricks->flags64) {
						// Still has some flags on it?
						node->tricks->info = NULL;
					} else {
						// Nothing left
						MP_FREE(TrickNode, node->tricks);
					}
				}
				node->trick_from_where = TRICKFROM_UNKNOWN;
			}
			if (node->tricks && node->tricks->info) {
				node->tricks->flags64 |= node->tricks->info->tnode.flags64;
			}
		} else if (!node->tricks && node->model) {
			if (node->model->trick) {
				gfxTreeInitGfxNodeWithObjectsTricks( node, node->model->trick); //Copy the struct
				node->trick_from_where = TRICKFROM_MODEL;
			}
		}
#ifdef CLIENT
		if (node->clothobj) {
			resetClothNodeParams(node);
		}
#endif

		if( node->child )
			gfxTreeResetTrickFlags( node->child );		
	}
}

