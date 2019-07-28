#define RT_ALLOW_BINDTEXTURE
#include "rt_filter.h"
#include "rt_cgfx.h"
#include "rt_tune.h"
#include "rt_init.h"
#include "wcw_statemgmt.h"
#include "gfx.h"
#include "gfxSettings.h"
#include "mathutil.h"
#include "tiff.h"
#include "failtext.h"
#include <file.h>

#define RT_PRIVATE
#include "rt_pbuffer.h"

#define kFilter_EffectName "filter"

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;

typedef struct filterGlobals {
	// quality settings
	int blur;

	// blur buffer size
	GLsizei blurWidth;
	GLsizei blurHeight;

	// framebuffer, renderbuffer and texture objects
	int numFbo;
	int curFbo;
	GLuint pushFbo;
	GLuint fbo0;
	GLuint srcTex;
	GLuint auxTex1[MAX_SLI];
	GLuint auxTex2[MAX_SLI];

	GLuint srcTarget;	// e.g., GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, etc

	// CgFX handles
	CGcontext cgContext;
	CGeffect cgEffect;
	
	CGtechnique blurFast2d;
	CGtechnique blurAlphaFast2d;
	CGtechnique gaussian1d;
	CGtechnique gaussianDepth1d;
	CGtechnique bilateral1d;
	CGtechnique bilateralDepth1d;
	CGtechnique trilateral1d;

	// Cached results from cgGetNamedEffectParameter()
	CGparameter modelViewProjParam;
	CGparameter sigmaParam;
	CGparameter screenstepParam;

	CGparameter colorTexParam;
	CGparameter colorTexNPOTParam;
	CGparameter color2TexParam;
	CGparameter color2TexNPOTParam;

} filterGlobals;

static filterGlobals g_rt_filter;

static void drawQuadShader(CGtechnique technique, float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, float z)
{	
	CGpass pass = cgGetFirstPass(technique);
	CGprofile vertexProfile = cgGetProgramProfile(cgGetPassProgram(pass, CG_VERTEX_DOMAIN));
	CGprofile fragmentProfile = cgGetProgramProfile(cgGetPassProgram(pass, CG_FRAGMENT_DOMAIN));
	cgGLEnableProfile(vertexProfile);
	cgGLEnableProfile(fragmentProfile);
	rt_cgfxSetRenderPass(technique, pass);
	glBegin(GL_QUADS);
	glTexCoord2f(u1, v1);
	glVertex3f(x1, y1, z);
	glTexCoord2f(u1, v2);
	glVertex3f(x1, y2, z);
	glTexCoord2f(u2, v2);
	glVertex3f(x2, y2, z);
	glTexCoord2f(u2, v1);
	glVertex3f(x2, y1, z);
	glEnd(); CHECKGL;
	rt_cgfxUnsetRenderPass(technique, pass);
	cgGLDisableProfile(fragmentProfile);
	cgGLDisableProfile(vertexProfile);
}

static void drawFullscreenShader(CGtechnique technique)
{
	drawQuadShader(technique, -1, -1, 1, 1, 0, 0, 1, 1, 0);
}

static void reloadShaders()
{
	const char * args[] = {
		"-strict",
//		"-profileopts",
//		"MaxInstructions=2048,NumInstructionSlots=4096,NumMathInstructionSlots=4096,MaxLocalParams=1024,NumTemps=512",
//		getShaderOptimizationArg(),
		//(g_rt_filter.lastTexTarget == GL_TEXTURE_RECTANGLE_ARB) ? "-DUSE_TEX_RECT=1" : "-DUSE_TEX_RECT=0",
		NULL
	};

	g_rt_filter.cgEffect = rt_cgfxGetEffect( kFilter_EffectName, args, true );
	g_rt_filter.blurFast2d = rt_cgfxGetTechnique( g_rt_filter.cgEffect, "blurFast2d" );
	g_rt_filter.blurAlphaFast2d = rt_cgfxGetTechnique( g_rt_filter.cgEffect, "blurAlphaFast2d" );
	g_rt_filter.gaussian1d = rt_cgfxGetTechnique( g_rt_filter.cgEffect, "gaussian1d" );
	g_rt_filter.gaussianDepth1d = rt_cgfxGetTechnique( g_rt_filter.cgEffect, "gaussianDepth1d" );
	g_rt_filter.bilateral1d = rt_cgfxGetTechnique( g_rt_filter.cgEffect, "bilateral1d" );
	g_rt_filter.bilateralDepth1d = rt_cgfxGetTechnique( g_rt_filter.cgEffect, "bilateralDepth1d" );
	g_rt_filter.trilateral1d = rt_cgfxGetTechnique( g_rt_filter.cgEffect, "trilateral1d" );

	g_rt_filter.modelViewProjParam = cgGetNamedEffectParameter(g_rt_filter.cgEffect, "ModelViewProj");
	g_rt_filter.sigmaParam = cgGetNamedEffectParameter(g_rt_filter.cgEffect, "sigma");
	g_rt_filter.screenstepParam = cgGetNamedEffectParameter(g_rt_filter.cgEffect, "screenstep");

	g_rt_filter.colorTexParam = cgGetNamedEffectParameter(g_rt_filter.cgEffect, "colorTex");
	g_rt_filter.colorTexNPOTParam = cgGetNamedEffectParameter(g_rt_filter.cgEffect, "colorTexNPOT");
	g_rt_filter.color2TexParam = cgGetNamedEffectParameter(g_rt_filter.cgEffect, "color2Tex");
	g_rt_filter.color2TexNPOTParam = cgGetNamedEffectParameter(g_rt_filter.cgEffect, "color2TexNPOT");

}

static void render_prep(void)
{
	// Load the shaders if they have not been loaded yet
	if (!g_rt_filter.cgContext)
	{
		g_rt_filter.cgContext = rt_cgfxGetGlobalContext();

		reloadShaders();
	}

	// Reset WCW_statemgmt's cached vertex/fragment shaders
	WCW_ResetState_CgFx();

	// Reset rt_cgfx's cached vertex/fragmet shaders
	if ( rt_cgGetCgShaderMode() )
	{
		rt_cgResetProgram(kShaderPgmType_FRAGMENT);
		rt_cgResetProgram(kShaderPgmType_VERTEX);
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS); CHECKGL;

	glMatrixMode(GL_MODELVIEW); CHECKGL;
	glPushMatrix(); CHECKGL;
	glLoadIdentity(); CHECKGL;

	glMatrixMode(GL_PROJECTION); CHECKGL;
	glPushMatrix(); CHECKGL;
	glLoadIdentity(); CHECKGL;
	glOrtho(-1, 1, -1, 1, 0, 1); CHECKGL;

	glDisable(GL_DEPTH_TEST); CHECKGL;
	glDepthMask(GL_FALSE); CHECKGL;
	WCW_DisableBlend();
	glDisable(GL_ALPHA_TEST); CHECKGL;

	cgGLSetManageTextureParameters(g_rt_filter.cgContext, CG_TRUE);
}

static void render_restore(void)
{
	cgGLSetManageTextureParameters(g_rt_filter.cgContext, CG_FALSE);

	glMatrixMode(GL_MODELVIEW); CHECKGL;
	glPopMatrix(); CHECKGL;

	glMatrixMode(GL_PROJECTION); CHECKGL;
	glPopMatrix(); CHECKGL;

	glPopAttrib(); CHECKGL;
	WCW_FetchGLState();
	WCW_ResetState();
}

static CGtechnique lookup_technique( int blurMode )
{
	const CGtechnique blurPrograms[] = {
		NULL,
		g_rt_filter.blurFast2d,
		g_rt_filter.blurAlphaFast2d,
		g_rt_filter.gaussian1d,
		g_rt_filter.bilateral1d,
		g_rt_filter.gaussianDepth1d,
		g_rt_filter.bilateralDepth1d,
		g_rt_filter.trilateral1d,
	};

	STATIC_ASSERT(ARRAY_SIZE(blurPrograms) == FILTER_BLUR_MAX);
	onlydevassert( blurMode >= 0 &&blurMode < FILTER_BLUR_MAX );
	return blurPrograms[ blurMode ];
}

void rdrFilterHandleEffectReloadDirect( const char* szEffectName )
{
	if ( stricmp( szEffectName, kFilter_EffectName ) == 0 )
	{
		reloadShaders();
	}
}

// Apply filter mode to an FBO Render Target (RT),
// It is assumed that the RT has an FBO with an aux
// color attachment for efficient 'ping-pong' filtering
// passes (e.g. as would be used on a separable filter kernel)
void rdrFilterRenderTargetDirect( PBuffer* pRT, int blurMode )
{
	// we only filter RT that are FBO targets with a second texture
	// attachment specifically for ping-pong passes
	if ( pRT->color_handle_aux[0] == 0 || !(pRT->flags&(PB_FBO|PB_COLOR_TEXTURE_AUX)) )
	{
		return;
	}

	render_prep();
	{
		CGtechnique technique = lookup_technique(blurMode);
		int tex0 = pRT->color_handle[pRT->curFBO];		// first render target color attachement
		int tex1 = pRT->color_handle_aux[pRT->curFBO];	// second (aux) render target color attachement
		int	width = pRT->width;
		int height = pRT->height;
		float screenstep[4] = {1.0f / (float)width, 1.0f / (float)height, 0.0f, 0.0f};

		glViewport(0, 0, width, height); CHECKGL;

		cgGLSetStateMatrixParameter(g_rt_filter.modelViewProjParam,CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);

		cgGLSetParameter4fv(g_rt_filter.screenstepParam, screenstep);
		cgGLSetParameter2f(g_rt_filter.colorTexNPOTParam, width, height);
		
		//** PASS 1
		// in the first pass we set the aux texture as the destination
		// surface and use the current fbo color buffer as the source
		// texture for the filtering

		// set the second texture as output buffer for the shader
		glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT); CHECKGL;

		// attach the input texture to the first Texture Unit
		cgGLSetupSampler(g_rt_filter.colorTexParam, tex0);

		drawFullscreenShader(technique);	// apply a pass of the selected filtering technique

		//** PASS 2
		// in the second pass we pong back to the original fbo destination
		// surface and use the fbo aux color buffer attach we just rendered
		// as the source texture for the filtering

		// set the first texture as output buffer for the shader
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT); CHECKGL;

		// attach the input texture to the first Texture Unit
		cgGLSetupSampler(g_rt_filter.colorTexParam, tex1);

		drawFullscreenShader(technique);	// apply a pass of the selected filtering technique
	}
	render_restore();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

This is the original prototype filter code used for the reflection viewports
which isn't production ready, it was just for visual evaluation purposes.

Part of the intent here was to provide a generic filtering service and to
use this service instead of the filtering code in the rt_ssao module....which
is why most of this code came from the ssao module but then got hacked up.

Should probably just delete this code until such time as someone wants to
provide a more generic filter module.

STILL USED to do dynamic cubemap filters if enabled in the tweak menu
*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static float sigma[4] = {2.0f, 1.0f, 0.0003f, 0.0f};

static GLuint makeTex(GLuint target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * data)
{
	GLuint texture;
	GLenum error;

	glGenTextures(1, &texture); CHECKGL;
	WCW_BindTexture(target, 0, texture);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0); CHECKGL;

	gldGetError();
	glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, data);
	error = gldGetError();

	WCW_BindTexture(target, 0, 0);

	if(error) {
		glDeleteTextures(1, &texture); CHECKGL;
		return 0;
	}

	return texture;
}

static void setTexMode(GLuint target, GLuint texture, GLint minFilter, GLint maxFilter, GLint wrapS, GLint wrapT)
{
	WCW_BindTexture(target, 0, texture);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter); CHECKGL;
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, maxFilter); CHECKGL;
	glTexParameterf(target, GL_TEXTURE_WRAP_S, wrapS); CHECKGL;
	glTexParameterf(target, GL_TEXTURE_WRAP_T, wrapT); CHECKGL;
	WCW_BindTexture(target, 0, 0);
}


#define checkFramebufferStatus() _checkFramebufferStatus(__FILE__, __LINE__, __FUNCTION__)
static GLenum _checkFramebufferStatus(const char * file, int line, const char * function)
{
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		const char * str = NULL;
		switch(status) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: str = "incomplete attachment"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: str = "missing attachment"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: str = "incomplete dimensions"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: str = "incomplete formats"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: str = "incomplete draw buffer"; break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: str = "incomplete read buffer"; break;
			case GL_FRAMEBUFFER_UNSUPPORTED_EXT: str = "unsupported"; break;
		}
		printf("glCheckFramebufferStatus = 0x%04x (%s) @ %s:%d:%s\n", status, str, file, line, function);
	}
	return status;
}

static GLenum setupFbo(GLuint colorTexTarget, GLuint colorTex, GLuint depthTexTarget, GLuint depthTex, GLuint depthRenderbuffer, GLsizei width, GLsizei height)
{
	if (!g_rt_filter.fbo0) {
		glGenFramebuffersEXT(1, &g_rt_filter.fbo0); CHECKGL;
	}

	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT); CHECKGL;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &g_rt_filter.pushFbo); CHECKGL;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_rt_filter.fbo0); CHECKGL;
	if (depthTex) {
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, depthTexTarget, depthTex, 0); CHECKGL;
	} else {
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthRenderbuffer); CHECKGL;
	}
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, colorTexTarget, colorTex, 0); CHECKGL;

	glViewport(0, 0, width, height); CHECKGL;
	return checkFramebufferStatus();
}

static void restoreFbo()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_rt_filter.pushFbo); CHECKGL;
	glPopAttrib(); CHECKGL;
	g_rt_filter.pushFbo = 0;
}

static bool initFbos()
{
	// Allocate textures
	return true;
}


static int	  g_tex_size;

static void drawFilter()
{
	if (g_rt_filter.blur == FILTER_NO_BLUR) 
		return;

	if (g_rt_filter.blurWidth != g_tex_size || g_rt_filter.numFbo != rdr_frame_state.numFBOs)
	{
		int i;
		if (g_rt_filter.auxTex1[g_rt_filter.curFbo]) {
			glDeleteTextures(g_rt_filter.numFbo, g_rt_filter.auxTex1); CHECKGL;
		}
		if (g_rt_filter.auxTex2[g_rt_filter.curFbo]) {
			glDeleteTextures(g_rt_filter.numFbo, g_rt_filter.auxTex2); CHECKGL;
		}
		g_rt_filter.numFbo = rdr_frame_state.numFBOs;
		for(i=0; i<g_rt_filter.numFbo; i++) {
			g_rt_filter.auxTex1[i] = makeTex( GL_TEXTURE_2D, GL_RGBA8, g_rt_filter.blurWidth, g_rt_filter.blurHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
			g_rt_filter.auxTex2[i] = makeTex( GL_TEXTURE_2D, GL_RGBA8, g_rt_filter.blurWidth, g_rt_filter.blurHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
		}
		g_tex_size = g_rt_filter.blurWidth;
	}

	g_rt_filter.curFbo = rdr_frame_state.curFrame % g_rt_filter.numFbo;

	glViewport(0, 0, g_rt_filter.blurWidth, g_rt_filter.blurHeight); CHECKGL;
	render_prep();

	// Blur g_rt_filter
	{
		CGtechnique technique = lookup_technique(g_rt_filter.blur);
		float screenstep[4] = {1.0f / (float)g_rt_filter.blurWidth, 1.0f / (float)g_rt_filter.blurHeight, 0.0f, 0.0f};

		cgGLSetStateMatrixParameter(g_rt_filter.modelViewProjParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);
		cgGLSetParameter4fv(g_rt_filter.sigmaParam, sigma);

		if(g_rt_filter.blur == FILTER_FAST_BLUR || g_rt_filter.blur == FILTER_FAST_ALPHA_BLUR) {
			cgGLSetParameter4fv(g_rt_filter.screenstepParam, screenstep);

			// copy out the cube face
			setTexMode(GL_TEXTURE_2D, g_rt_filter.auxTex2[g_rt_filter.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); CHECKGL;
			WCW_BindTexture(GL_TEXTURE_2D, 0, g_rt_filter.auxTex2[g_rt_filter.curFbo]);
			glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, g_rt_filter.blurWidth, g_rt_filter.blurHeight, 0 ); CHECKGL;

			// attach aux surface to current FBO
			// caller needs to set currentCubemapTarget before rendering each face so we know which one to set as current face
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rt_filter.auxTex1[g_rt_filter.curFbo], 0); CHECKGL;
			checkFramebufferStatus();

			glViewport(0, 0, g_rt_filter.blurWidth, g_rt_filter.blurHeight); CHECKGL;

			//			setTexMode(g_rt_filter.srcTarget, g_rt_filter.srcTex, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			setTexMode(GL_TEXTURE_2D, g_rt_filter.auxTex2[g_rt_filter.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			cgGLSetupSampler(g_rt_filter.colorTexParam, g_rt_filter.auxTex2[g_rt_filter.curFbo]);
			//			cgGLSetupSampler(g_rt_filter.colorTexParam, g_rt_filter.srcTex);
			cgGLSetParameter2f(g_rt_filter.colorTexNPOTParam, g_rt_filter.blurWidth, g_rt_filter.blurHeight);

			// draw with the technique
			drawFullscreenShader(technique);

			// ping back to original texture as target
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, g_rt_filter.srcTarget, g_rt_filter.srcTex, 0); CHECKGL;
			glViewport(0, 0, g_rt_filter.blurWidth, g_rt_filter.blurHeight); CHECKGL;
			checkFramebufferStatus();

			setTexMode(GL_TEXTURE_2D, g_rt_filter.auxTex1[g_rt_filter.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			cgGLSetupSampler(g_rt_filter.colorTexParam, g_rt_filter.auxTex1[g_rt_filter.curFbo]);
			cgGLSetParameter2f(g_rt_filter.colorTexNPOTParam, g_rt_filter.blurWidth, g_rt_filter.blurHeight);

			drawFullscreenShader(technique);
		} else  {
			float screenstepWidth[4] = {screenstep[0], 0.0f, screenstep[2], 0.0f};
			float screenstepHeight[4] = {0.0f, screenstep[1], 0.0f, screenstep[3]};

			cgGLSetParameter4fv(g_rt_filter.screenstepParam, screenstepWidth);

			// copy out the cube face
			setTexMode(GL_TEXTURE_2D, g_rt_filter.auxTex2[g_rt_filter.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); CHECKGL;
			WCW_BindTexture(GL_TEXTURE_2D, 0, g_rt_filter.auxTex2[g_rt_filter.curFbo]);
			glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, g_rt_filter.blurWidth, g_rt_filter.blurHeight, 0 ); CHECKGL;

			// attach aux surface to current FBO
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rt_filter.auxTex1[g_rt_filter.curFbo], 0); CHECKGL;
			glViewport(0, 0, g_rt_filter.blurWidth, g_rt_filter.blurHeight); CHECKGL;
			checkFramebufferStatus();

			//			setTexMode(g_rt_filter.srcTarget, g_rt_filter.srcTex, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			setTexMode(GL_TEXTURE_2D, g_rt_filter.auxTex2[g_rt_filter.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			cgGLSetupSampler(g_rt_filter.colorTexParam, g_rt_filter.auxTex2[g_rt_filter.curFbo]);
			//			cgGLSetupSampler(g_rt_filter.colorTexParam, g_rt_filter.srcTex);
			cgGLSetParameter2f(g_rt_filter.colorTexNPOTParam, g_rt_filter.blurWidth, g_rt_filter.blurHeight);

			drawFullscreenShader(technique);

			// ping back to original texture as target
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, g_rt_filter.srcTarget, g_rt_filter.srcTex, 0); CHECKGL;
			glViewport(0, 0, g_rt_filter.blurWidth, g_rt_filter.blurHeight); CHECKGL;
			checkFramebufferStatus();

			setTexMode(GL_TEXTURE_2D, g_rt_filter.auxTex1[g_rt_filter.curFbo], GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			cgGLSetupSampler(g_rt_filter.colorTexParam, g_rt_filter.auxTex1[g_rt_filter.curFbo]);
			cgGLSetParameter2f(g_rt_filter.colorTexNPOTParam, g_rt_filter.blurWidth, g_rt_filter.blurHeight);

			cgGLSetParameter4fv(g_rt_filter.screenstepParam, screenstepHeight);

			drawFullscreenShader(technique);
		}
	}

	render_restore();
}

void rdrFilterSetParamsDirect(RdrFilterParams * params)
{	
	// Load the shaders if they have not been loaded yet
	if (!g_rt_filter.cgContext)
	{
		g_rt_filter.cgContext = rt_cgfxGetGlobalContext();

		reloadShaders();
	}

	// set and validate params
	g_rt_filter.blur = params->mode;
	g_rt_filter.srcTex = params->srcTex;
	g_rt_filter.srcTarget = params->srcTarget;
	g_rt_filter.blurWidth = params->width;
	g_rt_filter.blurHeight = params->height;

	assert( g_rt_filter.srcTarget == GL_TEXTURE_2D || (g_rt_filter.srcTarget>=GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB && g_rt_filter.srcTarget<=GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB) );

}

void rdrFilterRenderDirect( RdrFilterParams* params )
{
	rdrFilterSetParamsDirect( params );
	drawFilter();
}

