/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 */
#define RT_ALLOW_VBO
#include "model.h"
#include "ogl.h"
#include "wcw_statemgmt.h" 
#include "renderstats.h" 
#include "bump.h" 
#include "renderbonedmodel.h" 
#include "error.h"
#include "MemoryMonitor.h"
#include "timing.h"
#include "rt_state.h"
#include "tex.h"
#include "tricks.h"
#include "renderprim.h"
#include "model_cache.h" 
#include "mathutil.h"
#include "cmdgame.h"

static CRITICAL_SECTION model_unpack_cs;

void modelCacheInitUnpackCS(void)
{
	InitializeCriticalSection(&model_unpack_cs);
}


void modelFreeVertexObject(VertexBufferObject *vbo)
{
	if (!vbo)
		return;
	rdrQueue(DRAWCMD_FREEVBO,&vbo,sizeof(vbo));
}

#if 0
void *modelConvertRgbBuffer(U8 *rgbs,int vert_count)
{
	ConvertRgb	*c;
	int			size;

	if (!rdr_caps.use_vbos)
		return rgbs;
	size = sizeof(ConvertRgb) + vert_count * 4;
	c = _alloca(size);
	memcpy(c->rgbs,rgbs,vert_count*4);
	c->vert_count	= vert_count;
	c->handle		= rdrAllocBufferHandle();
	rdrQueue(DRAWCMD_CONVERTRGBBUFFER,c,size);
	return (void*)c->handle;
}
#else
void *modelConvertRgbBuffer(U8 *rgbs,int vert_count)
{
	ConvertRgb	*c;
	int			handle;

	if (!rdr_caps.use_vbos)
		return rgbs;
	handle = rdrAllocBufferHandle();
	c = rdrQueueAlloc(DRAWCMD_CONVERTRGBBUFFER,sizeof(ConvertRgb) + vert_count * 4);
	memcpy(c->rgbs,rgbs,vert_count*4);
	c->vert_count	= vert_count;
	c->handle		= handle;
	rdrQueueSend();
	return (void*)handle;
}
#endif

void modelFreeRgbBuffer(U8 *rgbs)
{
	if (!rdr_caps.use_vbos)
		rdrFreeMem(rgbs); // Free asynchronously
	else
		rdrFreeBuffer(*(int *)&rgbs,"RGB_VBO");
}


static void modelFixup(Model *model)
{
	int j;
	VBO		*vbo = model->vbo;

	if(vbo->sts)
	{
		F32		scale = 1.f;
		int		scale_sts = 0;

		//1 . Flip v orientation. TO DO: this can be done in pre processing
		for(j=0;j<model->vert_count;j++)
		{
			vbo->sts[j][1] = 1.f - vbo->sts[j][1];
			if( vbo->sts2 ) 
				vbo->sts2[j][1] = 1.f - vbo->sts2[j][1];
		}

		//2. is any scaling done on these textures?, if so, do that
		for( j = 0 ; j < model->tex_count ; j++ )
		{
			TexBind *bind = model->tex_binds[j];
			if( !(bind->bind_scale[0][0] == 1.0 && bind->bind_scale[0][1] == 1.0 && bind->bind_scale[1][0] == 1.0 && bind->bind_scale[1][1] == 1.0) )
			{
				scale_sts = 1;
				break;
			}
		}

		// apply scaling
		// these 'cpu scaled' texture coordinates are only used by the old legacy materials and fallbacks
		// 'multi' materials do texture coordinate generation in the fragment shaders (see ScrollScales)
		// @todo, @bug:
		// If sub-object meshes share seam vertices and have different scaling applied then 
		// the scaling of the coordinate will only be done for the first sub-object we visit
		// and those texture coordinates will be wrong for the sub object on the other side of the
		// seam which usually leads to some pretty bad texture streaking along the first strip
		// of triangles along an edge adjoining different sub objects.
		if( scale_sts )
		{
			int ele_base = 0;
			// setup a tracking table to only visit a vertex once
			// otherwise we will incorrectly accumulate the scaling when
			// different triangles have the same vertex
			char *used = _alloca(model->vert_count);
			ZeroStructs(used, model->vert_count);
			for(j=0;j<model->tex_count;j++)
			{
				TexBind *bind = model->tex_binds[j];
				int sub_obj_tri_count = model->tex_idx[j].count;
				int sub_obj_index_count = sub_obj_tri_count*3;
				int	k,idx;

				for(k=ele_base;k<ele_base + sub_obj_index_count;k++)
				{
					idx = vbo->tris[k];
					if (used[idx])
						continue;	// we have already visited this vertex

					used[idx] = 1;	// mark vertex as visited

					// scale texture coords
					vbo->sts[idx][0] *= bind->bind_scale[0][0];
					vbo->sts[idx][1] *= bind->bind_scale[0][1];
					if (vbo->sts2) {	//if DUALTEXTUREs with different scaling, you need separate sts2s
						vbo->sts2[idx][0] *= bind->bind_scale[1][0];
						vbo->sts2[idx][1] *= bind->bind_scale[1][1];
					}
				}
				ele_base += sub_obj_index_count;
			}
		}
	}

	// 3. replace with dummy normals if necessary
	if (vbo->norms && (model->flags & OBJ_NOLIGHTANGLE))
	{
		int		i;
		for(i=0;i<model->vert_count;i++)
			setVec3(vbo->norms[i],1,-1,1);
	}
}

void modelUnpackReductions(Model *model)
{
	int deltalen;
	U8 *ptr, *reduction_data;
	void *new_data;
	GMeshReductions *reductions;

	EnterCriticalSection(&model_unpack_cs);

	if (model->unpack.reductions)
	{
		LeaveCriticalSection(&model_unpack_cs);
		return;
	}

	reductions = model->unpack.reductions = calloc(sizeof(GMeshReductions), 1);

	ptr = reduction_data = malloc(model->pack.reductions.unpacksize);
	geoUnpack(&model->pack.reductions, reduction_data, model->name, model->filename);

#define UNPACK_INT				*((int *)ptr); ptr += sizeof(int)
#define UNPACK_INTS(count)		new_data = malloc(count * sizeof(int)); memcpy(new_data, ptr, count * sizeof(int)); ptr += count * sizeof(int)
#define UNPACK_FLOATS(count)	new_data = malloc(count * sizeof(float)); memcpy(new_data, ptr, count * sizeof(float)); ptr += count * sizeof(float)

	reductions->num_reductions = UNPACK_INT;
	reductions->num_tris_left = UNPACK_INTS(reductions->num_reductions);
	reductions->error_values = UNPACK_FLOATS(reductions->num_reductions);
	reductions->remaps_counts = UNPACK_INTS(reductions->num_reductions);
	reductions->changes_counts = UNPACK_INTS(reductions->num_reductions);

	reductions->total_remaps = UNPACK_INT;
	reductions->remaps = UNPACK_INTS(reductions->total_remaps * 3);
	reductions->total_remap_tris = UNPACK_INT;
	reductions->remap_tris = UNPACK_INTS(reductions->total_remap_tris);

	reductions->total_changes = UNPACK_INT;
	reductions->changes = UNPACK_INTS(reductions->total_changes);

	deltalen = UNPACK_INT;
	reductions->positions = malloc(reductions->total_changes * sizeof(Vec3));
	uncompressDeltas(reductions->positions, ptr, 3, reductions->total_changes, PACK_F32);
	ptr += deltalen;

	deltalen = UNPACK_INT;
	reductions->tex1s = malloc(reductions->total_changes * sizeof(Vec2));
	uncompressDeltas(reductions->tex1s, ptr, 2, reductions->total_changes, PACK_F32);
	ptr += deltalen;

	assert(ptr == reduction_data + model->pack.reductions.unpacksize);

	free(reduction_data);

	LeaveCriticalSection(&model_unpack_cs);

#undef UNPACK_INT
#undef UNPACK_INTS
#undef UNPACK_FLOATS
}

void modelFreeAllUnpacked(Model *model)
{
	EnterCriticalSection(&model_unpack_cs);
	SAFE_FREE(model->unpack.tris);
	SAFE_FREE(model->unpack.verts);
	SAFE_FREE(model->unpack.norms);
	SAFE_FREE(model->unpack.sts);
	SAFE_FREE(model->unpack.sts3);
	freeGMeshReductions(model->unpack.reductions);
	model->unpack.reductions = 0;
	LeaveCriticalSection(&model_unpack_cs);
}


void modelUnpackAll(Model *model)
{
	EnterCriticalSection(&model_unpack_cs);

	if (!model->unpack.tris)
	{
		model->unpack.tris = malloc(sizeof(int) * 3 * model->tri_count);
		geoUnpackDeltas(&model->pack.tris,model->unpack.tris,3,model->tri_count,PACK_U32,model->name,model->filename);
	}

	if (!model->unpack.verts)
	{
		model->unpack.verts = malloc(sizeof(F32) * 3 * model->vert_count);
		geoUnpackDeltas(&model->pack.verts,model->unpack.verts,3,model->vert_count,PACK_F32,model->name,model->filename);
	}

	if (!model->unpack.norms && model->pack.norms.unpacksize)
	{
		model->unpack.norms = malloc(sizeof(F32) * 3 * model->vert_count);
		geoUnpackDeltas(&model->pack.norms,model->unpack.norms,3,model->vert_count,PACK_F32,model->name,model->filename);
	}

	if (!model->unpack.sts && model->pack.sts.unpacksize)
	{
		model->unpack.sts = malloc(sizeof(F32) * 2 * model->vert_count);
		geoUnpackDeltas(&model->pack.sts,model->unpack.sts,2,model->vert_count,PACK_F32,model->name,model->filename);
	}

	if (!model->unpack.sts3 && model->pack.sts3.unpacksize)
	{
		model->unpack.sts3 = malloc(sizeof(F32) * 2 * model->vert_count);
		geoUnpackDeltas(&model->pack.sts3,model->unpack.sts3,2,model->vert_count,PACK_F32,model->name,model->filename);
	}

	LeaveCriticalSection(&model_unpack_cs);
}

void modelSetupZOTris(Model *model)
{
	EnterCriticalSection(&model_unpack_cs);

	if (!model->unpack.tris)
	{
		model->unpack.tris = malloc(sizeof(int) * 3 * model->tri_count);
		geoUnpackDeltas(&model->pack.tris,model->unpack.tris,3,model->tri_count,PACK_U32,model->name,model->filename);
	}

	if (!model->unpack.verts)
	{
		model->unpack.verts = malloc(sizeof(F32) * 3 * model->vert_count);
		geoUnpackDeltas(&model->pack.verts,model->unpack.verts,3,model->vert_count,PACK_F32,model->name,model->filename);
	}

	LeaveCriticalSection(&model_unpack_cs);
}

void modelSetupPRQuads(Model *model)
{
	// Planar quads don't need critical section protection since they are only access from the main thread
	//EnterCriticalSection(&model_unpack_cs);

	if (!model->unpack.reflection_quads)
	{
		model->unpack.reflection_quads = malloc(sizeof(F32) * 3 * 4 * model->reflection_quad_count);
		geoUnpackDeltas(&model->pack.reflection_quads,model->unpack.reflection_quads,3,model->reflection_quad_count*4,PACK_F32,model->name,model->filename);
	}

	//LeaveCriticalSection(&model_unpack_cs);
}

void modelSetupRadiosity(Model *model)
{
	EnterCriticalSection(&model_unpack_cs);

	if (!model->unpack.tris)
	{
		model->unpack.tris = malloc(sizeof(int) * 3 * model->tri_count);
		geoUnpackDeltas(&model->pack.tris,model->unpack.tris,3,model->tri_count,PACK_U32,model->name,model->filename);
	}

	if (!model->unpack.verts)
	{
		model->unpack.verts = malloc(sizeof(F32) * 3 * model->vert_count);
		geoUnpackDeltas(&model->pack.verts,model->unpack.verts,3,model->vert_count,PACK_F32,model->name,model->filename);
	}

	if (!model->unpack.sts3 && model->pack.sts3.unpacksize)
	{
		model->unpack.sts3 = malloc(sizeof(F32) * 2 * model->vert_count);
		geoUnpackDeltas(&model->pack.sts3,model->unpack.sts3,2,model->vert_count,PACK_F32,model->name,model->filename);
	}

	LeaveCriticalSection(&model_unpack_cs);
}

MemoryPool MP_NAME(VBO) = 0;
CRITICAL_SECTION vbo_crit_sect;
void modelSetupVertexObject(Model *model, int useVbos, BlendModeType blend_mode )
{
	static int crit_init = 0;
	int			idx,total,vert_total,tri_bytes,vert_bytes,norm_bytes,st1_bytes,st2_bytes,st3_bytes,
				tangent_bytes,weight_bytes,matidx_bytes,delta;
	U8			*data;
	VBO			*vbo;
	bool		needBump = blendModeHasBump(blend_mode);
	bool		isMulti = (model->common_blend_mode.shader == BLENDMODE_MULTI) && blend_mode.shader == BLENDMODE_MULTI;

	//If you've already set up this guy, don't do it again unless something has changed
	if (vbo=model->vbo)
	{
		if ((vbo->bump_init || !(isBumpMapped(model) || needBump)) && // We already have bump inited or we don't need it
			(vbo->sts2_init || isMulti)) // We already have sts2 inited or we don't need it
			return;
		modelFreeVertexObject(vbo);
		vbo = model->vbo = NULL;
	}

	if (!crit_init)
	{
		InitializeCriticalSection(&vbo_crit_sect);
		crit_init = 1;
	}

	MP_CREATE(VBO, 2048);
	EnterCriticalSection(&vbo_crit_sect);
	vbo = model->vbo = MP_ALLOC(VBO);
	LeaveCriticalSection(&vbo_crit_sect);

	needBump = needBump || isBumpMapped(model) || vbo->bump_init;

	idx = norm_bytes = st1_bytes = st2_bytes = st3_bytes = tangent_bytes = weight_bytes = matidx_bytes = 0;

	tri_bytes	= sizeof(GL_INT) * 3 * model->tri_count;

	vert_bytes	= sizeof(vbo->verts[0]) * model->vert_count;
	if (modelHasNorms(model))
		norm_bytes	= sizeof(vbo->norms[0]) * model->vert_count;
	if (modelHasSts(model))
		st1_bytes	= sizeof(vbo->sts[0]) * model->vert_count;
	if (isMulti) {
		vbo->sts2_init = 0;
		st2_bytes = 0;
	} else if (modelHasSts(model)) {// fixme
		vbo->sts2_init = 1;
		st2_bytes = sizeof(vbo->sts2[0]) * model->vert_count;
	} else {
		// No sts2 available
		vbo->sts2_init = 1;
	}
	if (modelHasSts3(model))
		st3_bytes	= sizeof(vbo->sts3[0]) * model->vert_count;
	if (needBump)
	{
		tangent_bytes = sizeof(vbo->tangents[0]) * model->vert_count;
	}
	if (model->flags & OBJ_DRAWBONED)
	{
		BoneInfo	*boneinfo;
		if(!model->boneinfo) {
			model->boneinfo = assignDummyBoneInfo(model->name);
		}
		boneinfo = model->boneinfo;
		weight_bytes = sizeof(boneinfo->weights[0]) * model->vert_count;
		matidx_bytes = sizeof(boneinfo->matidxs[0]) * model->vert_count;
	}

	vert_total = vert_bytes + norm_bytes + st1_bytes + st2_bytes + st3_bytes + tangent_bytes + weight_bytes + matidx_bytes;
	total = vert_total + tri_bytes + sizeof(U32) + model->tex_count * sizeof(TexID);
	data = rdrQueueAlloc(DRAWCMD_CREATEVBO,total);

	*((VBO**)data)	= vbo;
	vbo->tex_ids	= (TexID*) (data+4);
 	vbo->tris		= (int*) (vbo->tex_ids + model->tex_count);
	modelGetTris(vbo->tris, model);

	idx = total - vert_total;
	vbo->verts = (void *)(data+idx);
	modelGetVerts(vbo->verts, model);
	idx += vert_bytes;

	if (norm_bytes)
	{
		vbo->norms = (void *)(data+idx);
		idx += norm_bytes;
		modelGetNorms(vbo->norms, model);
	}
	if (st1_bytes)
	{
		vbo->sts = (void *)(data+idx);
		idx += st1_bytes;
		modelGetSts(vbo->sts, model);
	}
	if (st2_bytes)
	{
		vbo->sts2 = (void *)(data+idx);
		memcpy(vbo->sts2,vbo->sts,model->vert_count * sizeof(Vec2));
		idx += st2_bytes;
	}
	if (st3_bytes)
	{
		vbo->sts3 = (void *)(data+idx);
		idx += st3_bytes;
		modelGetSts3(vbo->sts3, model);
	}
	if (weight_bytes || matidx_bytes)
	{
		int		i;

		vbo->weights = (void *)(data+idx);
		idx += weight_bytes;
		vbo->matidxs = (void *)(data+idx);
		idx += matidx_bytes;
		if (modelHasMatidxs(model))
		{
			static U8 *matidxs=0,*weights=0;
			static int maxvcount=-1;

			if (maxvcount < model->vert_count)
			{
				SAFE_FREE(matidxs);
				SAFE_FREE(weights);
				maxvcount = pow2(model->vert_count);
                weights = malloc(maxvcount);
				matidxs = malloc(2 * maxvcount);
			}

			modelGetWeights(weights, model);
			modelGetMatidxs(matidxs, model);

			for(i=0;i<model->vert_count;i++)
			{
				vbo->matidxs[i][0] = matidxs[i*2+0];
				vbo->matidxs[i][1] = matidxs[i*2+1];
				vbo->weights[i][0] = weights[i] * 1.f / 255.f;
				vbo->weights[i][1] = 1.f - vbo->weights[i][0];
			}
		}
		else
		{
			for(i=0;i<model->vert_count;i++)
			{
				vbo->matidxs[i][0] = 0;
				vbo->matidxs[i][1] = 0;
				vbo->weights[i][0] = 1;
				vbo->weights[i][1] = 0;
			}
		}
	}
	if (tangent_bytes)
	{
		vbo->tangents = (void *)(data+idx);
		idx += tangent_bytes;
		bumpInitObj(model);
	}
	modelFixup(model);

	vbo->bump_init			= needBump;
	vbo->vert_buffer_bytes	= vert_total;
	vbo->vert_count			= model->vert_count;
	vbo->tri_count			= model->tri_count;
	vbo->tex_count			= model->tex_count;
	vbo->flags				= model->flags;
	memcpy(vbo->tex_ids,model->tex_idx,sizeof(TexID) * vbo->tex_count);

	delta = (int)vbo->verts;
	if (!rdr_caps.use_vbos || useVbos == VBOS_DONTUSE)
	{
		vbo->tris = memcpy(malloc(vbo->tri_count * 3 * sizeof(int)),vbo->tris,vbo->tri_count * 3 * sizeof(int));
		vbo->verts = memcpy(malloc(vbo->vert_buffer_bytes),vbo->verts,vbo->vert_buffer_bytes);
		vbo->tex_ids = memcpy(malloc(vbo->tex_count * sizeof(TexID)),vbo->tex_ids,vbo->tex_count * sizeof(TexID));
	}
	else
	{
		vbo->verts			= 0;
		vbo->tris			= 0;
	}
	delta -= (int)vbo->verts;

	if (vbo->norms)
		vbo->norms		= (void *)((int)vbo->norms - delta);
	if (vbo->sts)
		vbo->sts		= (void *)((int)vbo->sts - delta);
	if (vbo->sts2)
		vbo->sts2		= (void *)((int)vbo->sts2 - delta);
	if (vbo->sts3)
		vbo->sts3		= (void *)((int)vbo->sts3 - delta);
	if (vbo->tangents)
		vbo->tangents	= (void *)((int)vbo->tangents - delta);
	if (vbo->weights)
		vbo->weights	= (void *)((int)vbo->weights - delta);
	if (vbo->matidxs)
		vbo->matidxs	= (void *)((int)vbo->matidxs - delta);
	if (!vbo->verts)
		rdrQueueSend();
	else
		rdrQueueCancel();
	RDRSTAT_ADD(vert_access_count,model->vert_count);
}



