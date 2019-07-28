#define RT_PRIVATE
#include "model.h"
#include "ogl.h"
#include <stdio.h>
#include "gfxwindow.h"
#include "memcheck.h" 
#include "render.h"
#include "camera.h"
#include "rendershadow.h"
#include "rendershadowmap.h"
#include "wcw_statemgmt.h"
#include "assert.h"
#include "cmdgame.h"
#include "model.h"
#include "tex.h"
#include "model_cache.h"
#include "rt_model_cache.h"
#include "gridcoll.h"
#include "font.h"
#include "cmdgame.h"
#include "fxutil.h"
#include "shadersATI.h"
#include "bump.h"
#include "gfx.h"
#include "gfxDebug.h"

#include "rendercloth.h"
#include "Cloth.h"
#include "ClothCollide.h"
#include "ClothMesh.h"
#include "clothnode.h"
#include "rt_tex.h"
#include "rt_tricks.h"
#include "rt_stats.h"
#include "ClothPrivate.h"


#if CLOTH_HACK

// ##########################################################################

int clothObjectsDrawn = 0;

static void render_mesh(ClothMesh *mesh)
{
	texSetWhite(TEXLAYER_BASE);
	texSetWhite(TEXLAYER_GENERIC);

	//glColorPointer(4,GL_UNSIGNED_BYTE,0,mesh->colors); 
	texBindTexCoordPointer( TEXLAYER_BASE, mesh->TexCoords ); 
	texBindTexCoordPointer( TEXLAYER_GENERIC, mesh->TexCoords ); 
	WCW_VertexPointer(3,GL_FLOAT,0, mesh->Points );
	WCW_NormalPointer(GL_FLOAT,0, mesh->Normals );

	WCW_PrepareToDraw();

	{
		int s;
		for (s=0; s<mesh->NumStrips; s++)
		{
			ClothStrip *strip = mesh->Strips + s;
			GLenum type =
				strip->Type == CLOTHMESH_TRISTRIP ? GL_TRIANGLE_STRIP :
				strip->Type == CLOTHMESH_TRIFAN ? GL_TRIANGLE_FAN :
				GL_TRIANGLES;
			glDrawElements(type, strip->NumIndices, GL_UNSIGNED_SHORT, strip->IndicesCCW ); CHECKGL;
			RT_STAT_DRAW_TRIS((type == GL_TRIANGLE_STRIP || type == GL_TRIANGLE_FAN) ? strip->NumIndices - 2 : strip->NumIndices / 3)
		}
	}
}

static void render_points(Mat4 viewspace, ClothMesh *mesh, RdrCloth *rc, int flags)
{
	int i;
	ClothMesh *pointmesh = ClothMeshCreate();

	modelDrawState(DRAWMODE_COLORONLY, ONLY_IF_NOT_ALREADY);
	
	ClothMeshPrimitiveCreateSphere(pointmesh, .05f, 0);

	for (i=0; i<mesh->NumPoints; i++)
	{
		Mat4 pmat,tmat;
		Mat44 m4;

		if (i >= rc->cloth_debug->commonData.NumParticles)
		{
			if (!(rc->Flags & CLOTH_FLAG_DEBUG_IM_POINTS))
				break;
			WCW_Color4(255,255,0,255);
		}
		else
		{
			if (rc->cloth_debug->ColAmt[i] > 0.0f) {
				WCW_Color4(255,0,0,255);
			} else {
				WCW_Color4(0,255,0,255);
			}
		}
		copyMat4(unitmat, pmat);
		copyVec3(mesh->Points[i], pmat[3]);
		mulMat4( viewspace, pmat, tmat ); 

		mat43to44( tmat, m4 );   
		addVec3(m4[3], rc->PositionOffset, m4[3]);
		WCW_LoadModelViewMatrixM44( m4 );  

		render_mesh(pointmesh);
	}
	
	ClothMeshDelete(pointmesh);
}

void modelDrawClothObjectDebug( RdrCloth *rc)
{
	int			i;
	int			nump;
	int			size_positions;
	ClothMesh	*mesh = rc->renderData.currentMesh;
	ClothRenderData	*renderData = &rc->renderData;
	
	// Copy dynamic data into correct buffers
	size_positions = sizeof(Vec3)*renderData->commonData.NumParticles;
	memcpy(renderData->RenderPos, ((char*)rc) + sizeof(RdrCloth), size_positions);
	if (renderData->hookNormals)
		renderData->hookNormals = (Vec3*)(((char*)rc) + sizeof(RdrCloth) + size_positions);

	// Interpolate points, update drawing info
	ClothUpdateDraw(&rc->renderData);
	nump = ClothNumRenderedParticles(&renderData->commonData, renderData->CurSubLOD);
	ClothMeshSetPoints(renderData->currentMesh, nump, renderData->RenderPos, renderData->Normals, renderData->TexCoords, renderData->BiNormals, renderData->Tangents);


	clothObjectsDrawn++;   
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), ONLY_IF_NOT_ALREADY);
	modelBindDefaultBuffer();
	modelSetAlpha(255);
	WCW_SunLightDir();
	WCW_Light(GL_LIGHT0, GL_AMBIENT, rc->ambient);  
	WCW_Light(GL_LIGHT0, GL_DIFFUSE, rc->diffuse); 
	// Render Cloth

	if (!(rc->Flags & CLOTH_FLAG_DEBUG_POINTS))
	{
		Mat44 m4;
		modelDrawState(DRAWMODE_DUALTEX_NORMALS, ONLY_IF_NOT_ALREADY);

		mat43to44( rc->viewspace, m4 );   
		addVec3(m4[3], rc->PositionOffset, m4[3]);
		WCW_LoadModelViewMatrixM44( m4 );  

		WCW_Color4(255,255,255,255);
		glCullFace( GL_FRONT ); CHECKGL;
		render_mesh(mesh);
		// Need to reverse lighting here
		glCullFace( GL_BACK ); CHECKGL;
		{ // Use different texture on the back
			TextureOverride temp = mesh->TextureData[0];
			mesh->TextureData[0] = mesh->TextureData[1];
			render_mesh(mesh);
			mesh->TextureData[0] = temp;
		}
	}

	if (rc->Flags & CLOTH_FLAG_DEBUG_POINTS)
	{
		render_points(rc->viewspace, mesh, rc, rc->Flags);
	}

	if (rc->Flags & CLOTH_FLAG_DEBUG_TANGENTS)
	{
		modelDrawState(DRAWMODE_COLORONLY, ONLY_IF_NOT_ALREADY);
		WCW_PrepareToDraw();
		for (i=0; i<mesh->NumPoints; i++)
		{
			Mat4 pmat,tmat;
			Mat44 m4;
			if (i >= rc->cloth_debug->commonData.NumParticles)
			{
				if (!(rc->Flags & CLOTH_FLAG_DEBUG_IM_POINTS) && (rc->Flags & CLOTH_FLAG_DEBUG_POINTS))
					break;
			}
			copyMat4(unitmat, pmat);
			copyVec3(mesh->Points[i], pmat[3]);
			mulMat4( rc->viewspace, pmat, tmat ); 

			mat43to44( tmat, m4 );   
			addVec3(m4[3], rc->PositionOffset, m4[3]);
			WCW_LoadModelViewMatrixM44( m4 );  

#define SCALE 1.0
			WCW_Color4(127,127,255,255);
			glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(mesh->Tangents[i][0]*SCALE,mesh->Tangents[i][1]*SCALE,mesh->Tangents[i][2]*SCALE);
			glEnd(); CHECKGL;
			RT_STAT_DRAW_TRIS(1)

			WCW_Color4(127,255,255,255);
			glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(mesh->BiNormals[i][0]*SCALE,mesh->BiNormals[i][1]*SCALE,mesh->BiNormals[i][2]*SCALE);
			glEnd(); CHECKGL;
			RT_STAT_DRAW_TRIS(1)
		}
	}

	if (rc->Flags & CLOTH_FLAG_DEBUG_NORMALS)
	{
		modelDrawState(DRAWMODE_COLORONLY, ONLY_IF_NOT_ALREADY);
		WCW_PrepareToDraw();
		for (i=0; i<mesh->NumPoints; i++)
		{
			Mat44 m4;
			if (i >= rc->cloth_debug->commonData.NumParticles)
			{
				if (!(rc->Flags & CLOTH_FLAG_DEBUG_IM_POINTS) && (rc->Flags & CLOTH_FLAG_DEBUG_POINTS))
					break;
			}

			mat43to44( rc->viewspace, m4); //tmat, m4 );   
			addVec3(m4[3], rc->PositionOffset, m4[3]);
			WCW_LoadModelViewMatrixM44( m4 );  

			WCW_Color4(200,100,200,255);
			glBegin(GL_LINES);
			//glVertex3f(0,0,0);
			//glVertex3f(mesh->Normals[i][0]*SCALE,mesh->Normals[i][1]*SCALE,mesh->Normals[i][2]*SCALE);
			glVertex3f(mesh->Points[i][0],mesh->Points[i][1],mesh->Points[i][2]);
			glVertex3f(mesh->Points[i][0] + mesh->Normals[i][0]*SCALE,
				mesh->Points[i][1] + mesh->Normals[i][1]*SCALE,
				mesh->Points[i][2] + mesh->Normals[i][2]*SCALE);
			glEnd(); CHECKGL;
			RT_STAT_DRAW_TRIS(1)
		}
	}


	
	if (rc->Flags & CLOTH_FLAG_DEBUG_COLLISION)
	{
		modelDrawState(DRAWMODE_DUALTEX_NORMALS, ONLY_IF_NOT_ALREADY);

		texSetWhite(TEXLAYER_BASE);
		texSetWhite(TEXLAYER_GENERIC);
		WCW_Color4(127,127,127,127);
		// Render Collision Objects
		glCullFace( GL_FRONT ); CHECKGL;
		for (i=0; i<rc->cloth_debug->NumCollidables; i++)
		{
			ClothCol *clothcol;
		
			clothcol = ClothGetCollidable(rc->cloth_debug, i);
			mesh = clothcol->Mesh;

			if (mesh)
			{
				Mat4 colmat, tmat;
				Mat44 m4;

				ClothColGetMatrix(clothcol, colmat);
				mulMat4( rc->viewspace, colmat, tmat ); 

				mat43to44( tmat, m4 );   
				addVec3(m4[3], rc->PositionOffset, m4[3]);
				WCW_LoadModelViewMatrixM44( m4 );  
		
				render_mesh(mesh);
			}
		}
	}

	if (rc->Flags & CLOTH_FLAG_DEBUG_HARNESS && rc->clothobj_debug->debugHookPos && rc->clothobj_debug->debugHookNormal)
	{
		modelDrawState(DRAWMODE_COLORONLY, ONLY_IF_NOT_ALREADY);
		WCW_PrepareToDraw();
		for (i=0; i<7; i++)
		{
			Mat4 pmat,tmat;
			Mat44 m4;

			copyMat4(unitmat, pmat);
			copyVec3(rc->clothobj_debug->debugHookPos[i], pmat[3]);
			mulMat4( rc->viewspace, pmat, tmat ); 

			mat43to44( tmat, m4 );   
			addVec3(m4[3], rc->PositionOffset, m4[3]);
			WCW_LoadModelViewMatrixM44( m4 );  

#undef SCALE
#define SCALE 0.7
			WCW_Color4(0,0,0,255);
			glBegin(GL_LINES);
			glVertex3f(0,0,0);
			glVertex3f(rc->clothobj_debug->debugHookNormal[i][0]*SCALE,rc->clothobj_debug->debugHookNormal[i][1]*SCALE,rc->clothobj_debug->debugHookNormal[i][2]*SCALE);
			glEnd(); CHECKGL;
			RT_STAT_DRAW_TRIS(1)
		}
	}

	glCullFace( GL_BACK ); CHECKGL;

	//WCW_DepthMask(1);
	//WCW_EnableGL_LIGHTING(1);

	WCW_Color4(255,255,255,255);
	
	WCW_LoadModelViewMatrixIdentity();
}


void modelDrawClothObject(GfxNode * node, BlendModeType blend_mode)
{
	static RdrCloth	*rc;
	static int		max_size;
	int				i,size;
	ClothObject		*clothobj = node->clothobj;
	Cloth			*cloth = ClothObjGetLODCloth(clothobj, -1);
	ClothMesh		*mesh = ClothObjGetLODMesh(clothobj, -1, -1);
	int				size_hooks;
	int				size_positions;
	F32				cubemap_attenuation = 0.0f;

	if (clothobj->GameData->readycount != 0xffffffff)
		return;

	if (!mesh)
		return;

	size = sizeof(RdrCloth);
	size += (size_positions = sizeof(Vec3) * cloth->commonData.NumParticles);
	if (cloth->renderData.hookNormals)
		size += (size_hooks = sizeof(Vec3) * cloth->commonData.MaxAttachments);
	else
		size_hooks = 0;
	if (size > max_size)
	{
		max_size = size * 2;
		rc = realloc(rc,max_size);
	}
	rc->alpha			= (F32)node->alpha * (1.f/255.f);
	rc->Flags			= cloth->Flags;
	if (node->tricks) {
		rc->trick			= *node->tricks;
		rc->has_trick = 1;
	} else {
		rc->has_trick = 0;
	}
	rc->blend_mode		= blend_mode;
	rc->cloth_debug		= cloth;
	rc->clothobj_debug	= clothobj;
	// Position offset into "cape" space
	assert(node->viewspace);
	mulVecMat3(cloth->PositionOffset, node->viewspace, rc->PositionOffset);
	copyMat4(cam_info.viewmat, rc->viewspace);
	if (game_state.wireframe || game_state.gfxdebug) {
		if (game_state.wireframe)
			rc->Flags |= CLOTH_FLAG_DEBUG_POINTS|CLOTH_FLAG_DEBUG_IM_POINTS;
		if (game_state.wireframe!=2)
			rc->Flags |= CLOTH_FLAG_DEBUG_AND_RENDER;
		if (game_state.wireframe==3)
			rc->Flags |= CLOTH_FLAG_DEBUG_NORMALS;
		if (game_state.gfxdebug & GFXDEBUG_NORMALS)
			rc->Flags |= CLOTH_FLAG_DEBUG_NORMALS;
		if (game_state.gfxdebug & GFXDEBUG_TANGENTS)
			rc->Flags |= CLOTH_FLAG_DEBUG_TANGENTS;
		if (game_state.gfxdebug & GFXDEBUG_BINORMALS)
			rc->Flags |= CLOTH_FLAG_DEBUG_TANGENTS;
	}
	modelDrawGetCharacterLighting(node, rc->ambient, rc->diffuse, rc->lightdir);
	if (rc->has_trick && rc->trick.info && (rc->trick.info->model_flags & OBJ_FULLBRIGHT))
	{
		rc->ambient[0] = rc->ambient[1] = rc->ambient[2] = 0.25;
		rc->diffuse[0] = rc->diffuse[1] = rc->diffuse[2] = 0.0;
	}

	if (game_state.tintAlphaObjects) {
		static U8		tint_alpha[] = {255, 20, 20, 255};
		static U8		tint_opaque[] = {50, 255, 50, 255};
		if (node->alpha==255 && !(node->flags & GFXNODE_ALPHASORT)) {
			for (i=0; i<4; i++)
				memcpy(mesh->colors[i], tint_opaque, 4);
		} else {
			for (i=0; i<4; i++)
				memcpy(mesh->colors[i], tint_alpha, 4);
		}
	}

	// Copy RenderData
	rc->renderData = *(&cloth->renderData);
	rc->renderData.commonData = *(&cloth->commonData);
	rc->renderData.hookNormals = (Vec3*)(!!rc->renderData.hookNormals); // Just a bool saying they're coming or not
	// Copy copies of dynamic data (hookNormals, positions)
	memcpy(((char*)rc) + sizeof(RdrCloth), cloth->CurPos, size_positions);
	if (size_hooks) {
		memcpy(((char*)rc) + sizeof(RdrCloth) + size_positions, 
			cloth->renderData.hookNormals, sizeof(Vec3)*cloth->commonData.MaxAttachments);
	}
	rc->renderData.currentMesh = mesh;
	rc->renderData.CurSubLOD = clothobj->CurSubLOD;

	if (rc->Flags & (CLOTH_FLAG_DEBUG))
	{
		rdrQueueDebug(modelDrawClothObjectDebug,rc,size);
		//modelDrawClothObjectDebug(rc);
		if (!(rc->Flags & CLOTH_FLAG_DEBUG_AND_RENDER))
			return;
	}
	for(i=0;i<2;i++)
	{
		int j;
		TextureOverride *binds;
		bool bIsMultiTex;
		RdrTexList *texlist = rc->texlist;

		binds = &mesh->TextureData[i];
		assert(binds->base);

		bIsMultiTex = binds->base->texopt && binds->base->texopt->flags & TEXOPT_TREAT_AS_MULTITEX;

		if (rc->blend_mode.shader == BLENDMODE_MULTI) {
			// TEXTODO: Allow overriding
			if (bIsMultiTex) {
				for (j=0; j<TEXLAYER_MAX_LAYERS; j++) 
					texlist[i].texid[j] = texDemandLoad(binds->base->tex_layers[j]);
			} else {
				// Colorblenddual/bumpmap_colorblenddual rendered as Multi9
				texlist[i].texid[TEXLAYER_BASE] = texDemandLoad(binds->base->tex_layers[TEXLAYER_BASE]);
				texlist[i].texid[TEXLAYER_MULTIPLY1] = white_tex->id;
				texlist[i].texid[TEXLAYER_BUMPMAP1] = texDemandLoad(binds->base->tex_layers[TEXLAYER_BUMPMAP1]);
				texlist[i].texid[TEXLAYER_DUALCOLOR1] = texDemandLoad(binds->generic);
				for (j=TEXLAYER_MASK; j<TEXLAYER_MAX_LAYERS; j++) 
					texlist[i].texid[j] = white_tex->id;
				texlist[i].texid[TEXLAYER_ADDGLOW1] = black_tex->id;
			}
			if (binds->base->tex_layers[TEXLAYER_BUMPMAP1])
				texlist[i].gloss1 = binds->base->tex_layers[TEXLAYER_BUMPMAP1]->gloss;
			else
				texlist[i].texid[TEXLAYER_BUMPMAP1] = dummy_bump_tex->id;
			if (binds->base->tex_layers[TEXLAYER_BUMPMAP2])
				texlist[i].gloss2 = binds->base->tex_layers[TEXLAYER_BUMPMAP2]->gloss;
			else
				texlist[i].texid[TEXLAYER_BUMPMAP2] = dummy_bump_tex->id;
		} else {
			// Colorblenddual/bumpmap_colorblenddual
			texlist[i].texid[TEXLAYER_BASE] = texDemandLoad(binds->base->tex_layers[TEXLAYER_BASE]);
			texlist[i].texid[TEXLAYER_GENERIC] = texDemandLoad(bIsMultiTex?binds->base->tex_layers[TEXLAYER_GENERIC] : binds->generic);
			texlist[i].texid[TEXLAYER_BUMPMAP1] = texDemandLoad(binds->base->tex_layers[TEXLAYER_BUMPMAP1]);
			if (blendModeHasBump(rc->blend_mode))
			{
				if (binds->base->tex_layers[TEXLAYER_BUMPMAP1])
					texlist[i].gloss1 = binds->base->tex_layers[TEXLAYER_BUMPMAP1]->gloss;
				else
					texlist[i].texid[TEXLAYER_BUMPMAP1] = dummy_bump_tex->id;
				if (binds->base->tex_layers[TEXLAYER_BUMPMAP2])
					texlist[i].gloss2 = binds->base->tex_layers[TEXLAYER_BUMPMAP2]->gloss;
				else
					texlist[i].texid[TEXLAYER_BUMPMAP2] = dummy_bump_tex->id;
			}
		}

		if (game_state.texoff)
			for (j=0; j<TEXLAYER_MAX_LAYERS; j++) 
				if (texlist[i].texid[j])
					texlist[i].texid[j] = (j == TEXLAYER_BUMPMAP1 || j == TEXLAYER_BUMPMAP2)?dummy_bump_tex->id:white_tex->id;

		texlist[i].scrollsScales = binds->base->scrollsScales;
	}

	rc->cubemap_attenuation = cubemap_attenuation;

	rdrQueue(DRAWCMD_CLOTH,rc,size);
}


#endif
