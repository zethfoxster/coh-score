#define RT_PRIVATE
#include "timing.h"
#include "ogl.h"
#include "wcw_statemgmt.h" 
#include "rt_particles.h"
#include "rt_model_cache.h"
#include "mathutil.h"
#include "rt_state.h"
#include "rt_stats.h"
#include "cmdgame.h"
#include "particle.h" // for enums ONLY!

void rdrPrepareToRenderParticleSystemsDirect(F32 **texarray)
{
	rdrBeginMarker(__FUNCTION__);

	/*  TO DO : make color combiner work for things with alpha on them per vertex
	WCW_EnableGL_LIGHTING(0);
	glEnableClientState( GL_COLOR_ARRAY ); CHECKGL;
	WCW_EnableColorMaterial();     
		constColor0[0] = 1.0; 
		constColor0[1] = 1.0 ;   
		constColor0[2] = 1.0;
		constColor0[3] = 0.1;   
		constColor1[0] = 1;
		constColor1[1] = 1;
		constColor1[2] = 1;
		constColor1[3] = 1;
		WCW_ConstantCombinerParamerterfv(0, constColor0);
		WCW_ConstantCombinerParamerterfv(1, constColor1);
		*/

	modelDrawState(DRAWMODE_DUALTEX, FORCE_SET);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), FORCE_SET);
	glAlphaFunc(GL_GREATER,0 ); CHECKGL;

	glDisable(GL_CULL_FACE); CHECKGL;
	WCW_LoadModelViewMatrixIdentity();
	WCW_DepthMask(0);	
	glShadeModel(GL_FLAT); CHECKGL;

	//Make absolutely sure there are no errors in the statemanagement.
	WCW_ClearTextureStateToWhite(); //for ATIs weirdness

	modelBindDefaultBuffer();
	texBindTexCoordPointer(TEXLAYER_BASE, *texarray); // RDRMAYBEFIX - texarray should be created in the thread, but I was too lazy to move it out of particle.c since it's static
	texBindTexCoordPointer(TEXLAYER_GENERIC, *texarray);
	WCW_BindBuffer( GL_ARRAY_BUFFER_ARB, 0 );

	rdrEndMarker();
}

void rdrCleanUpAfterRenderingParticleSystemsDirect()
{
	rdrBeginMarker(__FUNCTION__);
	WCW_LoadTextureMatrixIdentity(0);
	WCW_LoadTextureMatrixIdentity(1);

	glMatrixMode(GL_MODELVIEW); CHECKGL;
	WCW_DepthMask(1);
	WCW_Fog(1);
	glEnable(GL_CULL_FACE); CHECKGL;
	glShadeModel(GL_SMOOTH); CHECKGL;

	modelBindDefaultBuffer();
	rdrEndMarker();
}

void modelDrawParticleSysDirect(RdrParticlePkg *pkg )
{
	extern int alpha_write_mask;
	F32 * verts;
	U8 * rgbas;
	Mat4 matrix;

	verts = (F32 *)(pkg + 1);
	rgbas = ((U8*)verts) + pkg->particle_count * sizeof(Vec3) * 4;

	rdrBeginMarker(__FUNCTION__ " : particles %d", pkg->particle_count);

PERFINFO_AUTO_START("startDrawing",1);

	if( pkg->has_systemMatCamSpace ) //PARTICLE_LOCALFACING
	{
		WCW_LoadModelViewMatrix( pkg->systemMatCamSpace );
	}
	else //PARTICLE_FRONTFACING
	{
		WCW_LoadModelViewMatrixIdentity();
	}

PERFINFO_AUTO_STOP_START("all pre stuff",1);

	//Set up verts and colors
	WCW_VertexPointer(3, GL_FLOAT, 0, verts); 
	glColorPointer(4,GL_UNSIGNED_BYTE, 0, rgbas); CHECKGL;

	if(pkg->blend_mode == PARTICLE_ADDITIVE) 
	{
		WCW_BlendFuncPush(GL_SRC_ALPHA, GL_ONE);
		WCW_Fog(0);
		if (alpha_write_mask) { // Don't write alpha 
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); CHECKGL;
		}
	}
	else if (pkg->blend_mode == PARTICLE_SUBTRACTIVE ||
			 pkg->blend_mode == PARTICLE_SUBTRACTIVE_INVERSE)
	{
		if (rdr_caps.supports_subtract) {
			glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT); CHECKGL;
			WCW_BlendFuncPush(GL_SRC_ALPHA, GL_ONE);
		} else {
			// this does not subtract, but fakes it by masking
			WCW_BlendFuncPush(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		}
		WCW_Fog(0);
		if (alpha_write_mask) { // Don't write alpha 
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); CHECKGL;
		}
	}
	else if (pkg->blend_mode == PARTICLE_PREMULTIPLIED_ALPHA)
	{
		WCW_BlendFuncPush(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		WCW_Fog(0);
		if (alpha_write_mask) { // Don't write alpha 
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); CHECKGL;
		}
	}
	else if (pkg->blend_mode == PARTICLE_MULTIPLY)
	{
		WCW_BlendFuncPush(GL_ZERO, GL_SRC_COLOR);
		WCW_Fog(0);
		if (alpha_write_mask) { // Don't write alpha 
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); CHECKGL;
		}
	}
	else
	{	
		WCW_BlendFuncPush(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		WCW_Fog(1);
	}
	

	//Set up Texture 1 and 2

	if( pkg->whiteParticles || game_state.texoff)
		texSetWhite(TEXLAYER_BASE);
	else
		texBind(TEXLAYER_BASE, pkg->tex[0].texid);
#if 1
	zeroMat4(matrix);
	matrix[0][0] = pkg->tex[0].texscale[0];
	matrix[1][1] = pkg->tex[0].texscale[1];
	matrix[3][0] = pkg->tex[0].textranslate[0];
	matrix[3][1] = pkg->tex[0].textranslate[1];
	WCW_LoadTextureMatrix(0, matrix);
#else
	WCW_ActiveTexture(0);
	glMatrixMode(GL_TEXTURE); CHECKGL;
	glLoadIdentity(); CHECKGL;
	glTranslatef(pkg->tex[0].textranslate[0], pkg->tex[0].textranslate[1], 0); CHECKGL;
	glScalef(pkg->tex[0].texscale[0], pkg->tex[0].texscale[1], 0); CHECKGL;
#endif

	if( pkg->whiteParticles || game_state.texoff)
		texSetWhite( TEXLAYER_GENERIC);
	else
		texBind(TEXLAYER_GENERIC, pkg->tex[1].texid);
#if 1
	matrix[0][0] = pkg->tex[1].texscale[0];
	matrix[1][1] = pkg->tex[1].texscale[1];
	matrix[3][0] = pkg->tex[1].textranslate[0];
	matrix[3][1] = pkg->tex[1].textranslate[1];
	WCW_LoadTextureMatrix(1, matrix);
#else
	WCW_ActiveTexture(1);
	glLoadIdentity(); CHECKGL;
	glTranslatef( pkg->tex[1].textranslate[0], pkg->tex[1].textranslate[1], 0	); CHECKGL;
	glScalef(pkg->tex[1].texscale[0], pkg->tex[1].texscale[1], 0); CHECKGL;
#endif

	glMatrixMode(GL_MODELVIEW); CHECKGL;

PERFINFO_AUTO_STOP_START("glDrawElements",1);

	WCW_PrepareToDraw();

	//Bind Triangles and Draw this system
	glDrawElements( GL_TRIANGLES, pkg->particle_count * 6, GL_UNSIGNED_INT, pkg->tris ); CHECKGL;
	RT_STAT_DRAW_TRIS(pkg->particle_count * 2)
	//glDrawArrays( GL_QUADS, 0, system->particle_count * 4); //NVidia recommended this, but it's a couple frames slower

PERFINFO_AUTO_STOP();
	if (pkg->blend_mode == PARTICLE_SUBTRACTIVE || 
		pkg->blend_mode == PARTICLE_SUBTRACTIVE_INVERSE)
	{
		if (rdr_caps.supports_subtract) {
			glBlendEquationEXT(GL_FUNC_ADD_EXT); CHECKGL;
		}
		if (alpha_write_mask) {
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); CHECKGL;
		}
	} else if (pkg->blend_mode == PARTICLE_ADDITIVE)
	{
		if (alpha_write_mask) {
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); CHECKGL;
		}
	} else if (pkg->blend_mode == PARTICLE_PREMULTIPLIED_ALPHA)
	{
		if (alpha_write_mask) {
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); CHECKGL;
		}
	} else if (pkg->blend_mode == PARTICLE_MULTIPLY)
	{
		if (alpha_write_mask) {
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); CHECKGL;
		}
	}

	WCW_BlendFuncPop();
	rdrEndMarker();
}

