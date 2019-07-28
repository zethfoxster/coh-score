#define RT_ALLOW_VBO
#include "renderbonedmodel.h"
#include "model.h"
#include "ogl.h"
#include <stdio.h>
#include "tricks.h"
#include "error.h"
#include "model.h"
#include "memcheck.h"
#include "assert.h"
#include "mathutil.h"
#include "gfxtree.h"
#include "render.h"
#include "cmdgame.h"
#include "wcw_statemgmt.h"
#include "nvparse.h"
#include "camera.h"
#include "bump.h"
#include "shadersATI.h"
#include "utils.h"
#include "rendermodel.h"
#include "model_cache.h"
#include "rt_tex.h"
#include "anim.h"
#include "cubemap.h"

extern int skin_vpid;
BoneInfo myfakeboneinfo, myfakeboneinfo_list[BONEID_COUNT];
int myfakeboneinfo_weights_size=0; // Number allocated
int myfakeboneinfo_matidxs_size=0; // Number allocated
int mfbinit = 0;
//int	myfakeboneinfoVBOarrayID_Matidxs;
//int	myfakeboneinfoVBOarrayID_Weights;


void initMyFakeBoneInfo()
{
	int i;
	mfbinit = 1;
	myfakeboneinfo.numbones = 1;

	if( rdr_caps.use_vbos ) // VBO code does this in modelSetupVertexObject
		return;

	if (myfakeboneinfo_weights_size==0) {
		dynArrayFit((void**)&myfakeboneinfo.weights, sizeof(Weights), &myfakeboneinfo_weights_size, 384);
		dynArrayFit((void**)&myfakeboneinfo.matidxs, sizeof(Matidxs), &myfakeboneinfo_matidxs_size, 384);
	}
	for(i = 0; i < myfakeboneinfo_weights_size; i++)
	{
		myfakeboneinfo.weights[i][0] = 1.0;
		myfakeboneinfo.weights[i][1] = 0.0;
		myfakeboneinfo.matidxs[i][0] = 0;
		myfakeboneinfo.matidxs[i][1] = 0;
	}
	for (i=0; i<ARRAY_SIZE(myfakeboneinfo_list); i++) {
		myfakeboneinfo_list[i].weights = myfakeboneinfo.weights;
		myfakeboneinfo_list[i].matidxs = myfakeboneinfo.matidxs;
	}
}

void fixUpFakeBoneInfo(Model *model)
{
	if (!model->boneinfo->weights || model->boneinfo->weights != myfakeboneinfo.weights)
		// Not a fake bone info
		return;

	// Grow fake weights and matidxs array to fit
	if (model->vert_count > myfakeboneinfo_weights_size) {
		//printf("growing to %d\n", model->vert_count);
		dynArrayFit((void**)&myfakeboneinfo.weights, sizeof(Weights), &myfakeboneinfo_weights_size, model->vert_count);
		dynArrayFit((void**)&myfakeboneinfo.matidxs, sizeof(Matidxs), &myfakeboneinfo_matidxs_size, model->vert_count);
		initMyFakeBoneInfo(); // Reset existing pointers
	}
}

//TO DO, this is a bit sloppy
//Any model that 
BoneInfo * assignDummyBoneInfo(char * model_name)
{
	BoneInfo * boneinfo;
	BoneId idx;

	if(!mfbinit)		
		initMyFakeBoneInfo();

	//Extract the core bone name from the geometry name
	idx = bone_IdFromText(model_name+4);
	if(idx == BONEID_INVALID)
		idx = BONEID_DEFAULT;

	myfakeboneinfo_list[idx].bone_ID[0] = idx;
	myfakeboneinfo.bone_ID[1] = 0;
	boneinfo = &myfakeboneinfo_list[idx];
	boneinfo->weights = myfakeboneinfo.weights;
	boneinfo->matidxs = myfakeboneinfo.matidxs;
	boneinfo->numbones = 1;
	return boneinfo;
}

//A SeqGfxData holds info a gfxNode needs to know about it's sequencer, like positions of 
//other bones, and lighting.  If you want to draw an object using modelDrawBonedNode that doesn't
//have a sequencer, we fake it with this function.
SeqGfxData * assignDummySeqGfxData( Mat4 mymat )
{
	static SeqGfxData myFakeSeqGfxData;
	static int mfbinit = 0;
	if(!mfbinit)		
	{
		memset( &myFakeSeqGfxData, 0, sizeof(SeqGfxData) );
		mfbinit = 0;
	}
	myFakeSeqGfxData.light.ambientScale = 1.0;
	myFakeSeqGfxData.light.diffuseScale = 1.0;
	copyMat4( mymat, myFakeSeqGfxData.bpt[0] );

	return &myFakeSeqGfxData;
}

//Build a barebones GfxNode so you can call modelDrawBonedNode on things that aren't part of a sequencer
GfxNode * modelInitADummyNode( Model *model, Mat4 mat )
{
	static GfxNode node;
	static TrickNode trick;

	node.seqGfxData	= assignDummySeqGfxData( mat );
	node.tricks		= model->trick;
	node.model		= model;
	node.alpha		= 255;
	node.model->boneinfo = &myfakeboneinfo_list[0];

	return &node;
}


static void copyBonesForSock(GfxNode *node,SkinModel *skin)
{
	F32	 * offset;
	BoneId *idx_table;
	Mat4 * bone_table;
	Vec3 * bonemat;
	int	   j;
	SeqGfxData		*seqGfxData = node->seqGfxData;
	BoneInfo		*boneinfo;

	boneinfo			= node->model->boneinfo;
	skin->bone_count	= boneinfo ? boneinfo->numbones : 0;
	offset				= seqGfxData->btt[node->anim_id];
	idx_table			= boneinfo->bone_ID;
	bone_table			= seqGfxData->bpt;

	for(j = 0 ; j < boneinfo->numbones ; j++)     
	{
		bonemat = bone_table[idx_table[j]]; 
		mulVecMat4(offset,bonemat,skin->bone_mats[j][3]);  
		copyMat3(bonemat,skin->bone_mats[j]);
	}
}

//debug
int drawOrder;
int next;
int boned;
int nonboned;
int nonboned2;
//end debug

void modelDrawBonedNode(GfxNode *node, BlendModeType blend_mode, int tex_index, Mat4 viewMatrix)
{
	SkinModel	*skin;
	int			size, i;
	RdrTexList	*texlist,*src_texlist;
	TrickNode	*trick;
	Model		*model = node->model;
	int			bonecount;
	int			tex_count = (tex_index==-1)?model->tex_count:1;
	F32			cubemap_attenuation = 0.0f;

	// Asserts from legacy modelDrawNode()
	assert(blend_mode.shader);
	assert( (model->flags & OBJ_DRAWBONED) && (node->flags & GFXNODE_SEQUENCER));
	assert(model->boneinfo);
	
	drawOrder = node->unique_id * next++; //debug
	boned++; //debug

	if(!model->boneinfo)
	{
		model->boneinfo = assignDummyBoneInfo(model->name);
		fixUpFakeBoneInfo(model);
	}
	modelSetupVertexObject(model, VBOS_USE, blend_mode);
	bonecount = model->boneinfo->numbones;

	if (!model->vbo->sts) //TO DO this should be an error
		return;

	// prepare the texture list
	src_texlist = modelDemandLoadTextures(node->model,(node->flags & GFXNODE_CUSTOMTEX) ? &node->customtex : 0, 0, tex_index, 0, false, 0);

	for (i=0; i<tex_count; i++)
	{
		src_texlist[i].blend_mode = blend_mode; // Set blend mode calculated in addViewSortNodeSkinned

		// determine the correct cubemap texture ID if necessary
		if( blend_mode.blend_bits & BMB_CUBEMAP )
		{
			// This will be set if artists use "cubemap %dynamic%" in the material trick
			bool bForceDynamicCubemap = (src_texlist[i].texid[TEXLAYER_CUBEMAP] == dummy_dynamic_cubemap_tex->id);

			// The most common case for skinned part rendering is that the reflection system will supply
			// a texture ID of the dynamic environment cubemap (if available, e.g. not in a menu screen)
			if( !src_texlist[i].texid[TEXLAYER_CUBEMAP] || bForceDynamicCubemap )
			{
				// let cubemap module decide the cubemap texture.
				// this will return the dynamic cubemap id if available
				// however in the case of the character creator it will return a genericstatic static cubemap
				src_texlist[i].texid[TEXLAYER_CUBEMAP] = cubemap_GetTextureID(false, viewMatrix[3], bForceDynamicCubemap);
				// @todo does it still make sense to update the attenuation for a statically assigned cubemap?
				cubemap_attenuation = cubemap_CalculateAttenuation(false, viewMatrix[3], model->radius, bForceDynamicCubemap);
			}
			else
			{
				// The artist has specified an explicit static cubemap for this skinned piece in the material, e.g.:
				//       CubeMap generic_cubemap_face0.tga 
				// It should have been demand loaded above and we can just pass along the texture ID from that step.
				// @todo should attenuation still be calculated in this case? Probably not.
				cubemap_UseStaticCubemap();
			}
		}			
	}

	size = sizeof(SkinModel) + sizeof(Mat4) * (bonecount-1) + sizeof(RdrTexList) * tex_count;
	if (node->tricks)
		size += sizeof(TrickNode);
	skin = rdrQueueAlloc(DRAWCMD_BONEDMODEL,size);
	texlist = (RdrTexList*) (skin->bone_mats+bonecount);
	memcpy(texlist,src_texlist,sizeof(RdrTexList) * tex_count);

	skin->vbo = model->vbo;
	skin->debug_model_backpointer = model;
	copyVec3(node->rgba,skin->rgba[0]);
	copyVec3(node->rgba2,skin->rgba[1]);
	skin->rgba[0][3] = node->alpha;
	skin->tex_index = tex_index;
	//setBlendModes(texlist, node->model, blend_mode, tex_index, dist);

	if (tex_index!=-1)
		assert(blend_mode.shader == texlist->blend_mode.shader);

	skin->flags	= node->flags;
#ifndef FINAL
	if (game_state.gfxdebug && (!game_state.gfxdebugname[0] || strstri(model->name, game_state.gfxdebugname)))
		skin->flags |= GFXNODE_DEBUGME;
#endif

	modelDrawGetCharacterLighting(node, skin->ambient, skin->diffuse, skin->lightdir);
	copyBonesForSock(node,skin);
	if (node->tricks)
	{
		trick = (TrickNode*)(texlist + tex_count);
		*trick = *node->tricks;
		skin->has_trick = 1;
	}
	else
		skin->has_trick = 0;

	skin->cubemap_attenuation = cubemap_attenuation;

	rdrQueueSend();
}
















#if 0
	// mike's bone drawing code
			if(node->anim_id == 0)
			{
 				Entity *playerPtr();
 				Entity * e = playerPtr();

				glLoadIdentity(); CHECKGL; 

				// draw all bones (not very efficient right now, as it will draw all bones for each node drawn, redundantly...)
				if(e)
				{ 
					Mat4		temp;
					Mat4		mat;
					F32 * v;

				 	glLineWidth(5); CHECKGL;
   					glPointSize(6); CHECKGL;
					
 					for(i = 0 ; i <= 30 ; i++)
					{
 						if(i>=13 && i<=22)
							continue;

   						node = fxFindGfxNodeGivenBoneNum(e->seq, i);
						gfxTreeFindWorldSpaceMat(mat, node);
    					mulMat4(  cam_info.viewmat, mat, temp);
						copyMat4(temp, mat);
						
 						WCW_Color4(255, 0, 0, 255);
						glBegin(GL_POINTS);
							v = mat[3];
							glVertex3f(v[0], v[1], v[2]);
 						glEnd(); CHECKGL;

 						if(node->parent && node->anim_id != 0)
						{
 							WCW_Color4(255, 255, 0, 255);
							glBegin(GL_LINES);
								glVertex3f(v[0], v[1], v[2]);

								node = fxFindGfxNodeGivenBoneNum(e->seq, node->parent->anim_id);
								gfxTreeFindWorldSpaceMat(mat, node);
								mulMat4( cam_info.viewmat, mat, temp);
								copyMat4(temp, mat);

								v = mat[3];
								glVertex3f(v[0], v[1], v[2]);

							glEnd(); CHECKGL;
						}
					}

					glLineWidth(1); CHECKGL;
				}
			}
		}
#endif
