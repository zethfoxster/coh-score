#include "seqskeleton.h"
#include "stdtypes.h"
#include "mathutil.h"
#include "anim.h"
#include "seq.h"
#include "entity.h"
#include "costume.h"
#include "tricks.h"
#include "SuperAssert.h"
#include "animtrackanimate.h"
#include "earray.h"
#include "file.h"
#include "teamCommon.h"
#include "Supergroup.h"
#include "ragdoll.h"
#include "strings_opt.h"
#include "error.h"
#include "MessageStoreUtil.h"
#include "Quat.h"

#ifdef CLIENT 
#include "cmdgame.h"
#include "player.h"
#include "seqgraphics.h"
#include "font.h"
#include "render.h"
#include "fx.h"
#include "sprite_text.h"
#include "model.h"
#endif

#ifdef SERVER
#include "cmdserver.h"
#endif

#include "timing.h"

#ifdef CLIENT
static void animExtractBoneTranslations(GfxNode * gfx_node, Vec3 * BoneTranslationTotal)
{
	assert( BoneTranslationTotal );
	if(gfx_node)
	{
		BoneId boneidx = gfx_node->anim_id;
		BoneId parentboneidx = SAFE_MEMBER(gfx_node->parent, anim_id);
		
		if(!bone_IdIsValid(parentboneidx) || !bone_IdIsValid(boneidx))
			return;

		if(boneidx == 0)
		{
			zeroVec3(BoneTranslationTotal[0]);			
		}
		else
		{
			addVec3(BoneTranslationTotal[parentboneidx],
			gfx_node->mat[3], BoneTranslationTotal[boneidx]);
		}
	}
}
#endif

static Model * animFindBoneInHeader( ModelHeader * header, int id )
{
	int i;

	for( i = 0 ; i < header->model_count ; i++ )
	{
		if( id == header->models[i]->id )
		{
			return header->models[i];
		}
	}

	return 0;
}

int animCheckForLoadingObjects( GfxNode * node, int seqHandle )
{
	for( ; node ; node = node->next )
	{
		if( seqHandle == node->seqHandle )
		{
			if( node->model && node->model->loadstate != LOADED )
				return OBJECT_LOADING_IN_PROGRESS;

			if( OBJECT_LOADING_IN_PROGRESS == animCheckForLoadingObjects( node->child, seqHandle ) )
				return OBJECT_LOADING_IN_PROGRESS;
		}
	}
	return OBJECT_LOADING_COMPLETE;
}

//called by Shannon
void changeBoneScale( SeqInst * seq, F32 newbonescale )
{
	Model * model;
	ModelHeader * header;
	BoneId bone;
	Vec3 unitvec;
	Vec3 diff;

#if CLIENT
	// JE: Hackish code to have a seperate entity operated on when in SHOW_TEMPLATE mode
	// but sharing the player's costume pointer
	if (game_state.game_mode == SHOW_TEMPLATE && playerPtr() && seq == playerPtr()->seq) {
		SeqInst *seq2 = playerPtrForShell(1)->seq;
		seq = seq2;
	}
#else
	devassert(seqScaledOnServer(seq));
#endif

	unitvec[0] = unitvec[1] = unitvec[2] = 1.0;

	assert(seq);
 	assert( newbonescale >= -1.0 && newbonescale <= 1.0 );
	// newbonescale = -4.0;
	seq->bonescale_ratio = newbonescale; //just for the record, not actually needed

	if( newbonescale < 0 )
		header = seq->bonescale_anm_skinny;
	else if( newbonescale > 0 )
		header = seq->bonescale_anm_fat;
	else
		header = 0;

	seqClearBoneScales(seq);
	for(bone = 0; bone < BONEID_COUNT; bone++)
	{
		if( header )
		{
			model = animFindBoneInHeader( header, bone );
			if( model )
			{	
				Vec3 vScale;
				subVec3( model->scale, unitvec, diff );
				scaleVec3( diff, ABS(newbonescale), diff );
				addVec3(diff, unitvec, vScale);
				seqSetBoneScale(seq, bone, vScale);
			}
		}
		assert(seqBoneScale(seq, bone)[0] != 0);
	}	

	seq->updated_appearance = 1;
}

#ifdef CLIENT
void changeBoneScaleEx( SeqInst * seq, const Appearance * appearance )
{
	Model * model;
	ModelHeader * header;
	BoneId bone;
	int j, k;

	// JE: Hackish code to have a seperate entity operated on when in SHOW_TEMPLATE mode
	// but sharing the player's costume pointer
	if (game_state.game_mode == SHOW_TEMPLATE && playerPtr() && seq == playerPtr()->seq) {
		SeqInst *seq2 = playerPtrForShell(1)->seq;
		seq = seq2;
	}
	assert(seq);

	// newbonescale = -4.0;
	seq->bonescale_ratio = appearance->fBoneScale; //just for the record, not actually needed

 	seq->shoulderScale = appearance->fShoulderScale;
	seq->hipScale = appearance->fHipScale;
	seq->legScale = appearance->fLegScale;
	seq->armScale = appearance->fArmScale;
	copyVec3(appearance->fHeadScales, seq->headScales);

	for(j = 0; j < 3; j++)
	{
		// head scales are special (they are a raw scaling factor, ignoring the fat/skinny model settings)
		F32 limit;
		if(seq->headScales[j] > 0)
			limit = seq->type->headScaleMax[j];
		else
			limit = seq->type->headScaleMin[j];
		seq->headScales[j] *= limit;
	}

	if(appearance->fBoneScale <= 0)
		seq->neckScale = appearance->fBoneScale;	// for now, slave it
	else
		seq->neckScale = 0;

	seqClearBoneScales(seq);
	for(bone = 0; bone < BONEID_COUNT; bone++)
	{
		Vec3 scaleV = {0};
		Vec3 vScaleResult;

		if(bone == BONEID_WAIST)
		{
			setVec3same(scaleV, appearance->fWaistScale);
		}
		else if(bone >= BONEID_BROW && bone <= BONEID_NOSE) // brow, cheeks, chin, cranium, jaw, nose
		{
			// i guess this used to something like this:
			// int idx = (bone == BONEID_HEAD) ? 0 : ...;
			// hence starting at idx 1 but, BONEID_HEAD won't hit this block anymore
			int idx = bone - BONEID_BROW + 1;
			copyVec3(appearance->f3DScales[idx], scaleV);
		}
		else if(bone != BONEID_HEAD)
		{
			setVec3same(scaleV, appearance->fBoneScale);
		
			if(appearance->bodytype == kBodyType_Female)
			{
				if(bone == BONEID_BOSOMR || bone == BONEID_BOSOML)
					setVec3same(scaleV, appearance->fChestScale);
			}
			else // male or huge
			{
				if(bone == BONEID_CHEST)
					setVec3same(scaleV, appearance->fChestScale);
				else if(bone == BONEID_BOSOMR || bone == BONEID_BOSOML)
					setVec3same(scaleV, 0);
			}
		}
		copyVec3(onevec3, vScaleResult);

		for(k=0;k<3;k++)
		{
			scaleV[k] = MINMAX(scaleV[k], -1, 1);

			if( scaleV[k] < 0 )
				header = seq->bonescale_anm_skinny;
			else if( scaleV[k] > 0 )
				header = seq->bonescale_anm_fat;
			else
				header = 0;

			if( header )
			{
				model = animFindBoneInHeader( header, bone );
				if( model )
				{	
					vScaleResult[k] = 1 + ((model->scale[k] - 1) * ABS(scaleV[k]));
					//if(k != 1) { static float multiplier = 3.f; vScaleResult[k] *= multiplier; } // for testing extreme bone scales
			//		subVec3( model->scale, unitvec, diff );
			//		scaleVec3( diff, ABS(scale), diff );
			//		addVec3(diff, unitvec, seq->bonescales[i] );
				}
			}
		}
		seqSetBoneScale(seq, bone, vScaleResult);
		assert(seqBoneScale(seq, bone)[0] != 0);
	}	

	seq->updated_appearance = 1;
}
#endif

/*
void changeSpecificBoneScale( SeqInst * seq, int bone, F32 newbonescale )
{
	Model * model;
	ModelHeader * header;
	Vec3 diff;
	Vec3 vResult;

	// JE: Hackish code to have a seperate entity operated on when in SHOW_TEMPLATE mode
	// but sharing the player's costume pointer
	if (game_state.game_mode == SHOW_TEMPLATE && seq == playerPtr()->seq) {
		SeqInst *seq2 = playerPtrForShell(1)->seq;
		seq = seq2;
	}

	assert(seq);
	assert( newbonescale >= -1.0 && newbonescale <= 1.0 );
	// newbonescale = -4.0;
//	seq->bonescale_ratio = newbonescale; //just for the record, not actually needed

	if( newbonescale < 0 )
		header = seq->bonescale_anm_skinny;
	else if( newbonescale > 0 )
		header = seq->bonescale_anm_fat;
	else
		header = 0;

	copyVec3(onevec3, vResult);
	if( header )
	{
		model = animFindBoneInHeader( header, bone );
		if( model )
		{	
			subVec3( model->scale, onevec3, diff );
			scaleVec3( diff, ABS(newbonescale), diff );
			addVec3(diff, onevec3, vResult );
		}
	}
	seqSetBoneScale(seq, bone, vResult);
	assert(seqBoneScale(seq, bone)[0] != 0);
	
	seq->updated_appearance = 1;
}
*/


//removed possibly to be reincluded if we try the shadow sock...
/*if( gfx_node->anim_id == 0 ) //hack
{
if( seq->type->useshadow == 1 && seq->lod_level <= HIGHEST_LOD_TO_HAVE_SHADOW)
gfx_node->flags |= GFXNODE_SHADOWROOT;
}*/

static void animBuildSkeleton( const BoneLink skeleton_heirarchy[], int root_idx, GfxNode * parent, SeqInst * seq, int is_root )
{
	GfxNode * gfx_node;
	const BoneLink * bone_link;
	BoneId idx;

	//Note the build order is the same as the later traversal order.  Someday I may want to
	//guarantee that the gfxNodes are alloced in one contiguous chunk of MAX_BONES, even if the gfx_node
	//Memory Pool gets fragmented...
	for(idx = root_idx; idx != BONEID_INVALID; idx = bone_link->next)
	{
		//get the bonelink
		assert(bone_IdIsValid(idx));
		bone_link = &(skeleton_heirarchy[idx]);
		assert( bone_link && idx == bone_link->id ); //redundant really

		//insert and set up gfxnode
		gfx_node = gfxTreeInsert(parent);
		gfx_node->anim_id = bone_link->id;
#ifdef CLIENT
		gfx_node->seqGfxData = &(seq->seqGfxData);
#endif
		gfx_node->seqHandle = seq->handle;

		if( seq->type->lightAsDoorOutside )
			gfx_node->flags |= GFXNODE_LIGHTASDOOROUTSIDE;
		gfx_node->flags |= GFXNODE_SEQUENCER;

#ifdef SERVER
		if(!seqScaledOnServer(seq))
		{
			copyVec3(onevec3, gfx_node->bonescale);
		}
		else
#endif
		{
			const F32 *scale = seqBoneScale(seq, gfx_node->anim_id);
			copyVec3(scale, gfx_node->bonescale);
			seqSetBoneNeedsRotationBlending(seq, gfx_node->anim_id, 1);
			seqSetBoneNeedsTranslationBlending(seq, gfx_node->anim_id, 1);
#ifdef CLIENT
			//gfx_node->scale = MAX(seq->currgeomscale[0], seq->currgeomscale[1]) ;//(assume x=z)(this is just used for vis)
			gfxTreeSetCustomColors( gfx_node, seq->newcolor1[idx], seq->newcolor2[idx], GFXNODE_CUSTOMCOLOR );
			gfxTreeSetCustomTextures( gfx_node, seq->newtextures[idx] );
			gfx_node->rgbs = seq->rgbs_array[idx];
#endif
		}

		if( bone_link->child != -1 )
		{
			animBuildSkeleton( skeleton_heirarchy, bone_link->child, gfx_node, seq, 0 );
		}
	}
}

static BoneId bonesToDitchAt2[] =
{
	BONEID_BACK,
	BONEID_BELT,
	BONEID_BROACH,
	BONEID_COLLAR,
	BONEID_EMBLEM,
	BONEID_EYES,
	BONEID_NECK,
};

static int okToDitchBone(BoneId id, int lod, const SeqType *type)
{
	if (lod >= 2)
	{
		int i;

		// default bones to ditch
		for(i = 0; i < ARRAY_SIZE(bonesToDitchAt2); i++)
			if(bonesToDitchAt2[i] == id)
				return 1;
	}

	return 0;
}

static struct {
	char	modelName[100];
	char	filepath[MAX_PATH];
	Model*	model;
} previousModels[100];

S32 previousModelsCount;
S32 collectPreviousModels;

void animAddPreviousModel(GfxNode* node)
{
	if(	collectPreviousModels &&
		node &&
		node->model &&
		node->model->filename && 
		node->model->name &&
		previousModelsCount < ARRAY_SIZE(previousModels))
	{
		strcpy(previousModels[previousModelsCount].filepath, node->model->filename);
		strcpy(previousModels[previousModelsCount].modelName, node->model->name);
		previousModels[previousModelsCount].model = node->model;
	
		previousModelsCount++;
	}
}

extern GeoLoadData* useThisGeoLoadData;

static Model* modelFindPrevious( char *name, char * filename, int load_type, int use_type )
{
	Model* model;
	int i;
#if CLIENT
	if(game_state.disableGeometryBackgroundLoader)
	{
		useThisGeoLoadData = NULL;
	}
	else
#endif
	{
		for(i = 0; i < previousModelsCount; i++){
			if(!stricmp(filename, previousModels[i].filepath))
			{
				if(!stricmp(name, previousModels[i].modelName))
				{
					useThisGeoLoadData = NULL;
					return previousModels[i].model;
				}
				else
				{
					useThisGeoLoadData = previousModels[i].model->gld;
				}
			}
		}
	}
		
	model = modelFind(name, filename, load_type, use_type);
	
	useThisGeoLoadData = NULL;
	
	return model;
}

//Get Right Geometry for the gfx nodes, given the situation
static Model * animGetGeometryForThisBone(BoneId id, SeqInst * seq)
{
	const char *lod_suffixes[] = {"", "_LOD1", "_LOD2", "_LOD3", "_LOD4"};
	const char *geohead = 0;
	Model *model=NULL;

	if(!devassert(bone_IdIsValid(id)))
		return NULL;

	if (okToDitchBone(id, seq->lod_level, seq->type))
		return NULL;

#ifdef CLIENT
	if( !seq->newgeometry[id] || stricmp(seq->newgeometry[id],"NONE")) //if I'm not told no geometry on this bone
	{
		char		buf[100];
		char		geometry[100];
		char		filepath[300];

		int lod_level=seq->lod_level;

		while (!model && lod_level>=0)
		{
			//First try to get custom geometry
			if(seq->newgeometry[id])
			{
				//because Jeremy is doing things some crazy way and doesn't want to change it, see entity docs for explanation of this logic
				//Now really, why do I need the filename at all, why not just use one hugmongus file...
				strcpy(filepath,seq->newgeometryfile[id]);

				geohead = seq->newgeometry[id];		//geometry name
				//sprintf( geometry, "%s%s", geohead, lod_suffixes[lod_level]);
				STR_COMBINE_BEGIN(geometry);
				STR_COMBINE_CAT(geohead);
				STR_COMBINE_CAT(lod_suffixes[lod_level]);
				STR_COMBINE_END();

				if (lod_level == 0 || filepath && filepath[0]) {
					model = modelFindPrevious( geometry, filepath, LOAD_BACKGROUND, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE);
				}

				//Special rule for LOD level 1: if you couldn't find LOD1 geometry, use high LOD geometry, if available
				if( !model && lod_level == 1 )
				{
					model = modelFindPrevious( (char*)geohead, filepath, LOAD_BACKGROUND, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE);
				}

				if(!model && game_state.checkcostume && lod_level == 0)
				{
					printToScreenLog(1, " ");
					printToScreenLog(1, " ## ERROR! ## ");
					printToScreenLog(1, " %s asked for bad custom geometry: ", seq->type->name);
					printToScreenLog(1, " %s at %s", geometry, filepath);
					printToScreenLog(1, " ");
				}
			}

			//If no custom geometry, try to get Geo_Name geometry from the default files
			if (!model && seq->type->graphics)
			{
				//sprintf( buf, "GEO_%s%s", getBoneNameFromNumber( id ), lod_suffixes[lod_level] );
				STR_COMBINE_BEGIN(buf);
				STR_COMBINE_CAT("GEO_");
				STR_COMBINE_CAT(bone_NameFromId(id));
				STR_COMBINE_CAT(lod_suffixes[lod_level]);
				STR_COMBINE_END();
				model = modelFind( buf, seq->type->graphics, LOAD_BACKGROUND, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE);
			}

			lod_level--;
		} // while (!model && lod_level)

		//if (model && lod_level != seq->lod_level-1) {
		//	printf("Failed to find LOD %d for %s\n", seq->lod_level, seq->type);
		//}

		////hack for martin to force a very simple model on NPCs for his testing
		//if(game_state.martinboxes && stricmp(seq->type->name, "male") && stricmp(seq->type->name, "huge") && stricmp(seq->type->name, "fem") )
		//{
		//	//sprintf( buf, "GEO_%s", getBoneNameFromNumber( id ) );
		//	STR_COMBINE_BEGIN(buf);
		//	STR_COMBINE_CAT("GEO_");
		//	STR_COMBINE_CAT(getBoneNameFromNumber( id ));
		//	STR_COMBINE_END();
		//	model = modelFindPrevious(buf,"player_library/G_Object.geo",LOAD_BACKGROUND, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE);
		//}
	}
#endif
	return model;
}



static void animAttachGeometryToSkeleton( GfxNode * node, SeqInst * seq )
{
	for( ; node ; node = node->next )
	{
		TrickNode *trick = NULL;
		assert( bone_IdIsValid( node->anim_id ) );

		PERFINFO_AUTO_START("animGetGeometryForThisBone", 1);
			node->model = animGetGeometryForThisBone( node->anim_id, seq );

			if (node->model) {
				trick = node->model->trick;
			}else{
				PERFINFO_AUTO_START("failed", 1);
				PERFINFO_AUTO_STOP();
			}
		PERFINFO_AUTO_STOP();
			
		//Init Tricks (PTOR more explicitly get rid of old tricks here)
		if ( node->model )
		{
			PERFINFO_AUTO_START("gfxTreeInitGfxNodeWithObjectsTricks", 1);
				gfxTreeInitGfxNodeWithObjectsTricks( node, trick ); //Copy the struct
				node->model->radius = seq->type->vissphereradius; //TO DO hack figure this out //TO DO, handle big hunks of geometry like balloons
				#ifdef CLIENT
					gfxTreeSetBlendMode( node );
				#endif
			PERFINFO_AUTO_STOP();
		}

		// Set flags for reload telling it where it got its trick from
		if (node->tricks && node->model) {
			node->trick_from_where = TRICKFROM_MODEL;
		}

		//Get Shadow model
		//Currently not used, so I turned it off for performance
   		if( 0 )//&& seq->type->useshadow && seq->lod_level <= HIGHEST_LOD_TO_HAVE_SHADOW )//&& seq->type->shadowName[0] )
		{
			char buf[256];
			//sprintf( buf, "GEO_%s", getBoneNameFromNumber( node->anim_id ) );
			STR_COMBINE_BEGIN(buf);
			STR_COMBINE_CAT("GEO_");
			STR_COMBINE_CAT(bone_NameFromId(node->anim_id));
			STR_COMBINE_END();
			//node->shadow_model = modelFind( buf, seq->type->shadowName, LOAD_BACKGROUND, GEO_INIT_FOR_DRAWING | GEO_USED_BY_GFXTREE);
			if( node->shadow_model )
				node->shadow_model->radius =  MAX( 60 , seq->type->vissphereradius ); //TO DO hack figure this out //TO DO, handle big hunks of geometry like balloons
		}
		else
		{
			node->shadow_model = 0; //redundant but explicit
		}

#ifdef CLIENT
		if (node->model && modelNeedsAlphaSort(node->model)==OBJ_ALPHASORT)
			seq->alphasort = 1;
#endif

		if( node->child )
			animAttachGeometryToSkeleton( node->child, seq );
	}
}

//Get the first rotation (the base skeleton) in for a bone in an animationTrack
//TO DO move out
static void getBaseRotation( const BoneAnimTrack * bt, Mat3 mat )
{
	Quat rot;

	if( bt->flags & ROTATION_COMPRESSED_TO_5_BYTES )
	{
		animExpand5ByteQuat( (U8*)bt->rot_idx, rot );
	}
	else if( bt->flags & ROTATION_COMPRESSED_TO_8_BYTES )
	{
		S16	* cquats;
		cquats = (S16*)bt->rot_idx;
		animExpand8ByteQuat( cquats, rot );	
	}
	else if( bt->flags & ROTATION_UNCOMPRESSED )
	{
		memcpy( &rot, (F32*)bt->rot_idx, 16 );
		assert( !vec4IsZero(rot));
	}
	else if ( bt->flags & ROTATION_COMPRESSED_NONLINEAR )
	{
		animExpandNonLinearQuat( (U8*)bt->rot_idx, rot );
	}
	else
		assert(0);

	quatToMat(rot, mat);
}

//TO DO move out
//Get the first position (the base skeleton) in for a bone in an animationTrack
static void getBasePosition( const BoneAnimTrack * bt, Vec3 posVec )
{
	if( bt->flags & POSITION_UNCOMPRESSED )
	{
		F32 * myPos;
		myPos = &(((F32*)bt->pos_idx)[ 0 ]);
		copyVec3( myPos, posVec );
	}
	else if( bt->flags & POSITION_COMPRESSED_TO_6_BYTES )
	{
		Vec3 pos;
		animExpand6BytePosition( &((S16*)bt->pos_idx)[0], pos );
		copyVec3( pos, posVec );
	}
	else
		assert(0);
}



void gfxNodeSetChildUseFlags( GfxNode * node, int seqHandle )
{
	if( node && node->seqHandle == seqHandle )
	{
		node->useFlags |= GFX_USE_CHILD;
		gfxNodeSetChildUseFlags( node->parent, seqHandle );
	}
}

//Clear the GFX_USE_CHILD bit if this node no longer has any children used for anything, recurse up
void gfxNodeClearChildUseFlags( GfxNode * node, int seqHandle )
{
	int stillHasKids = 0;
	GfxNode * cNode;

	if( node && node->seqHandle == seqHandle )
	{
		for( cNode = node->child ; cNode ; cNode = cNode->next )
		{
			if( cNode->useFlags )
			{
				stillHasKids = 1;
				break;
			}
		}
		if( !stillHasKids )
		{
			node->useFlags &= ~GFX_USE_CHILD;
			gfxNodeClearChildUseFlags( node->parent, seqHandle );
		}
	}
}

/*/Debug func
void animCheckFxUse( GfxNode * pNode, int seqHandle )
{
GfxNode * node;

for( node = pNode ; node ; node = node->next )
{
if( node->child )
{
if( node->child->seqHandle != seqHandle )
node->useFlags |= GFX_USE_FX;	
else
animSetFxUse( node->child, seqHandle );
}
}
}*/

//Set the GFX_USE_FX bit
#if CLIENT
void gfxNodeSetFxUseFlag( GfxNode * node )
{
	if( node )
	{
		if(node->fxRefCount < 200)//hacky, if fxrefcount is over 200, then damn, just forget it and always draw that sucker
		{
			node->fxRefCount++;

			//Debug
			if( !((node->fxRefCount <= 1) || (node->useFlags & GFX_USE_FX) ))
			{
				char * info = fxPrintFxDebug();
				Errorf( "%s %s", textStd("FxError"), info );
				node->fxRefCount = 0;
				game_state.fxkill = 1;
				return;
			}
			//End Debug

			node->useFlags |= GFX_USE_FX;
			gfxNodeSetChildUseFlags( node->parent, node->seqHandle ); //redundant if fxRefCount>1, assert instead?

			//animCheckFxUse( GfxNode * node, int seqHandle )
		}
	}
}

//Decrement ref count and remove GFX_USE_FX no fx use this anymore
void gfxNodeClearFxUseFlag( GfxNode * node )
{
	if( node )
	{
		if(node->fxRefCount < 200)//hacky, if fxrefcount is over 200, then damn, just forget it and always draw that sucker
		{
			node->fxRefCount--;

			//Debug
			if( !( node->fxRefCount >= 0 && node->fxRefCount != 255) )
			{
				char * info = fxPrintFxDebug();
				Errorf( "%s \n%s", textStd("FxError"), info );
				node->fxRefCount = 0;
				game_state.fxkill = 1;
				return;
			}
			//End debug

			if( !node->fxRefCount )
			{
				node->useFlags &= ~GFX_USE_FX;
				if( !node->useFlags )
					gfxNodeClearChildUseFlags( node->parent, node->seqHandle );
			}
		}
	}
}
#endif

//Figure out which bones are used by skin in this animation currently
void animGetBoneUse( GfxNode * pNode, int seqHandle, int bonesUsed[BONEID_COUNT] )
{
	GfxNode * node;
	BoneInfo * bi;
	int i;

	for( node = pNode ; node ; node = node->next )
	{
		if( node->seqHandle == seqHandle )
		{
			if( node->model )
			{
				if (!(node->model->loadstate & LOADED))
					continue;
				node->useFlags |= GFX_USE_OBJ;
				bi = node->model->boneinfo;
				if( bi )
				{
					for( i = 0 ; i < bi->numbones ; i++ )
					{
						assert( bi->bone_ID[i] < BONEID_COUNT );
						bonesUsed[ bi->bone_ID[i] ] = 1;
					}
				}
			}
			if( node->child )
				animGetBoneUse( node->child, seqHandle, bonesUsed );
		}
	}
}

//Now that you know all the bones that are used in the animation, set the useFlags for the anim
void animSetBoneUseFlags( GfxNode * pNode, int seqHandle, int bonesUsed[BONEID_COUNT] )
{
	GfxNode * node;

	for( node = pNode ; node ; node = node->next )
	{
		if( node->seqHandle == seqHandle )
		{
			if( bone_IdIsValid( node->anim_id ) && bonesUsed[node->anim_id] )
			{
				node->useFlags |= GFX_USE_SKIN;
			}
			if( node->child )
				animSetBoneUseFlags( node->child, seqHandle, bonesUsed );
		}
	}
}

//Clear out all the GFX_USE_CHILD flags so I can set them again from scratch
void animClearAllParentUse( GfxNode * pNode, int seqHandle )
{
	GfxNode * node;

	for( node = pNode ; node ; node = node->next )
	{
		if( node->seqHandle == seqHandle )
		{
			node->useFlags &= ~GFX_USE_CHILD;
			if( node->child )
				animClearAllParentUse( node->child, seqHandle );
		}
	}
}

//Somewhat inefficient way to look at all the nodes, set their parent's GFX_USE_CHILD if useFlags is true
void animSetAllParentUse( GfxNode * pNode, int seqHandle )
{
	GfxNode * node;

	for( node = pNode ; node ; node = node->next )
	{
		if( node->seqHandle == seqHandle )
		{
			if( node->useFlags )
				gfxNodeSetChildUseFlags( node->parent, seqHandle );

			if( node->child )
				animSetAllParentUse( node->child, seqHandle );
		}
	}
}

//Set GFX_USE_FX again based on foreign nodes attached
#ifdef CLIENT 
void animSetFxUse( GfxNode * pNode, int seqHandle )
{
	GfxNode * node;

	for( node = pNode ; node ; node = node->next )
	{
		if( node->seqHandle != seqHandle )
		{
			//assert we are on the surface of the fx
			assert(node->parent && node->parent->seqHandle == seqHandle );
			gfxNodeSetFxUseFlag( node->parent );
		}
		else
		{
			if( node->child )
				animSetFxUse( node->child, seqHandle );
		}
	}
}
#endif

//Debug function for counting rejected nodes
static int countNonHidChildrenGfx(GfxNode * pNode, int seqHandle)
{
	GfxNode * node;
	int total = 0;
	for( node = pNode ; node ; node = node->next )
	{
		assert( node->seqHandle == seqHandle );
		if( !(node->flags & GFXNODE_HIDE) )
		{
			if( node->child )
				total+=countNonHidChildrenGfx( node->child, seqHandle );
		}
	}
	return total;
}

void animCheckUseFlagsResults(GfxNode *node, int seqHandle)
{
	for(;node;node = node->next)
	{
		//Debug
		if( node->seqHandle && !node->useFlags )
		{
			assert( node->seqHandle == seqHandle );
			countNonHidChildrenGfx(node->child, node->seqHandle);
		}
		else if( node->seqHandle ) //leave out the sky
		{

		}
		//End Debug
	}
}

void animSetBoneUseFlagsForRagdoll( GfxNode* pNode, int seqHandle )
{
	GfxNode * node;

	for( node = pNode ; node ; node = node->next )
	{
		assert( node->seqHandle == seqHandle );
		if ( isRagdollBone(node->anim_id))
			node->useFlags |= GFX_USE_SKIN;
		if ( node->child )
			animSetBoneUseFlagsForRagdoll(node->child, seqHandle);
	}
}


//Recalculate all the GFX_USE_* flags
void animCalcObjAndBoneUse( GfxNode * pNode, int seqHandle )
{
#ifdef CLIENT
	int bonesUsed[BONEID_COUNT];

	//memset( bonesUsed, 0, sizeof(int) * MAX_BONES );
	ZeroArray(bonesUsed);

	animSetFxUse( pNode, seqHandle );
	animGetBoneUse( pNode, seqHandle, bonesUsed );
	animSetBoneUseFlags( pNode, seqHandle, bonesUsed );
	animClearAllParentUse( pNode, seqHandle );
	animSetAllParentUse( pNode, seqHandle );

	if( game_state.animdebug )
		animCheckUseFlagsResults( pNode, seqHandle );
#endif
#ifdef SERVER 
	animSetBoneUseFlagsForRagdoll( pNode, seqHandle );
	animClearAllParentUse( pNode, seqHandle );
	animSetAllParentUse( pNode, seqHandle );
#endif
}










static void animAttachBoneTracks( const SkeletonAnimTrack * skeleton, const SkeletonAnimTrack * base_skeleton, GfxNode * node, Vec3 btt[] )
{
	assert( skeleton && skeleton->bone_tracks );

	for( ; node ; node = node->next )
	{
		assert( node && bone_IdIsValid( node->anim_id ) );
	
		//1. Find the new bone track to attach to the gfxnode
		node->bt = animFindTrackInSkeleton( skeleton, node->anim_id );
	 
		if( !node->bt )
		{ 
			node->bt = animFindTrackInSkeleton( skeleton->backupAnimTrack, node->anim_id );
			//11.03.05 Added this, if for some reason the bone isn't anywhere to be found, try the skeleton that the guy 
			//is actually using as a base, there is no excuse for it not to be there.
			if(!node->bt)
				node->bt = animFindTrackInSkeleton( base_skeleton, node->anim_id );

			node->flags |= GFXNODE_DONTANIMATE; //if you are relying on backups, dont bother to run the anim
		}
		else
		{
			node->flags &= ~GFXNODE_DONTANIMATE;
		}

		//TO DO shouldn't this be flaged as a bug?
		if( !node->bt )
			continue;

		//MULTITYPES  getting the deltas between the playing anim's base sjkeleton and the sequencers base skeleton allows you to play male anims on females for example, if they differ only in base skeleton
		//(using the base_skeleton instead of the skeleton to generate the initial xforams and btt
		{
			const BoneAnimTrack * baseBt;

			baseBt = animFindTrackInSkeleton( base_skeleton, node->anim_id );
			if(!baseBt)
			{
				Errorf("Could not find animTrack for bone (%s) in %s", bone_NameFromId(node->anim_id), base_skeleton->name);
				continue;
				// assert( baseBt );
			}
			/////////// Get base positions and rotations
			//These initial matrices in node->mat are overwritten immediately by animPlay
			//They are here so you can turn off animPlay w/o crashing, and as a place to store them for a sec till
			//anim ExtractBoneTranslations uses them
			getBasePosition( baseBt, node->mat[3] );
			getBaseRotation( baseBt, node->mat ); //(should be none  No base skeleton is allowed to have rotations.)
		}

		{ //Calcuate the node->animDelta for later
			Vec3 basePos;
			Vec3 animBasPos;

			copyVec3( node->mat[3], basePos );
			getBasePosition( node->bt, animBasPos );  //can move this out to seqLoad, but little need, probably
			subVec3( animBasPos, basePos, node->animDelta );	
		}

#ifdef CLIENT
		animExtractBoneTranslations( node, btt );
#endif
		
		if( node->child )
			animAttachBoneTracks( skeleton, base_skeleton, node->child, btt );
 	}
}

void animSetHeader(SeqInst *seq, int preserveOldAnimation )
{
	Animation * anim;
	int relink;

	int loadType = LOAD_ALL;

	PERFINFO_AUTO_START("animSetHeader", 1);

		PERFINFO_AUTO_START("top", 1);
		
			#ifdef SERVER 
				loadType = LOAD_ALL_SHARED;
			#endif

			assert(seq->gfx_root && seq->gfx_root->unique_id != -1);

			anim = &seq->animation;
			
			//Move to sequencer creation
			{
				char filename[MAX_PATH];
				const TypeP * typeP = seqGetTypeP( seq->info, seq->type->seqTypeName );

				if (!typeP)
					FatalErrorf( "Couldn't find TypeDef %s in Sequencer %s", seq->type->seqTypeName, seq->info->name );

				//sprintf(filename, "sequencers/%s", seq->info->name);
				STR_COMBINE_BEGIN(filename);
				STR_COMBINE_CAT("sequencers/");
				STR_COMBINE_CAT(seq->info->name);
				STR_COMBINE_END();

				seq->animation.baseAnimTrack = animGetAnimTrack( typeP->baseSkeleton, loadType, filename );
				if( !seq->animation.baseAnimTrack )
					FatalErrorf( "Animation file %s doesn't exist. Sequencer %s wants it as a base skeleton", typeP->baseSkeleton, seq->type->seqTypeName );
			}
			assert( seq->animation.baseAnimTrack && anim->move );

			//PTOR can problably get rid of this too
			if (!preserveOldAnimation)
			{
				seq->curr_interp_frame = 0;
				seqClearAllBoneAnimationStoredBits(seq);
			}
			
			if(BLEND_NONE!=0)
			{
				assert(0);
			}
			//memset(seq->curr_interp_state, BLEND_NONE, sizeof(char)*MAX_BONES);
			seqClearAllBlendingBits(seq);

			collectPreviousModels = 1;

		PERFINFO_AUTO_STOP_START("middle", 1);

			//TO DO move this out or gone altogether
			if( gfxTreeNodeIsValid( seq->gfx_root, seq->gfx_root_unique_id ) )
			{
				previousModelsCount = 0;
				gfxTreeDeleteAnimation( seq->gfx_root, seq->handle );
			}

			seq->gfx_root->anim_id = BONEID_INVALID;

#ifdef CLIENT 
			seq->gfx_root->seqGfxData = &(seq->seqGfxData);
#endif
			
		PERFINFO_AUTO_STOP_START("middle2", 1);

			animBuildSkeleton(	seq->animation.baseAnimTrack->skeletonHeirarchy->skeleton_heirarchy,
								seq->animation.baseAnimTrack->skeletonHeirarchy->heirarchy_root,
								seq->gfx_root,
								seq,
								1 );

#ifdef CLIENT
			//Debug
			if( STATE_STRUCT.quickLoadAnims && !anim->typeGfx->animTrack )

			{
				seqAttachAnAnim( cpp_const_cast(TypeGfx*)(anim->typeGfx), loadType );
			}
			//End debug
#endif

		PERFINFO_AUTO_STOP_START("middle3", 1);
			allocateLastBoneTransforms(seq);

#ifdef CLIENT 
			animAttachBoneTracks( anim->typeGfx->animTrack, seq->animation.baseAnimTrack, seq->gfx_root->child, seq->seqGfxData.btt);
#else
			animAttachBoneTracks( anim->typeGfx->animTrack, seq->animation.baseAnimTrack, seq->gfx_root->child, NULL);
#endif

		PERFINFO_AUTO_STOP_START("middle4", 1);

			//TO DO move out to not done all the time  ( ########## Performance Culprit ########### )
			#ifdef CLIENT
				seq->alphasort = 0;
				animAttachGeometryToSkeleton( seq->gfx_root->child, seq );
			#endif

			collectPreviousModels = 0;
			previousModelsCount = 0;

		PERFINFO_AUTO_STOP_START("middle5", 1);

			//TO DO move this out or gone altogethr
			relink = gfxTreeRelinkSuspendedNodes( seq->gfx_root, seq->handle );

			#ifdef CLIENT
				if(!relink)
					printToScreenLog(1, "     (Model w/o bone: %s) \n", anim->move->typeGfx[0]->animP[0]->name);
			#endif

			// Check to see if weapons should be hidden for this animation (eg for swimming, beastrun, etc)
			{
				GfxNode *nodeWeapR = gfxTreeFindBoneInAnimation(BONEID_WEPR, seq->gfx_root->child, seq->handle, 1);
				GfxNode *nodeWeapL = gfxTreeFindBoneInAnimation(BONEID_WEPL, seq->gfx_root->child, seq->handle, 1);

				if( getMoveFlags(seq, SEQMOVE_HIDEWEAPON) )
				{
					//printf("seqSetMove: hiding weapon (node 0x%08x), WEPR location {%.1f, %.1f, %.1f}\n", nodeWeapR, nodeWeapR->mat[3][0], nodeWeapR->mat[3][1], nodeWeapR->mat[3][2]);
					if(nodeWeapR) nodeWeapR->flags |= GFXNODE_HIDE;
					if(nodeWeapL) nodeWeapL->flags |= GFXNODE_HIDE;
				}
				else
				{
					// unhide in case was hidden in prev animation (not sure if need this, if GfxNode's are always
					//	recreated for each animation, see gfxTreeDeleteAnimation and animAttachGeometryToSkeleton above)
					if(nodeWeapR) nodeWeapR->flags &= ~GFXNODE_HIDE;
					if(nodeWeapL) nodeWeapL->flags &= ~GFXNODE_HIDE;
				}
			}

			//TO DO move this out to the original seq build
			//if( seq->gfx_root )
			//	seq->gfx_root->flags |= GFXNODE_ANIMATIONROOT;

		PERFINFO_AUTO_STOP_START("middle6", 1);

			//For the nodelator
			seq->loadingObjects = animCheckForLoadingObjects( seq->gfx_root->child, seq->handle );

			//Figure out whether this should be alpha sorted based on the textures on it

		PERFINFO_AUTO_STOP_START("middle7", 1);

			#ifdef CLIENT
				animCalculateSortFlags( seq->gfx_root->child );
			#endif

		PERFINFO_AUTO_STOP_START("bottom", 1);

			animCalcObjAndBoneUse(seq->gfx_root->child, seq->handle );

		PERFINFO_AUTO_STOP();

	PERFINFO_AUTO_STOP();
}


