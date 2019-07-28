#define RT_PRIVATE
#include "ogl.h"
#include "wcw_statemgmt.h"
#include "bump.h"
#include "shadersAti.h"
#include "rendermodel.h"
#include "utils.h"
#include "error.h"
#include "rt_bonedmodel.h"
#include "rt_model_cache.h"
#include "rt_tex.h"
#include "rt_tricks.h"
#include "model_cache.h"
#include "rt_stats.h"
#include "mathutil.h"
#include "assert.h"
#include "rt_shadow.h"
#include "cmdgame.h"
#include "tex.h"
#include "rt_cubemap.h"
#include "rt_shadowmap.h"
#include "renderWater.h"	// for kReflectProjClip_All, etc.
#include "gfxDebug.h"

#define MAX_VERTS 6000

// Even checking this slows things down
//#define SOFT_NORMALIZE (rdr_frame_state.gfxdebug & GFXDEBUG_VERTINFO)

#define WEIGHTIT(inparray, outparray, matType)	{						\
		mulVecMat##matType(inparray[vertidx], bonemats[ matidx0 ], vec1);\
		scaleVec3(vec1, vbo->weights[vertidx][0], vec1);				\
		copyVec3(vec1, outparray[vertidx]);								\
																		\
		mulVecMat##matType(inparray[vertidx], bonemats[ matidx1 ], vec1);		\
		scaleVec3(vec1, vbo->weights[vertidx][1], vec1);				\
		addVec3(vec1, outparray[vertidx], outparray[vertidx]);			\
/*		if (SOFT_NORMALIZE && matType == 3)							*/	\
/*			normalVec3(outparray[vertidx]);							*/	\
	}


void DeformObject(Mat4 bonemats[], VBO * vbo, int vert_count, Vec3 * weightedverts, Vec3 * weightednorms, Vec3 * weightedtangents)
{
	int			vertidx, matidx0, matidx1, bump = 0;
	Vec3		vec1, *verts,*norms;
	Vec4		*tangents;

	verts		= vbo->verts;
	norms		= vbo->norms;
	tangents	= vbo->tangents;

	if (tangents && weightedtangents)
		bump = 1;
	for(vertidx = 0; vertidx < vert_count; vertidx++) //for each vert
	{
		matidx0 = vbo->matidxs[vertidx][0]/3;
		matidx1 = vbo->matidxs[vertidx][1]/3;

		WEIGHTIT(verts, weightedverts, 4);
		WEIGHTIT(norms, weightednorms, 3);
		if (bump)
		{
			WEIGHTIT(tangents, weightedtangents, 3);
			// note: binormal is calculated on the fly as needed in shaders and code
		}
	}
}

static INLINEDBG int isBorder(VBO *vbo, int a, int b)
{
	int		i,count,edges=0;

	count = vbo->tri_count * 3;
	for (i = 0; i < count; i += 3)
	{
		if (vbo->tris[i] == a || vbo->tris[i+1] == a || vbo->tris[i+2] == a)
		{
			if (vbo->tris[i] == b || vbo->tris[i+1] == b || vbo->tris[i+2] == b)
				edges++;
		}
	}

	return edges == 1;
}

static INLINEDBG void drawWireframeSeam(VBO *vbo, Vec3 * verts, int a, int b)
{
	F32		*v;

	if (isBorder(vbo, vbo->tris[a], vbo->tris[b])){ 
		WCW_Color4(100,255,128,255);
	} else {
		WCW_Color4(100,128,255,255);
	}

	glBegin(GL_LINES);
	v = verts[vbo->tris[a]];
	glVertex3f(v[0],v[1],v[2]);

	v = verts[vbo->tris[b]];
	glVertex3f(v[0],v[1],v[2]);
	glEnd(); CHECKGL;
}

static void modelDrawBonedWireframeSeams( VBO *vbo, Vec3 * verts )
{
	int		i,count;
	
	modelDrawState(DRAWMODE_COLORONLY, FORCE_SET);
	modelBlendState(BlendMode(BLENDMODE_MODULATE, 0), ONLY_IF_NOT_ALREADY);
	WCW_PrepareToDraw();

	count = vbo->tri_count * 3;
	for(i=0;i<count;i+=3)
	{
		drawWireframeSeam(vbo, verts, i, i+1);
		drawWireframeSeam(vbo, verts, i+1, i+2);
		drawWireframeSeam(vbo, verts, i+2, i);
		RT_STAT_DRAW_TRIS(3)
	}
}


/*
	Not thread-safe
#include "gfxwindow.h"
#include "sprite_text.h"
#include "StashTable.h"
static void modelDrawVertInfo( VBO *vbo, Vec3 * verts, Vec3 *norms, Vec3 *tangents, Vec3 *binormals, Vec2 *sts )
{
	int		i;
	char buf[2048];
	F32 yinc=12;
	int r = rand()%8;
	StashTable htVerts = stashTableCreateWithStringKeys(vbo->vert_count*2, FullyAutomatic|OverWriteExistingValue);
#define VERT_FORMAT "{%1.4f, %1.4f, %1.4f}"

	modelDrawState( DRAWMODE_COLORONLY, FORCE_SET );
	modelBlendState(BLENDMODE_MULTIPLY, ONLY_IF_NOT_ALREADY);

	WCW_Color4(r&1?255:0,r&2?255:0,r&4?255:0,255);

//	glBegin(GL_POINTS);
	for (i=0; i<vbo->vert_count; i++) {
		Vec2 screenPos;
		F32 y, x;
		int cnt=0;
//		glVertex3fv(verts[i]);
		gfxWindowScreenPos(verts[i], screenPos);
		y = rdr_frame_state.height - screenPos[1];
		x = screenPos[0];
		if (y<0 || y>rdr_frame_state.height || x<0 || x>rdr_frame_state.width)
			continue;

		sprintf(buf, VERT_FORMAT, verts[i][0], verts[i][1], verts[i][2]);
		if (cnt=(int)stashFindPointerReturnPointer(htVerts, buf)) {
			y += yinc*cnt*2+cnt*yinc/2;
		}
		stashAddInt(htVerts, buf, cnt+1);

		y+=yinc;
		sprintf(buf, "Vert %d:%s", i, cnt==0?"":cnt==1?"*":cnt==2?"**":cnt==3?"***":"****");
		prnt(x, y, verts[i][2], 1.0f, 1.0f, buf);
//		y+=yinc;
//		sprintf(buf, "P: " VERT_FORMAT, verts[i][0], verts[i][1], verts[i][2]);
//		prnt(x, y, verts[i][2], 1.0f, 1.0f, buf);
//		y+=yinc;
//		sprintf(buf, "N: " VERT_FORMAT, norms[i][0], norms[i][1], norms[i][2]);
//		prnt(x, y, verts[i][2], 1.0f, 1.0f, buf);
//		y+=yinc;
//		sprintf(buf, "T: " VERT_FORMAT, tangents[i][0], tangents[i][1], tangents[i][2]);
//		prnt(x, y, verts[i][2], 1.0f, 1.0f, buf);
//		y+=yinc;
//		sprintf(buf, "B: " VERT_FORMAT, binormals[i][0], binormals[i][1], binormals[i][2]);
//		prnt(x, y, verts[i][2], 1.0f, 1.0f, buf);
		y+=yinc;
		sprintf(buf, "ST: {%1.3f,%1.3f}", sts[i][0], sts[i][1]);
		prnt(x, y, verts[i][2], 1.0f, 1.0f, buf);
	}
//	glEnd(); CHECKGL;
	WCW_Color4(255,255,255,255);
	stashTableDestroy(htVerts);
}
*/

typedef struct Vec3dynarray {
	Vec3 *verts;
	int max_size;
} Vec3dynarray;

void fitVec3dynarray(Vec3dynarray *v3da, int size)
{
	dynArrayFit((void**)&v3da->verts, sizeof(Vec3), &v3da->max_size, size);
}

void loadBoneMatrices(SkinModel *skin)
{
	int		j,k;
	
	static const size_t kNumFloatsPerVec4 = 4;
	static const int kNumFloat4sPerMatrix = 3;
	
	GLuint tempVec4ArrNumItems = ( skin->bone_count * kNumFloat4sPerMatrix );
	size_t nTempArrayBytes =  ( tempVec4ArrNumItems * sizeof(Vec4) );
	float* tempVec4Arr = (float*)_alloca( nTempArrayBytes );
	float* pDestArrLoc = tempVec4Arr;
	
	for(j=0;j<skin->bone_count;j++)
	{
		Mat4Ptr	mp = skin->bone_mats[j];
		for (k = 0; k < kNumFloat4sPerMatrix; ++k)     
		{
			Vec4	col;
			int nIndex = (16 + (( j * 3 ) + k ));

			col[0] = mp[0][k];    
			col[1] = mp[1][k]; 
			col[2] = mp[2][k];
			col[3] = mp[3][k];
			
			//WCW_SetCgShaderParamArray4fv(kShaderPgmType_VERTEX, kShaderParam_BoneMatrixArrVP, j * 3 + k, col );
			copyVec4( col, pDestArrLoc );
			pDestArrLoc += kNumFloatsPerVec4;
		}
	}
	
	assert( (unsigned int)pDestArrLoc == ( (unsigned int)tempVec4Arr + nTempArrayBytes ) );
	WCW_SetCgShaderParamArray4fv(kShaderPgmType_VERTEX, kShaderParam_BoneMatrixArrVP, tempVec4Arr, tempVec4ArrNumItems );
}

void setupGeo(SkinModel *skin,VertexBufferObject *vbo)
{
	if ( !rdr_caps.use_vertshaders)
	{
		static Vec3dynarray	weightedverts,weightednorms,weightedtangents;
		VBO	*vbo = skin->vbo;

		WCW_LoadModelViewMatrixIdentity();
		modelDrawState(DRAWMODE_DUALTEX_NORMALS, ONLY_IF_NOT_ALREADY);
		modelBlendState(BlendMode(BLENDMODE_COLORBLEND_DUAL, 0), ONLY_IF_NOT_ALREADY);
		fitVec3dynarray(&weightedverts, MAX(vbo->vert_count, 384));
		fitVec3dynarray(&weightednorms, MAX(vbo->vert_count, 384));
		fitVec3dynarray(&weightedtangents, MAX(vbo->vert_count, 384));
		DeformObject(skin->bone_mats, vbo, skin->vbo->vert_count, weightedverts.verts, weightednorms.verts, weightedtangents.verts);
		WCW_BindBuffer(GL_ARRAY_BUFFER_ARB, 0);
		WCW_NormalPointer(GL_FLOAT, 0, weightednorms.verts);
		WCW_VertexPointer(3,GL_FLOAT,0, weightedverts.verts);
	}
	else
	{
		WCW_NormalPointer(GL_FLOAT, 0, vbo->norms);
		WCW_VertexPointer(3,GL_FLOAT,0, vbo->verts);
		//Set Skinning Weights in Vert Shader
		glVertexAttribPointerARB(1, 2, GL_FLOAT, GL_FALSE, 0, vbo->weights); CHECKGL;
		glVertexAttribPointerARB(5, 2, GL_SHORT, GL_FALSE, 0, vbo->matidxs); CHECKGL;
	}
}

static INLINEDBG void setupCustomColors(SkinModel *skin)
{
	scaleVec3(skin->rgba[0],(1.f/255.f),constColor0);
	scaleVec3(skin->rgba[1],(1.f/255.f),constColor1);

	WCW_ConstantCombinerParamerterfv(0, constColor0);
	WCW_ConstantCombinerParamerterfv(1, constColor1);
}

static void setupVertexLighting(SkinModel *skin)
{
	WCW_LoadModelViewMatrixIdentity();

	skin->lightdir[3] = 0.0; //this means directional lighting, please (GL is so dumb)

	WCW_Light(GL_LIGHT0, GL_AMBIENT, skin->ambient);  
	WCW_Light(GL_LIGHT0, GL_DIFFUSE, skin->diffuse);  

	WCW_LightPosition(skin->lightdir, NULL); 
}

void drawLoopBoned(SkinModel *skin,RdrTexList *texlists,int tex_index)
{
	int					i,j,ele_base=0;
	VBO					*vbo = skin->vbo;

	for(i = 0 ; i < vbo->tex_count ; i++) 
	{
		if (tex_index==-1 || tex_index==i)
		{
			RdrTexList *texlist = &texlists[(tex_index==-1)?i:0];
				
			rdrBeginMarker("%s: TEX %d, model \"%s\"", __FUNCTION__, tex_index, ((skin->debug_model_backpointer && skin->debug_model_backpointer->name) ? skin->debug_model_backpointer->name:"NO_NAME") );

			// Vert shader inputs
			if ( rdr_caps.use_vertshaders)
			{
				ModelStateSetFlags	flags=ONLY_IF_NOT_ALREADY|((texlist->blend_mode.blend_bits&BMB_HIGH_QUALITY)?BIT_HIGH_QUALITY_VERTEX_PROGRAM:0);
				if (texlist->blend_mode.shader == BLENDMODE_BUMPMAP_COLORBLEND_DUAL)
				{
					modelDrawState( DRAWMODE_BUMPMAP_SKINNED, flags);

					glVertexAttribPointerARB(7, 4, GL_FLOAT, GL_FALSE, 0, vbo->tangents); CHECKGL;
					WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_LightDirVP, skin->lightdir );
				}
				else if (texlist->blend_mode.shader == BLENDMODE_MULTI)
				{
					modelDrawState( DRAWMODE_BUMPMAP_SKINNED_MULTITEX, flags);

					glVertexAttribPointerARB(7, 4, GL_FLOAT, GL_FALSE, 0, vbo->tangents); CHECKGL;
				}
				else if (texlist->blend_mode.shader == BLENDMODE_COLORBLEND_DUAL)
				{	
					skin->ambient[3] = 0;
					modelDrawState( DRAWMODE_HW_SKINNED, ONLY_IF_NOT_ALREADY);
					WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_AmbientParameterVP, skin->ambient ); 
					WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_DiffuseParameterVP, skin->diffuse ); 
					WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_LightDirVP, skin->lightdir );            
				}
				else
					assert(0);

				loadBoneMatrices(skin);
			} //End if vert_shaders

			// Pixel shader inputs
			modelBlendState( texlist->blend_mode, ONLY_IF_NOT_ALREADY);

			if (texlist->blend_mode.shader == BLENDMODE_BUMPMAP_COLORBLEND_DUAL)
			{
				texBind(TEXLAYER_BASE,texlist->texid[TEXLAYER_BASE]);
				texBind(TEXLAYER_GENERIC,texlist->texid[TEXLAYER_GENERIC]);
				texBind(TEXLAYER_BUMPMAP1,texlist->texid[TEXLAYER_BUMPMAP1]);
				if( texlist->blend_mode.blend_bits & BMB_CUBEMAP && texlist->texid[TEXLAYER_CUBEMAP])
					texBindCube(TEXLAYER_CUBEMAP, texlist->texid[TEXLAYER_CUBEMAP]);
				setupBumpPixelShader(skin->ambient, skin->diffuse, skin->lightdir, texlist, texlist->blend_mode, true, true, skin->cubemap_attenuation);
			} else if (texlist->blend_mode.shader == BLENDMODE_COLORBLEND_DUAL) {
				texBind(TEXLAYER_BASE,texlist->texid[TEXLAYER_BASE]);
				texBind(TEXLAYER_GENERIC,texlist->texid[TEXLAYER_GENERIC]);
			} else if (texlist->blend_mode.shader == BLENDMODE_MULTI) {
				for (j=0; j<rdr_caps.max_layer_texunits; j++)
				{
					if (excludedLayer(texlist->blend_mode, j))
						continue;

					if( j==TEXLAYER_CUBEMAP) {
						if( texlist->blend_mode.blend_bits&BMB_CUBEMAP && texlist->texid[j] )
						{
							assert(rdr_view_state.cubemapMode);
							texBindCube(TEXLAYER_CUBEMAP, texlist->texid[j]);
						}
					} else {
						texBind(j, texlist->texid[j]);
					}
				}
				setupBumpMultiVertShader(texlist, skin->lightdir);
				setupBumpMultiPixelShader(skin->ambient, skin->diffuse, skin->lightdir, texlist, texlist->blend_mode, false, true, true, 1.0f);
			}
			
			WCW_PrepareToDraw();
	
			glDrawElements(GL_TRIANGLES, vbo->tex_ids[i].count*3,GL_UNSIGNED_INT,&vbo->tris[ele_base]); CHECKGL;

			RT_STAT_DRAW_TRIS(vbo->tex_ids[i].count)
			rdrEndMarker();
		}
		ele_base += vbo->tex_ids[i].count*3;
	}
}

void drawLoopBonedWireframe(SkinModel *skin)
{
	VBO					*vbo = skin->vbo;

	// Vert shader inputs
	if ( rdr_caps.use_vertshaders) {
		Vec4 ambient = {1,1,1,0};
		Vec4 diffuse = {0,0,0,0};
		Vec4 lightdir = {0,-1,0,0};
		modelDrawState( DRAWMODE_HW_SKINNED, ONLY_IF_NOT_ALREADY);
		WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_AmbientParameterVP, ambient ); 
		WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_DiffuseParameterVP, diffuse ); 
		WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_LightDirVP, lightdir );            
		loadBoneMatrices(skin);
	} else {
		glDisable(GL_LIGHTING); CHECKGL;
	}

	// Pixel shader inputs
	modelBlendState(BlendMode(BLENDMODE_COLORBLEND_DUAL, 0), ONLY_IF_NOT_ALREADY);
	WCW_PrepareToDraw();
	texSetWhite(TEXLAYER_BASE);
	texSetWhite(TEXLAYER_GENERIC);

	glDisable(GL_CULL_FACE); CHECKGL;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); CHECKGL;
	glDrawElements(GL_TRIANGLES, vbo->tri_count*3,GL_UNSIGNED_INT,vbo->tris); CHECKGL;
	RT_STAT_DRAW_TRIS(vbo->tri_count)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); CHECKGL;
	glEnable(GL_CULL_FACE); CHECKGL;

	if ( !rdr_caps.use_vertshaders) {
		glEnable(GL_LIGHTING); CHECKGL;
	}

}

static void drawDebugWireframe(SkinModel *skin)
{
	static Vec3dynarray weightedverts,weightednorms,weightedtangents;
	VBO					*vbo = skin->vbo;

	if (rdr_caps.use_vbos)
	{
		static int doneonce=0;
		if (!doneonce && rdr_view_state.gfxdebug)
		{
			doneonce = 1;
			Errorf("Some gfx debug features require a '/novbos 1' command before they will work");
		}
		return;
	}
	fitVec3dynarray(&weightedverts, vbo->vert_count);
	fitVec3dynarray(&weightednorms, vbo->vert_count);
	fitVec3dynarray(&weightedtangents, vbo->vert_count);
 	DeformObject(skin->bone_mats,vbo,vbo->vert_count,weightedverts.verts,weightednorms.verts,weightedtangents.verts);

	WCW_LoadModelViewMatrixIdentity();
	if (rdr_view_state.gfxdebug & GFXDEBUG_SEAMS)
		modelDrawBonedWireframeSeams( vbo, weightedverts.verts );
	if (skin->flags & GFXNODE_DEBUGME || rdr_view_state.wireframe)
	{
		if( rdr_view_state.gfxdebug & (GFXDEBUG_NORMALS|GFXDEBUG_BINORMALS|GFXDEBUG_TANGENTS) )
		{
			modelDrawState( DRAWMODE_COLORONLY, FORCE_SET);
			modelBlendState(BlendMode(BLENDMODE_MODULATE,0), ONLY_IF_NOT_ALREADY);
		}
#ifndef FINAL
		modelDrawVertexBasisTBN( vbo, weightedverts.verts, weightednorms.verts, weightedtangents.verts );	// override VBO with skinned data
#endif
//		if (rdr_frame_state.gfxdebug & GFXDEBUG_VERTINFO && skin->flags & GFXNODE_DEBUGME)
//		{
//			modelDrawVertInfo(vbo, weightedverts.verts, weightednorms.verts, weightedtangents.verts, NULL, vbo->sts);
//		}
		WCW_Color4(255,255,255,255);
	}
}

void modelDrawBonedNodeDirect(SkinModel *skin)
{
	VBO			*vbo = skin->vbo;
	TrickNode	*trick;
	RdrTexList *texlist;
	int			tex_count=(skin->tex_index==-1)?vbo->tex_count:1;
	int			bRestoreProjection = false;

	RT_STAT_INC(bonemodel_drawn);
	RT_STAT_ADDMODEL(vbo);

	// Check the current projection matrix mode, can apply to the whole viewport
	// or just to individual entities (since it is only skinned entities that we
	// currently need special entities for)
	if ( game_state.reflection_proj_clip_mode == kReflectProjClip_EntitiesOnly )
	{
		if (!rdr_caps.supports_vp_clip_planes_nv && rdr_view_state.renderPass == RENDERPASS_REFLECTION )
		{
			glMatrixMode( GL_PROJECTION ); CHECKGL;
			glPushMatrix(); CHECKGL;
			glLoadMatrixf((F32 *)rdr_view_state.projectionClipMatrix); CHECKGL;
			glMatrixMode(GL_MODELVIEW); CHECKGL;
			bRestoreProjection = true;
		}
	}

	texlist = (RdrTexList*) (skin->bone_mats + skin->bone_count);

	if (skin->has_trick)
		trick = (TrickNode*) (texlist + tex_count);
	else
		trick = 0;
	skin->ambient[3] = 1.0;  
	skin->diffuse[3] = 1.0;  

	modelBindBuffer(skin->vbo); 

	WCW_TexLODBias(0, -2.0); 
	WCW_TexLODBias(1, -2.0);

	if( !(rdr_caps.features & GFXF_BUMPMAPS) ) //should I do full check for bumpmapped thing?
		setupVertexLighting(skin);

	//do this after setting GL light
	skin->lightdir[3] = 0.5; // this value is used in the vertex shader - don't change it!
   	constColor0[3] = (F32)skin->rgba[0][3] * (1.f/255.f); //current fade
	constColor1[3] = 1;

	//This is before CUSTOM COLORS because custom colors should take precedence
	gfxNodeTricks( trick, vbo, NULL, texlist[0].blend_mode, false ); 
	setupCustomColors(skin);

	if ( !rdr_caps.use_vertshaders )
		assert(texlist[0].blend_mode.shader == BLENDMODE_COLORBLEND_DUAL); // Only dual color blend mode allowed for non-ARBvp code
	setupGeo(skin,vbo);
	texBindTexCoordPointer( TEXLAYER_BASE,vbo->sts);
	texBindTexCoordPointer( TEXLAYER_GENERIC,vbo->sts2);

//	if (rdr_caps.use_vertshaders)
//		WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_LightDirVP, skin->lightdir );

	if (rdr_view_state.wireframe != 2)
	{
		if (rdr_view_state.wireframe || (rdr_view_state.gfxdebug & GFXDEBUG_SEAMS))
		{
			glEnable(GL_POLYGON_OFFSET_FILL); CHECKGL;
			glPolygonOffset(1,1); CHECKGL;
		}

		drawLoopBoned(skin,texlist,skin->tex_index);

		if (rdr_view_state.wireframe || (rdr_view_state.gfxdebug & GFXDEBUG_SEAMS)) {
			glDisable(GL_POLYGON_OFFSET_FILL); CHECKGL;
		}
	}

	if (rdr_view_state.wireframe && !(rdr_view_state.gfxdebug & GFXDEBUG_SEAMS))
		drawLoopBonedWireframe(skin);

	// done automatically:
	//if( skin->flags & (GFXNODE_CUSTOMCOLOR|GFXNODE_TINTCOLOR) && rdr_caps.chip == TEX_ENV_COMBINE)
	//	WCW_Color4(255,255,255,255);

/*
	if (rdr_caps.chip & R200) // RDRMAYBEFIX
		texSetWhite(TEXLAYER_BASE);
*/

	gfxNodeTricksUndo( trick, vbo, false );

	if (rdr_view_state.gfxdebug || rdr_view_state.wireframe == 3)
		drawDebugWireframe(skin);

	if (bRestoreProjection)
	{
		glMatrixMode( GL_PROJECTION ); CHECKGL;
		glPopMatrix(); CHECKGL;
		glMatrixMode(GL_MODELVIEW); CHECKGL;
	}
}
