#define RT_PRIVATE
#include "ogl.h"
#include "wcw_statemgmt.h"
#include "rt_cloth.h"
#include "rt_state.h"
#include "rt_model_cache.h"
#include "rt_tricks.h"
#include "rt_tex.h"
#include "rt_model.h"
#include "rt_stats.h"
#include "mathutil.h"
#include "assert.h"
#include "rt_shadow.h"

#include "clothnode.h"
#include "Cloth.h"
#include "ClothCollide.h"
#include "ClothMesh.h"
#include "ClothPrivate.h"

#include "cmdgame.h"
void modelDrawClothObjectDirect( RdrCloth *rc )
{
	ClothMesh		*mesh = rc->renderData.currentMesh;
	ClothRenderData	*renderData = &rc->renderData;
	Vec4			ambient;
	Vec4			diffuse;
	Vec4			lightdir;
	int				i, nump;
	int				size_positions;
	ModelStateSetFlags flags = ONLY_IF_NOT_ALREADY|((rc->blend_mode.blend_bits&BMB_HIGH_QUALITY)?BIT_HIGH_QUALITY_VERTEX_PROGRAM:0);
	TrickNode		*trick;

	if (!rc->Flags || (rc->Flags & CLOTH_FLAG_DEBUG_AND_RENDER)) {
		// Copy dynamic data into correct buffers
		size_positions = sizeof(Vec3)*renderData->commonData.NumParticles;
		memcpy(renderData->RenderPos, ((char*)rc) + sizeof(RdrCloth), size_positions);
		if (renderData->hookNormals)
			renderData->hookNormals = (Vec3*)(((char*)rc) + sizeof(RdrCloth) + size_positions);

		// Interpolate points, update drawing info
		ClothUpdateDraw(&rc->renderData);
		nump = ClothNumRenderedParticles(&renderData->commonData, renderData->CurSubLOD);
		ClothMeshSetPoints(renderData->currentMesh, nump, renderData->RenderPos, renderData->Normals, renderData->TexCoords, renderData->BiNormals, renderData->Tangents);
	}

	// Draw it!

	RT_STAT_INC(clothmodel_drawn);
	//TODO: RT_STAT_ADDMODEL(vbo);

	copyVec3(rc->ambient,ambient);
	copyVec3(rc->diffuse,diffuse);
	copyVec3(rc->lightdir,lightdir);
	lightdir[3]		= 0.0; //this means directional lighting, please (GL is so dumb)
	ambient[3]		= 1.0;
	diffuse[3]		= 1.0;
   	constColor0[3]	= rc->alpha;

	WCW_TexLODBias(0, -1.0); 
	WCW_TexLODBias(1, -1.0);

	//Set Tricks ( could change constColors ) 
	//This is over CUSTOM COLORS because custom colors should take precedence
	if (rc->has_trick)
		trick = &rc->trick;
	else
		trick = 0;

	gfxNodeTricks( trick, NULL, NULL, rc->blend_mode, false ); 

	if (rdr_view_state.wireframe)
	{
		//TODO: Cape wireframe
	}

	modelBindDefaultBuffer(); // NO VBOs in this function

	if ( !rdr_caps.use_vertshaders ) 
	{
		assert( rc->blend_mode.shader == BLENDMODE_COLORBLEND_DUAL);
	}

	modelBlendState(rc->blend_mode, ONLY_IF_NOT_ALREADY);

	if (rc->blend_mode.shader == BLENDMODE_COLORBLEND_DUAL) // No bumpmaps
	{
		modelDrawState(DRAWMODE_DUALTEX_NORMALS, ONLY_IF_NOT_ALREADY);

		setupFixedFunctionVertShader(trick);

		// Set world light (only for no vert shaders or no bumpmaps)
		ambient[3] = 1.0;  
		WCW_Light(GL_LIGHT0, GL_AMBIENT, ambient);  
		WCW_Light(GL_LIGHT0, GL_DIFFUSE, diffuse);  
	}
	else if (rc->blend_mode.shader == BLENDMODE_BUMPMAP_COLORBLEND_DUAL)
	{
		modelDrawState( DRAWMODE_BUMPMAP_DUALTEX, flags);

		// Set up binormals and tangents
		glVertexAttribPointerARB(6, 3, GL_FLOAT, GL_FALSE, 0, mesh->BiNormals); CHECKGL;
		glVertexAttribPointerARB(7, 3, GL_FLOAT, GL_FALSE, 0, mesh->Tangents); CHECKGL;
	} else if (rc->blend_mode.shader == BLENDMODE_MULTI)
	{
		modelDrawState( DRAWMODE_BUMPMAP_MULTITEX, flags);

		// Set up binormals and tangents
		glVertexAttribPointerARB(6, 3, GL_FLOAT, GL_FALSE, 0, mesh->BiNormals); CHECKGL;
		glVertexAttribPointerARB(7, 3, GL_FLOAT, GL_FALSE, 0, mesh->Tangents); CHECKGL;
	}

	WCW_NormalPointer(GL_FLOAT, 0, mesh->Normals);
	WCW_VertexPointer(3,GL_FLOAT,0, mesh->Points);
	texBindTexCoordPointer( TEXLAYER_BASE,mesh->TexCoords);
	texBindTexCoordPointer( TEXLAYER_GENERIC,mesh->TexCoords);

	for (i = 0; i<2; i++) {
		// All capes have custom colors
		constColor0[0] = mesh->colors[i*2+0][0] * (1.f/255.f);
		constColor0[1] = mesh->colors[i*2+0][1] * (1.f/255.f);
		constColor0[2] = mesh->colors[i*2+0][2] * (1.f/255.f);

		constColor1[0] = mesh->colors[i*2+1][0] * (1.f/255.f);
		constColor1[1] = mesh->colors[i*2+1][1] * (1.f/255.f);
		constColor1[2] = mesh->colors[i*2+1][2] * (1.f/255.f);

		WCW_ConstantCombinerParamerterfv(0, constColor0);
		WCW_ConstantCombinerParamerterfv(1, constColor1);

		if (i==0) {
			glCullFace( GL_FRONT ); CHECKGL; // Draws the "back" of the cape (the outer portion)
		} else {
			glCullFace( GL_BACK ); CHECKGL; // Draws the "front" of the cape (the inner portion)
		}

		if (i==1) { // Invert the lighting direction
			subVec3(zerovec3,lightdir,lightdir);
		}

		// Setup inputs to pixel/vertex shaders
		if (rc->blend_mode.shader == BLENDMODE_BUMPMAP_COLORBLEND_DUAL)
		{
			// Bind textures
			texBind(TEXLAYER_BASE, rc->texlist[i].texid[TEXLAYER_BASE]);
			texBind(TEXLAYER_GENERIC, rc->texlist[i].texid[TEXLAYER_GENERIC]);
			//Bind the bumpmapped texture
			if( rc->texlist[i].texid[TEXLAYER_BUMPMAP1])
				texBind(TEXLAYER_BUMPMAP1, rc->texlist[i].texid[TEXLAYER_BUMPMAP1] );
			else
				texBind(TEXLAYER_BUMPMAP1, rdr_view_state.dummy_bump_tex_id );

			// Set up inputs to pixel shader
			setupBumpPixelShader(ambient, diffuse, lightdir, &rc->texlist[i], rc->blend_mode, true, true, rc->cubemap_attenuation);

			// Set up inputs to Vertex shader
			WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_LightDirVP, lightdir );
		} else if (rc->blend_mode.shader == BLENDMODE_COLORBLEND_DUAL) {

			// Bind textures
			texBind(TEXLAYER_BASE, rc->texlist[i].texid[TEXLAYER_BASE]);
			texBind(TEXLAYER_GENERIC, rc->texlist[i].texid[TEXLAYER_GENERIC]);

			// Lightdir changes per side of cape
			WCW_LoadModelViewMatrixIdentity();
			WCW_LightPosition(lightdir, NULL); //(w == 0) == directional
		} else if (rc->blend_mode.shader == BLENDMODE_MULTI) {
			int j;
			// Bind textures
			for (j=0; j<rdr_caps.max_layer_texunits; j++) {
				int id = rc->texlist[i].texid[j];
				texBind(j, id?id:rdr_view_state.white_tex_id);
			}

			// Set up inputs to Vertex shader
			setupBumpMultiVertShader(&rc->texlist[i], lightdir);
			// Set up inputs to pixel shader
			setupBumpMultiPixelShader(ambient, diffuse, lightdir, &rc->texlist[i], rc->blend_mode, false, true, true, 1.0f);
		}

		// Load transformation matrix
		{
			Mat44 m4;
			mat43to44( rc->viewspace, m4 );
			addVec3(m4[3], rc->PositionOffset, m4[3]);
			WCW_LoadModelViewMatrixM44( m4 );
		}

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

	WCW_Color4(255,255,255,255);
	
//	glActiveTextureARB(GL_TEXTURE0_ARB); CHECKGL; // makes things work... 
//	texSetWhite(TEXLAYER_BUMPMAP1); //Super slow, makes ATI work State Manage
	gfxNodeTricksUndo(trick, NULL, false ); 

	WCW_LoadModelViewMatrixIdentity();

	//TODO: RT_STAT_FINISH_MODEL;
}

void freeClothNodeDirect(ClothObject *clothobj)
{
	extern int ClothNodeNum;
	ClothNodeData * nodedata = clothobj->GameData;
	if (nodedata)
	{
		switch(nodedata->type)
		{
		case CLOTH_TYPE_TAIL:
		case CLOTH_TYPE_CAPE:
			{
				ClothCapeData * capedata = (ClothCapeData *)nodedata;
				free(capedata);
				break;
			}
		default:
			{
				free(nodedata);
				break;
			}
		}
	}
	ClothObjectDelete(clothobj);
	ClothNodeNum--;
}
