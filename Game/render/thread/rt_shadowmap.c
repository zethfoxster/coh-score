// rt_shadowmap.c

#define RT_ALLOW_BINDTEXTURE
#define RT_PRIVATE
#include "rendershadowmap.h"
#include "superassert.h"
#include "failtext.h"
#include "gfx.h"
#include "mathutil.h"
#include "camera.h"
#include "rt_shadowmap.h"
#include "rt_tune.h"
#include "rt_tex.h"
#include "renderutil.h"
#include "rt_cgfx.h"
#include "rt_stats.h"
#include "wcw_statemgmt.h"
#include "rt_shaderMgr.h"

#include "gfxwindow.h"
#include "player.h"
#include "newfeature.h"

typedef enum RTShadowOverrideID {
	kShadowMapOverride_Off,
	kShadowMapOverride_modulationClamp_ambient,
	kShadowMapOverride_modulationClamp_diffuse,
	kShadowMapOverride_modulationClamp_specular,
	kShadowMapOverride_modulationClamp_vertex_lit,
	kShadowMapOverride_modulationClamp_NdotL_rolloff,
	kShadowMapOverride_lastCascadeFade,
	kShadowMapOverride_farMax,
	kShadowMapOverride_splitLambda,
	kShadowMapOverride_maxSunAngleDegrees,
	//-------------------------------------
	kShadowMapOverride_COUNT
} RTShadowOverrideID;

typedef struct optOverrideSpec {
	void*	pTargetVal;
	size_t	targetBytes;
	U32		oldVal;			// this would need to grow if targetBytes can be larger than sizeof(U32).
} optOverrideSpec;

static optOverrideSpec	g_SceneOverrides[kShadowMapOverride_COUNT] = { 0 };

// for checking GL errors just in this file
//#undef  CHECKGL
//#define CHECKGL rdrCheckThread(); checkgl(__FILE__,__LINE__);

static shadowMapGlobals		g_rtShadowMapOpts = {0};
static shadowMapSet			g_rtShadowMaps = { 0 };
static Mat44				g_rtShadowMats[4];

static GLuint shadowDisabledTexture = 0;
static bool g_bShadowMapCompares = true;

//
// To test for overridden settings.
//
#define IsMarkerValue(_theVal_,_markerValue_) \
			( fabs( (_theVal_) - (_markerValue_) ) < 0.01f )
#define CustomizeIfValid( _id_, _custom_val_p_, _ignore_val_ )	\
			if  ( ! IsMarkerValue((*(_custom_val_p_)), (_ignore_val_)) ) \
				setSceneCustomization( _id_, _custom_val_p_ );

static INLINEDBG Mat44* getMatrix(int map_index)
{
	shadowMap * map = g_rtShadowMaps.maps + map_index;
	assert(map_index >=0 && map_index < g_rtShadowMaps.numShadows);
	return &map->shadowTexMat;
}

static INLINEDBG int getTexture(int map_index, int fbo_index)
{
	shadowMap * map = g_rtShadowMaps.maps + map_index;
	assert(map_index >=0 && map_index < g_rtShadowMaps.numShadows);
	return (map->active) ? map->texture[fbo_index] : shadowDisabledTexture;
}

// get the fbo texture for the given map at the current active fbo index 
static INLINEDBG int getActiveTexture(int map_index )
{
	assert(map_index >=0 && map_index < g_rtShadowMaps.numShadows);
	{
		shadowMap * map = g_rtShadowMaps.maps + map_index;
		int fbo_index = viewport_GetPBuffer(map->viewport)->curFBO;
		return (map->active) ? map->texture[fbo_index] : shadowDisabledTexture;
	}
}

static INLINEDBG bool isMapValid(int map_index, int fbo_index)
{
	return (map_index>=0) && (map_index < g_rtShadowMaps.numShadows) && g_rtShadowMaps.maps[map_index].active && getTexture(map_index, fbo_index);
}

void rt_shadowmap_init(shadowMapSet* pMapSet)
{
	if(!shadowDisabledTexture) {
		unsigned int depthDisabled = ~0;
		glGenTextures(1, &shadowDisabledTexture); CHECKGL;
		WCW_BindTexture(GL_TEXTURE_2D, 0, shadowDisabledTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &depthDisabled); CHECKGL;
		WCW_BindTexture(GL_TEXTURE_2D, 0, 0);
	}

	// refresh from the main thread.
	memcpy( &g_rtShadowMaps, pMapSet, sizeof(shadowMapSet) );
}

Mat44* rdrShadowMapGetMatrix(int map_index)
{
   assert(map_index >= 0);
   assert(map_index < SHADOWMAP_MAX_SHADOWS);
   return (map_index < g_rtShadowMaps.numShadows) ? (Mat44*)&(g_rtShadowMaps.maps[map_index].shadowTexMat) : NULL;
}

static void drawCube(void)
{
	glBegin(GL_QUADS);
	glVertex3f(+1,-1,+1); glVertex3f(+1,-1,-1); glVertex3f(+1,+1,-1); glVertex3f(+1,+1,+1);
	glVertex3f(+1,+1,+1); glVertex3f(+1,+1,-1); glVertex3f(-1,+1,-1); glVertex3f(-1,+1,+1);
	glVertex3f(+1,+1,+1); glVertex3f(-1,+1,+1); glVertex3f(-1,-1,+1); glVertex3f(+1,-1,+1);
	glVertex3f(-1,-1,+1); glVertex3f(-1,+1,+1); glVertex3f(-1,+1,-1); glVertex3f(-1,-1,-1);
	glVertex3f(-1,-1,+1); glVertex3f(-1,-1,-1); glVertex3f(+1,-1,-1); glVertex3f(+1,-1,+1);
	glVertex3f(-1,-1,-1); glVertex3f(-1,+1,-1); glVertex3f(+1,+1,-1); glVertex3f(+1,-1,-1);
	glEnd(); CHECKGL;
}

static void drawQuad(void)
{
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(-1,-1);
	glTexCoord2f(0, 1);
	glVertex2f(-1,+1);
	glTexCoord2f(1, 1);
	glVertex2f(+1,+1);
	glTexCoord2f(1, 0);
	glVertex2f(+1,-1);
	glEnd(); CHECKGL;
}

static void drawPlane( float size_x, float size_y, Vec3 loc )
{
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(loc[0]-size_x,loc[1]-size_y,loc[2]);
	glTexCoord2f(0, 1);
	glVertex3f(loc[0]-size_x,loc[1]+size_y,loc[2]);
	glTexCoord2f(1, 1);
	glVertex3f(loc[0]+size_x,loc[1]+size_y,loc[2]);
	glTexCoord2f(1, 0);
	glVertex3f(loc[0]+size_x,loc[1]-size_y,loc[2]);
	glEnd(); CHECKGL;
}


// calculate the world coordinates of the 8 corner points of the given view frustum
static void buildFrustum( shadowFrustum* frustum, Mat4 view_to_world, float z_near, float z_far, float fov_x, float fov_y )
{
	Vec3 points[8];
	float fov_x_rad = RAD(fov_x);
	float fov_y_rad = RAD(fov_y);

	frustum->fov_x = fov_x_rad;
	frustum->fov_y = fov_y_rad;
	
	frustum->z_far = z_far;
	frustum->z_near = z_near;

	copyVec3( view_to_world[3], frustum->origin );
	copyVec3( view_to_world[2], frustum->viewDir );
	copyMat4( view_to_world, frustum->view_to_world );

	calcFrustumPoints( points, view_to_world, z_near, z_far, fov_x, fov_y );

	copyVec3( points[0], frustum->nbl );
	copyVec3( points[1], frustum->ntl );
	copyVec3( points[2], frustum->ntr );
	copyVec3( points[3], frustum->nbr );
	copyVec3( points[4], frustum->fbl );
	copyVec3( points[5], frustum->ftl );
	copyVec3( points[6], frustum->ftr );
	copyVec3( points[7], frustum->fbr );
}

static void calcFrustumSlice( Vec3* points, float z_slice, shadowFrustum* frust )
{
	Vec3 slice_center;

	float height = tanf(frust->fov_y/2.0f) * z_slice;
	float width = tanf(frust->fov_x/2.0f) * z_slice;

	// calculate the center point of the near and far plane rectangles
	scaleAddVec3( frust->viewDir, z_slice, frust->origin, slice_center );	// center of slice plane = eye_loc + view_dir*z_slice

	// slice corners
	scaleAddVec3( frust->view_to_world[1], -height, slice_center, points[0] );	//  slice_center - up*near_height - right*near_width;
	scaleAddVec3( frust->view_to_world[0], -width, points[0], points[0] );

	scaleAddVec3( frust->view_to_world[1], +height, slice_center, points[1] );	//  slice_center + up*near_height - right*near_width;
	scaleAddVec3( frust->view_to_world[0], -width, points[1], points[1] );

	scaleAddVec3( frust->view_to_world[1], +height, slice_center, points[2] );	//  slice_center + up*near_height + right*near_width;
	scaleAddVec3( frust->view_to_world[0], +width, points[2], points[2] );

	scaleAddVec3( frust->view_to_world[1], -height, slice_center, points[3] );	//  slice_center - up*near_height + right*near_width;
	scaleAddVec3( frust->view_to_world[0], +width, points[3], points[3] );
}

static void drawVolumes(void)
{
	int i;
	for(i = 0; i < g_rtShadowMaps.numShadows; i++) {
		shadowMap * map = g_rtShadowMaps.maps + i;
		if(!map->active) continue;

		glPushMatrix(); CHECKGL;
		glMultMatrixf((const float *)map->shadowMat); CHECKGL;
		glColor4ub(map->debugColor[0] * 255, map->debugColor[1] * 255, map->debugColor[2] * 255, map->debugColor[3] * 255);
		CHECKGL;
		drawCube();
		glPopMatrix(); CHECKGL;
	}
}

static void drawFrustum(shadowFrustum* frust)
{
	glBegin(GL_QUADS);

	//near plane
	glColor4ub(255, 255, 0, g_rtShadowMapOpts.debugDrawAlpha * 255);
	glVertex3f(frust->ntl[0],frust->ntl[1],frust->ntl[2]);
	glVertex3f(frust->ntr[0],frust->ntr[1],frust->ntr[2]);
	glVertex3f(frust->nbr[0],frust->nbr[1],frust->nbr[2]);
	glVertex3f(frust->nbl[0],frust->nbl[1],frust->nbl[2]);

	//far plane
	glColor4ub(192, 192, 0, g_rtShadowMapOpts.debugDrawAlpha * 255);
	glVertex3f(frust->ftr[0],frust->ftr[1],frust->ftr[2]);
	glVertex3f(frust->ftl[0],frust->ftl[1],frust->ftl[2]);
	glVertex3f(frust->fbl[0],frust->fbl[1],frust->fbl[2]);
	glVertex3f(frust->fbr[0],frust->fbr[1],frust->fbr[2]);

	//bottom plane
	glColor4ub(255, 255, 255, g_rtShadowMapOpts.debugDrawAlpha * 255);
	glVertex3f(frust->nbl[0],frust->nbl[1],frust->nbl[2]);
	glVertex3f(frust->nbr[0],frust->nbr[1],frust->nbr[2]);
	glVertex3f(frust->fbr[0],frust->fbr[1],frust->fbr[2]);
	glVertex3f(frust->fbl[0],frust->fbl[1],frust->fbl[2]);

	//top plane
	glColor4ub(216, 216, 216, g_rtShadowMapOpts.debugDrawAlpha * 255);
	glVertex3f(frust->ntr[0],frust->ntr[1],frust->ntr[2]);
	glVertex3f(frust->ntl[0],frust->ntl[1],frust->ntl[2]);
	glVertex3f(frust->ftl[0],frust->ftl[1],frust->ftl[2]);
	glVertex3f(frust->ftr[0],frust->ftr[1],frust->ftr[2]);

	//left plane
	glColor4ub(192, 192, 192, g_rtShadowMapOpts.debugDrawAlpha * 255);
	glVertex3f(frust->ntl[0],frust->ntl[1],frust->ntl[2]);
	glVertex3f(frust->nbl[0],frust->nbl[1],frust->nbl[2]);
	glVertex3f(frust->fbl[0],frust->fbl[1],frust->fbl[2]);
	glVertex3f(frust->ftl[0],frust->ftl[1],frust->ftl[2]);

	// right plane
	glColor4ub(165, 165, 165, g_rtShadowMapOpts.debugDrawAlpha * 255);
	glVertex3f(frust->nbr[0],frust->nbr[1],frust->nbr[2]);
	glVertex3f(frust->ntr[0],frust->ntr[1],frust->ntr[2]);
	glVertex3f(frust->ftr[0],frust->ftr[1],frust->ftr[2]);
	glVertex3f(frust->fbr[0],frust->fbr[1],frust->fbr[2]);

	glEnd(); CHECKGL;

}

static void drawShadowedViewFrustum(void)
{	
	drawFrustum(&g_rtShadowMapOpts.viewFrustum);
}

static void drawSplits(void)
{
	// use the saved view frustum to build split quads

	int i;
	for(i = 0; i < g_rtShadowMaps.numShadows; i++)
	{
		shadowMap * map = g_rtShadowMaps.maps + i;
		if(!map->active) continue;

		{
			Vec3 points[4];
			float z_slice = map->csm_eye_z_split_far;
			calcFrustumSlice( points, z_slice, &g_rtShadowMapOpts.viewFrustum );

			glBegin(GL_QUADS);

			glColor4ub(0, 255, 255, g_rtShadowMapOpts.debugDrawAlpha * 255);
			glVertex3f(points[0][0],points[0][1],points[0][2]);
			glVertex3f(points[1][0],points[1][1],points[1][2]);
			glVertex3f(points[2][0],points[2][1],points[2][2]);
			glVertex3f(points[3][0],points[3][1],points[3][2]);

			glEnd(); CHECKGL;
		}
	}

}

static void drawAxis( Vec3 origin, float length )
{
	glBegin(GL_LINES);
		glColor3ub(255,0,0);
		glVertex3f(origin[0],origin[1], origin[2]);
		glVertex3f(origin[0]+length,origin[1], origin[2]);
		glColor3ub(0,255,0);
		glVertex3f(origin[0],origin[1], origin[2]);
		glVertex3f(origin[0],origin[1]+length, origin[2]);
		glColor3ub(0,0,255);
		glVertex3f(origin[0],origin[1], origin[2]);
		glVertex3f(origin[0],origin[1], origin[2]+length);
	glEnd(); CHECKGL;

}


static void drawDebugAxis(void)
{
	Vec3 pos;
	playerGetPos(pos);
	drawAxis( pos, 3.0f );
	setVec3(pos, 0.0f, 0.0f, 0.0f );
	drawAxis( pos, 1000.0f );
}

void rt_rdrShadowMapDebug3D(void)
{
	Mat44 mat;

	rdrBeginMarker(__FUNCTION__);
	WCW_FetchGLState();
	WCW_ResetState();

	WCW_BindVertexProgram(0);
	WCW_DisableVertexProgram();
	WCW_BindFragmentProgram(0);
	WCW_DisableFragmentProgram();
	WCW_PrepareToDraw();

	glPushAttrib(GL_ALL_ATTRIB_BITS); CHECKGL;

	glMatrixMode(GL_MODELVIEW); CHECKGL;
	glPushMatrix(); CHECKGL;
	mat43to44(rdr_view_state.viewmat, mat);
	glLoadMatrixf((const float *)mat); CHECKGL;

	glDisable(GL_TEXTURE_2D); CHECKGL;
	glDepthMask(GL_FALSE); CHECKGL;
	glDisable(GL_CULL_FACE); CHECKGL;
	WCW_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_ALPHA_TEST); CHECKGL;

	if (!game_state.shadowMapFreeze)
	{
		buildFrustum( &g_rtShadowMapOpts.viewFrustum, cam_info.cammat, -rdr_view_state.nearz, -rdr_view_state.farz, gfx_window.fovx, gfx_window.fovy );
	}

	if (game_state.shadowMapDebug & showVolumes3D)
	{
		drawVolumes();
	}
	if (game_state.shadowMapDebug & showFrustum)
	{
		drawShadowedViewFrustum();
	}
	if (game_state.shadowMapDebug & showAxis)
	{
		drawDebugAxis();
	}
	if (game_state.shadowMapDebug & showSplits)
	{
		drawSplits();
	}

	glPopMatrix(); CHECKGL;
	glPopAttrib(); CHECKGL;

	WCW_FetchGLState();
	WCW_ResetState();
	rdrEndMarker();
}

static void drawOverview2D(void)
{
	Mat44 worldTo2D;
	float scale;

	glPushMatrix(); CHECKGL;

	identityMat44(worldTo2D);

	scale = 1.0f/g_rtShadowMapOpts.debugScale;
	worldTo2D[0][0] = scale;

	// swap z and -y
	worldTo2D[1][1] = 0.0f;
	worldTo2D[2][1] = scale;

	worldTo2D[2][1] = -scale;
	worldTo2D[2][2] = 0.0f;

	// center on screen
	worldTo2D[3][0] = 600.0f;
	worldTo2D[3][1] = 300.0f;

	glMultMatrixf( (float*)&worldTo2D); CHECKGL;

	drawShadowedViewFrustum();
	drawDebugAxis();
	drawSplits();

	glPopMatrix(); CHECKGL;
}

static void drawShadowMapsOnQuads2D(void)
{
	rdrSetMarker(__FUNCTION__);

	// all 4 cascades share the same render target
	// so we only need to draw the entire map 0 texture
	if ( g_rtShadowMaps.numShadows > 0 )
	{
		float mapSize = (float)g_rtShadowMapOpts.debugShowMapSize;
		const int texShadowMap = getActiveTexture(0);

		WCW_BindTexture( GL_TEXTURE_2D, 0, texShadowMap );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE ); CHECKGL;

		glPushMatrix(); CHECKGL;
		glTranslatef( g_rtShadowMapOpts.debugShowMapOffsetX + mapSize/2, g_rtShadowMapOpts.debugShowMapOffsetY + mapSize/2,0); CHECKGL;
		glScalef(mapSize,mapSize,1); CHECKGL;
		drawQuad(); CHECKGL;
		glPopMatrix(); CHECKGL;

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB ); CHECKGL;
	}
}

void rt_rdrShadowMapDebug2D(void)
{
	rdrBeginMarker(__FUNCTION__);
	WCW_FetchGLState();
	WCW_ResetState();

	WCW_BindVertexProgram(0);
	WCW_DisableVertexProgram();
	WCW_BindFragmentProgram(0);
	WCW_DisableFragmentProgram();
	WCW_PrepareToDraw();

	glPushAttrib(GL_ALL_ATTRIB_BITS); CHECKGL;

	glMatrixMode(GL_MODELVIEW); CHECKGL;
	glPushMatrix(); CHECKGL;

	WCW_EnableTexture(GL_TEXTURE_2D, 0); CHECKGL;
	glDisable(GL_DEPTH_TEST); CHECKGL;
	glDisable(GL_CULL_FACE); CHECKGL;
	WCW_DisableBlend();
	glDisable(GL_ALPHA_TEST); CHECKGL;

	if (game_state.shadowMapDebug & showMaps2D)
		drawShadowMapsOnQuads2D();
	if (game_state.shadowMapDebug & showOverview2D)
		drawOverview2D();

	glPopMatrix(); CHECKGL;
	glPopAttrib(); CHECKGL;

	WCW_FetchGLState();
	WCW_ResetState();
	rdrEndMarker();
}

static bool g_bEnableSlopeScaleBias = true;

void rt_shadowViewportPreCallback(int map_index)
{
	shadowMap * map = g_rtShadowMaps.maps + map_index;
	assert(map_index >=0 && g_rtShadowMaps.numShadows);

	if (g_bEnableSlopeScaleBias)
	{
		rdrBeginMarker(__FUNCTION__);
		glEnable(GL_POLYGON_OFFSET_FILL); CHECKGL;
		glPolygonOffset( g_rtShadowMapOpts.slopeBias * map->biasScale, g_rtShadowMapOpts.constantBias * map->biasScale ); CHECKGL;
		rdrEndMarker();
	}

	WCW_Color4(	255, 255, 255, 255);
}

void rt_shadowViewportPostCallback(int map_index)
{
	static float fBorderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	shadowMap * map = g_rtShadowMaps.maps + map_index;
	assert(map_index >=0 && g_rtShadowMaps.numShadows);

	rdrBeginMarker(__FUNCTION__);
	if (g_bEnableSlopeScaleBias)
	{
		glDisable(GL_POLYGON_OFFSET_FILL); CHECKGL;
	}

	WCW_BindTexture( GL_TEXTURE_2D, 0, map->texture[viewport_GetPBuffer(map->viewport)->curFBO] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB ); CHECKGL;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL ); CHECKGL;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_rtShadowMapOpts.enablePCF ? GL_LINEAR : GL_NEAREST ); CHECKGL;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_rtShadowMapOpts.enablePCF ? GL_LINEAR : GL_NEAREST ); CHECKGL;
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, fBorderColor); CHECKGL;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER ); CHECKGL;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER ); CHECKGL;
	WCW_BindTexture( GL_TEXTURE_2D, 0, 0 );

	texBind(TEXLAYER_BASE, 0);
	WCW_FetchGLState();
	WCW_ResetState();
	rdrEndMarker();
}

// @todo hack to tell shaders not to lookup shadowmaps during map pass until we have separate shaders
// @todo we can also do this once at beginning of viewports that render with shadows
static void disableInShaders( const int tex_unit )
{
	Vec4 shadowParams;			// x - clamp 1
	// a clamp of 1 effectively disables shadowmap lookup in the shaders
	setVec4(shadowParams, 1.0f, 0.0f, 0.0f, 0.0f );	// x - enable 1,
//	WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_ShadowParamsVP, shadowParams);
	WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_ShadowParamsFP, shadowParams);

	// disable the sampler with the shadowmap atlas
	if ( g_rtShadowMaps.numShadows > 0 )
	{
		texBind(tex_unit, shadowDisabledTexture);
	}
}

static void setupSampler( const int tex_unit, const int shadowMapBind, const int fboIndex )
{
	const int texShadowMap = getTexture( shadowMapBind, fboIndex );
	assert(texShadowMap);
	texBind(tex_unit, texShadowMap);
}

static void setupShaderConstants( void )
{
	int i;

	// generate the shader param matrices and set them into the 'env' cache
	{
		memset( g_rtShadowMats, 0, sizeof(Mat44)*4 );
		assert( g_rtShadowMaps.numShadows <= 4 );
		for (i=0; i<g_rtShadowMaps.numShadows; ++i)
		{
			const Mat44* mtxShadowMap = getMatrix(i);
			Mat44 Mv;

			assert(mtxShadowMap);

			// @todo we can precalc this ahead of time, was for development
			// need to pre-multiply with the cameras view to world matrix
			mat43to44(rdr_view_state.inv_viewmat, Mv);
			mulMat44( *mtxShadowMap, Mv, g_rtShadowMats[i]);

		// @todo keep setting individual 'matrices' until the array is working in the wrapper
		//@todo pass transpose to save shader constant, i.e. 3x4 instead of 4x3
		// Cg wrapper is setting column major by default so we can probably switch shader?
			WCW_SetCgShaderParamArray4fv(kShaderPgmType_FRAGMENT, kShaderParam_ShadowMap1MatrixFP + i, (float*)(&g_rtShadowMats[i]), 4);
		}

//		WCW_SetCgShaderParamArray4fv(kShaderPgmType_FRAGMENT, kShaderParam_ShadowMapMatrixArrayFP, (float*)g_rtShadowMats, 16);
	}

	// cache the split z values for the fragment shader constant
	{
		Vec4 z_splits;
		Vec4 shadowParams2;			// x - shadowing boundary, y - shadowing boudnary fade factor, z - extra N*L rolloff clamp, w - map size scale (for texel offsets)
		F32 lastSplitStart = 0.0f;
		F32 lastSplitEnd;
		float mapWidth = (float)(g_rtShadowMaps.maps[0].viewport ? g_rtShadowMaps.maps[0].viewport->width : 1);
		float mapHeight = (float)(g_rtShadowMaps.maps[0].viewport ? g_rtShadowMaps.maps[0].viewport->height : 1);
		float invMapSize = 1.0f/mapWidth;

		zeroVec4(z_splits);
		for (i=0; i<g_rtShadowMaps.numShadows; ++i)
		{
			z_splits[i] = fabs( g_rtShadowMaps.maps[i].csm_eye_z_split_far );
		}		
		WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_ShadowSplitsFP, z_splits);

		if (g_rtShadowMaps.numShadows > 1)
		{
			lastSplitStart = z_splits[g_rtShadowMaps.numShadows - 2];
		}
		lastSplitEnd = z_splits[g_rtShadowMaps.numShadows - 1];

		// We supply shadowing boundary and transition zone scale size to shader
		// so that it can fade out shadows as they approach the shadow boundary
		// We currently do this as a percentage over the last cascade but it could
		// be any distance
		{
			shadowParams2[0] = lastSplitEnd;
			if ( g_rtShadowMapOpts.lastCascadeFade > 1e-5)
				shadowParams2[1] = 1.0f / ((lastSplitEnd - lastSplitStart) * g_rtShadowMapOpts.lastCascadeFade);
			else
				shadowParams2[1] = 1e5;	// large scaling factor makes transition zone essentially right on top of boundary
		}

		shadowParams2[2] = g_rtShadowMapOpts.modulationClamp_NdotL_rolloff;
		assert( mapWidth == mapHeight );	// assuming width/height same for now
		shadowParams2[3] = invMapSize;
		WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_ShadowParams2FP, shadowParams2);

	}

	// shadow render control shader params
	{
		Vec4 shadowParams;			// xyz - shadow strength clamps (ambient, diffuse, specular) when lit per pixel, w - shadow strength clamp when lit per vertex

#define LERP(v0, v1, w) ((v1)*(w)+(v0)*(1-(w)))
		float modulationClampNight_ambient		= g_rtShadowMapOpts.modulationClamp_ambient + g_rtShadowMapOpts.nightModulationReduction * (1.0f - g_rtShadowMapOpts.modulationClamp_ambient);
		float modulationClampNight_diffuse		= g_rtShadowMapOpts.modulationClamp_diffuse + g_rtShadowMapOpts.nightModulationReduction * (1.0f - g_rtShadowMapOpts.modulationClamp_diffuse);
		float modulationClampNight_specular		= g_rtShadowMapOpts.modulationClamp_specular + g_rtShadowMapOpts.nightModulationReduction * (1.0f - g_rtShadowMapOpts.modulationClamp_specular);
		float modulationClampNight_vertex_lit	= g_rtShadowMapOpts.modulationClamp_vertex_lit + g_rtShadowMapOpts.nightModulationReduction * (1.0f - g_rtShadowMapOpts.modulationClamp_vertex_lit);
		float modulationClampNight_ambient_only_obj	= g_rtShadowMapOpts.modulationClamp_ambient_only_obj + g_rtShadowMapOpts.nightModulationReduction * (1.0f - g_rtShadowMapOpts.modulationClamp_ambient_only_obj);
		float nightPercent = ( rdr_view_state.fixed_sun_pos ? 0.0f : ( rdr_view_state.lamp_alpha / 255.0f ));
		float modulationClamp_ambient			= LERP(g_rtShadowMapOpts.modulationClamp_ambient, modulationClampNight_ambient, nightPercent);
		float modulationClamp_diffuse			= LERP(g_rtShadowMapOpts.modulationClamp_diffuse, modulationClampNight_diffuse, nightPercent);
		float modulationClamp_specular			= LERP(g_rtShadowMapOpts.modulationClamp_specular, modulationClampNight_specular, nightPercent);
		float modulationClamp_vertex_lit		= LERP(g_rtShadowMapOpts.modulationClamp_vertex_lit, modulationClampNight_vertex_lit, nightPercent);
		float modulationClamp_ambient_only_obj	= LERP(g_rtShadowMapOpts.modulationClamp_ambient_only_obj, modulationClampNight_ambient_only_obj, nightPercent);
#undef LERP

		setVec4(shadowParams, modulationClamp_ambient, modulationClamp_diffuse, modulationClamp_specular, modulationClamp_vertex_lit );	
		WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_ShadowParamsFP, shadowParams);

		// cache of the currently set shadow params so that we can override more easily (@todo kind of a hack)
		copyVec4(shadowParams, g_rtShadowMapOpts.currentShadowParamsFP );	
		g_rtShadowMapOpts.current_modulationClamp_ambient_only_obj = modulationClamp_ambient_only_obj;
	}

	// set cascade blend overlap distances
	{
		Vec4 shadowParams3;			// x - unused, yzw - cascade blend distances for casscades 2,3,4
		zeroVec4(shadowParams3);
		for (i=0; i<g_rtShadowMaps.numShadows; ++i)
		{
			shadowParams3[i] = game_state.shadowmap_split_overlap[i];
		}		
		WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_ShadowParams3FP, shadowParams3 );
	}
}

// In some circumstances we may need to use a custom ambient shadow clamp.
// For example, on objects with the OBJ_NOLIGHTANGLE flag there is no diffuse
// lighting (diffuse light color is set to zero).
// Since there is no diffuse lighting here we need to have the shadows attenuate the ambient
// much more if we want cast shadows to show up on these objects
// @todo kind of a hack
void rt_shadowmap_override_ambient_clamp()
{
	if( !rt_cgGetCgShaderMode() )
		return;

	// @todo viewports should probably decide if they want shadows or not
	if (rdr_view_state.shadowMode >= SHADOW_SHADOWMAP_LOW)
	{
		rdrBeginMarker(__FUNCTION__);
		{
			Vec4 shadowParams;			// xyz - shadow strength clamps (ambient, diffuse, specular) when lit per pixel, w - shadow strength clamp when lit per vertex
			copyVec4( g_rtShadowMapOpts.currentShadowParamsFP, shadowParams );	// copy from cache of the currently set shadow params so that we can override more easily (@todo kind of a hack)
			shadowParams[0] = g_rtShadowMapOpts.current_modulationClamp_ambient_only_obj;
			WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_ShadowParamsFP, shadowParams);
		}
		rdrEndMarker();
	}
}

void rt_shadowmap_restore_viewport_ambient_clamp( void )
{
	if( !rt_cgGetCgShaderMode() )
		return;

	// @todo viewports should probably decide if they want shadows or not
	if (rdr_view_state.shadowMode >= SHADOW_SHADOWMAP_LOW)
	{
		rdrBeginMarker(__FUNCTION__);
		{
			WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_ShadowParamsFP, g_rtShadowMapOpts.currentShadowParamsFP);
		}
		rdrEndMarker();}
}

void rt_shadowmap_dbg_test_settings(void)
{
	if (game_state.shadowDebugFlags&kShadowTestToggle)
	{
		if( !rt_cgGetCgShaderMode() )
			return;

		// @todo viewports should probably decide if they want shadows or not
		if (rdr_view_state.shadowMode >= SHADOW_SHADOWMAP_LOW )
		{
			// enable shadowmap lookups
			setupShaderConstants();

			// setup the sampler for the maps texture atlas
			if ( g_rtShadowMaps.numShadows > 0 )
			{
				// see what sampler is already set?
				const int texShadowMap = getActiveTexture(0);
				GLint t;
				WCW_ActiveTexture(TEXLAYER_SHADOWMAP);
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &t); 
				if (t != texShadowMap)
				{
					texBind(TEXLAYER_SHADOWMAP, texShadowMap);
				}
			}
		}
		else
		{
			// disable shadowmap lookups
			disableInShaders(TEXLAYER_SHADOWMAP);
		}
	}
}

// Called to setup the shadowmapping environment for a rendering target
// The state set here should only need to be setup once at the start of rendering big sets of batches
// as there currently aren't any collisions or per object changes, i.e. constant for the target
// HOWEVER, beware steps that can reset the texture sampler states back to 0 (such as doing a frame
// grab for water)
void rt_shadowmap_begin_using(void)
{
	if( !rt_cgGetCgShaderMode() )
		return;

	rdrBeginMarker(__FUNCTION__);

	// @todo viewports should probably decide if they want shadows or not
	if (rdr_view_state.shadowMode >= SHADOW_SHADOWMAP_LOW)
	{
		// enable shadowmap lookups
		setupShaderConstants();

		// setup the sampler for the maps texture atlas
		if ( g_rtShadowMaps.numShadows > 0 )
			setupSampler( TEXLAYER_SHADOWMAP, 0, viewport_GetPBuffer(g_rtShadowMaps.maps[0].viewport)->curFBO );
	}
	else
	{
		// disable shadowmap lookups
		disableInShaders(TEXLAYER_SHADOWMAP);
	}

	rdrEndMarker();
}

//@todo probably not necessary any longer
void rt_shadowmap_end_using(void)
{
	if( !rt_cgGetCgShaderMode() )
		return;

	// disable shadowmap lookups
	disableInShaders(TEXLAYER_SHADOWMAP);
}

static void initOverrideItem( RTShadowOverrideID id, void* overriddenValue, size_t itemSize )
{
	g_SceneOverrides[id].pTargetVal		= overriddenValue;
	g_SceneOverrides[id].targetBytes	= itemSize;
	assert(itemSize <= sizeof(g_SceneOverrides[id].oldVal) );
	
	// grab the current value, while we're at it.
	memmove( &g_SceneOverrides[id].oldVal, overriddenValue, itemSize );
}

static void initOverrideTable(void)
{
	static bool bInitialized = false;
	if ( ! bInitialized )
	{
		initOverrideItem(kShadowMapOverride_Off,							&scene_info.shadowMaps_Off,							sizeof(int) );
		initOverrideItem(kShadowMapOverride_modulationClamp_ambient,		&g_rtShadowMapOpts.modulationClamp_ambient,			sizeof(F32) );
		initOverrideItem(kShadowMapOverride_modulationClamp_diffuse,		&g_rtShadowMapOpts.modulationClamp_diffuse,			sizeof(F32) );
		initOverrideItem(kShadowMapOverride_modulationClamp_specular,		&g_rtShadowMapOpts.modulationClamp_specular,		sizeof(F32) );
		initOverrideItem(kShadowMapOverride_modulationClamp_vertex_lit,		&g_rtShadowMapOpts.modulationClamp_vertex_lit,		sizeof(F32) );
		initOverrideItem(kShadowMapOverride_modulationClamp_NdotL_rolloff,	&g_rtShadowMapOpts.modulationClamp_NdotL_rolloff,	sizeof(F32) );
		initOverrideItem(kShadowMapOverride_lastCascadeFade,				&g_rtShadowMapOpts.lastCascadeFade,					sizeof(F32) );
		initOverrideItem(kShadowMapOverride_farMax,							&game_state.shadowFarMax,							sizeof(F32) );
		initOverrideItem(kShadowMapOverride_splitLambda,					&game_state.shadowSplitLambda,						sizeof(F32) );
		initOverrideItem(kShadowMapOverride_maxSunAngleDegrees,				&game_state.shadowMaxSunAngleDeg,					sizeof(F32) );
		bInitialized = true;
	}
}

static void clearSceneCustomizations( void )
{
	int i;
	initOverrideTable();
	for ( i=0; i < ARRAY_SIZE(g_SceneOverrides); i++ )
	{
		memmove( g_SceneOverrides[i].pTargetVal, &g_SceneOverrides[i].oldVal, g_SceneOverrides[i].targetBytes );
	}
}

static void setBaseSceneCustomizations( void )
{
	int i;
	initOverrideTable();
	for ( i=0; i < ARRAY_SIZE(g_SceneOverrides); i++ )
	{
		memmove( &g_SceneOverrides[i].oldVal, g_SceneOverrides[i].pTargetVal, g_SceneOverrides[i].targetBytes );
	}
}

static void setSceneCustomization( RTShadowOverrideID id, void* pVal )
{
	memmove( g_SceneOverrides[id].pTargetVal, pVal, g_SceneOverrides[id].targetBytes );
}

void rt_initShadowMapMenu(void)
{
	int i;
	static const int shadowCullValues[] = {cullNone, cullBack, cullFront};
	static const char * shadowCullNames[] = {"None", "Back", "Front", NULL};
	static const int shadowDirValues[] = {dirSun, dirY, dir30, dir45, dir60, dirZ, dirView, dirTweak };
	static const char * shadowDirNames[] = {"Sun", "-Y", "30 deg", "45 deg", "60 deg", "-Z", "View", "deg tweak", NULL};
	static const int shadowViewBounds[] = {viewBoundSphere, viewBoundTight, viewBoundApprox, viewBoundFixed};
	static const char * shadowViewBoundNames[] = {"Sphere", "Tight", "Approx", "Fixed", NULL};
	static const int shadowSampleModeValues[] = {sampleOne, sampleDither, sample16};
	static const char * shadowSampleModeNames[] = {"1x1", "2x2 dither", "4x4", NULL};
	static const int shadowDeflickerValues[] = {deflickerNone, deflickerStatic, deflickerAlways};
	static const char * shadowDeflickerNames[] = {"none", "static", "always", NULL};
	static const int shadowCompiledShaderSelectionValues[] = {SHADOWSHADER_FAST, SHADOWSHADER_MEDIUMQ, SHADOWSHADER_HIGHQ, SHADOWSHADER_DEBUG,SHADOWSHADER_NONE };
	static const char * shadowCompiledShaderSelectionNames[] = {"fast", "medium", "high quality", "tweakable (very slow)", "none", NULL};
	static const int shadowModeValues[] = {SHADOW_OFF, SHADOW_SHADOWMAP_LOW, SHADOW_SHADOWMAP_MEDIUM, SHADOW_SHADOWMAP_HIGH, SHADOW_SHADOWMAP_HIGHEST};
	static const char * shadowModeNames[] = {"Off", "Low", "Medium", "High", "Highest", NULL};
	static const int stippleFadeModeValues[] = {stippleFadeNever, stippleFadeAlways, stippleFadeHQOnly};
	static const char * stippleFadeModeNames[] = {"Off", "Always", "HQ Only", NULL};	
	static const int shadowSplitCalcModeValues[] = {shadowSplitCalc_DynamicMaxSplits, shadowSplitCalc_Dynamic, shadowSplitCalc_StaticAssignment};
	static const char * shadowSplitCalcModeNames[] = {"Dynamic 4 splits", "Dynamic", "Static Assignment", NULL};	

	initOverrideTable();
	
	g_rtShadowMapOpts.enablePCF = true;
	g_rtShadowMapOpts.cull = cullNone;
	g_rtShadowMapOpts.slopeBias = +8.0f;
	g_rtShadowMapOpts.constantBias = +1.0f;

	g_rtShadowMapOpts.modulationClamp_ambient			= 0.85f;
	g_rtShadowMapOpts.modulationClamp_diffuse			= 0.15f;
	g_rtShadowMapOpts.modulationClamp_specular			= 0.25f;
	g_rtShadowMapOpts.modulationClamp_vertex_lit		= 0.65f;	// for simple materials when not upgraded to per pixel (e.g. RGBS baked lighting)
	g_rtShadowMapOpts.modulationClamp_NdotL_rolloff		= 0.75f;	// extra N*L rolloff to help hide projection errors.
	g_rtShadowMapOpts.modulationClamp_ambient_only_obj	= 0.35f;	// if the obj doesn't use diffuse lighting (e.g. OBJ_NOLIGHTANGLE) then we need to shadow ambient much more
	g_rtShadowMapOpts.nightModulationReduction			= 0.5f;		// how much to subdue shadows at night, a value of 1.0 completely hides nighttime shadows

	g_rtShadowMapOpts.debugDrawAlpha = 0.1;
	g_rtShadowMapOpts.debugShowSplits = 0.0f;
	g_rtShadowMapOpts.debugShowMapSize = 256;
	g_rtShadowMapOpts.debugShowMapOffsetX = 129;
	g_rtShadowMapOpts.debugShowMapOffsetY = 129;
	g_rtShadowMapOpts.debugScale = 100.0f;

	g_rtShadowMapOpts.sampleMode = sample16;
	g_rtShadowMapOpts.lastCascadeFade = 1.0f;

	game_state.shadowSplitCalcType = shadowSplitCalc_DynamicMaxSplits;	// default cascade split calculation style, See comments in csmCalcSplits()

	game_state.shadowDebugFlags |= quantizeScaleOffset;
	game_state.shadowDebugFlags |= renderUsingMaps;
	game_state.shadowDebugFlags |= orientOnView;
	game_state.shadowDebugFlags |= quantizeLocal;

	
	game_state.shadowDebugFlags |= kRenderSkinnedIntoMap;
	game_state.shadowDebugFlags |= kUseCustomRenderLoop;
	game_state.shadowDebugFlags |= kShadowCapsuleCull;
	game_state.shadowDebugFlags |= kUpgradeShadowedToPerPixel;
	game_state.shadowDebugFlags |= kForceFallbackMaterial;

	game_state.shadowStippleOpaqueFadeMode = stippleFadeNever;
	game_state.shadowStippleAlphaFadeMode = stippleFadeHQOnly;

	game_state.shadowWidthMax = 1024.0f;
	game_state.shadowFarMax = 2000.0f;
	game_state.shadowLightSizeZ = 5000.0f;
	game_state.shadowSplit1 = 55.0f;
	game_state.shadowSplit2 = 135.0f;
	game_state.shadowSplit3 = 400.0f;
	game_state.shadowSplitLambda = 0.90f;
	game_state.shadowDirAltitude = 45.0f;	
	game_state.shadowMaxSunAngleDeg = 50.0f;
	game_state.shadowScaleQuantLevels = 64.0f;
	game_state.shadowViewBound = viewBoundTight;
	game_state.shadowDeflickerMode = deflickerAlways;
	game_state.shadowmap_lod_alpha_snap_entities = 210;
	game_state.shadowmap_lod_alpha_snap = 210;
	game_state.shadowmap_lod_alpha_begin = 25;
	game_state.shadowmap_lod_alpha_end = 175;
	game_state.shadowDebugPercent1 = .01;
	game_state.shadowDebugPercent2 = .05;
	game_state.shadowmap_extra_fov = 0;				// degrees to expand y fov when preparing for shadowmap capsule culling

	// amount to overlap cascade rendering/culling so that we can blend at split boundaries
	game_state.shadowmap_split_overlap[0] = 0;
	game_state.shadowmap_split_overlap[1] = 4.0f;
	game_state.shadowmap_split_overlap[2] = 8.0f;
	game_state.shadowmap_split_overlap[3] = 16.0f;

	for (i=0; i<6; ++i)
	{
		game_state.biasScale[i] = 1.0f;
	}
	tunePushMenu("ShadowMap");

	tuneEnum("Enable Shadow Maps", (int*)&game_state.shadowMode, shadowModeValues, shadowModeNames, NULL);
	tuneEnum("Shader Select", (int*)&game_state.shadowShaderSelection, shadowCompiledShaderSelectionValues, shadowCompiledShaderSelectionNames, NULL);
	tuneBit("Render w/ Maps", &game_state.shadowDebugFlags, renderUsingMaps, NULL);

	tuneFloat("Shadow Ambient Clamp", &g_rtShadowMapOpts.modulationClamp_ambient, 0.005f, 0.0f, 1.0f, NULL);
	tuneFloat("Shadow Diffuse Clamp", &g_rtShadowMapOpts.modulationClamp_diffuse, 0.005f, 0.0f, 1.0f, NULL);
	tuneFloat("Shadow Specular Clamp", &g_rtShadowMapOpts.modulationClamp_specular, 0.005f, 0.0f, 1.0f, NULL);
	tuneFloat("Shadow Diffuse Rolloff Clamp", &g_rtShadowMapOpts.modulationClamp_NdotL_rolloff, 0.005f, 0.0f, 1.0f, NULL);
	tuneFloat("Shadow Vertex Lit Clamp", &g_rtShadowMapOpts.modulationClamp_vertex_lit, 0.005f, 0.0f, 1.0f, NULL);
	tuneFloat("Shadow Ambient Only Obj Clamp", &g_rtShadowMapOpts.modulationClamp_ambient_only_obj, 0.005f, 0.0f, 1.0f, NULL);
	
	tuneFloat("Shadow Strength Night Reduction", &g_rtShadowMapOpts.nightModulationReduction, 0.005f, 0.0f, 1.0f, NULL);

	tuneFloat("Max Shadow Vis Z", &game_state.shadowFarMax, 10.0f, 10.0f, 20000.0f, NULL);
	tuneFloat("Light Z Range", &game_state.shadowLightSizeZ, 10.0f, 10.0f, 20000.0f, NULL);

	tuneEnum("Opaque LOD fade mode", (int*)&game_state.shadowStippleOpaqueFadeMode, stippleFadeModeValues, stippleFadeModeNames, NULL);
	tuneEnum("Alpha LOD fade mode", (int*)&game_state.shadowStippleAlphaFadeMode, stippleFadeModeValues, stippleFadeModeNames, NULL);
	tuneInteger("LOD fade begin", &game_state.shadowmap_lod_alpha_begin, 1, 0, 255, NULL);
	tuneInteger("LOD fade end", &game_state.shadowmap_lod_alpha_end, 1, 0, 255, NULL);
	tuneInteger("LOD alpha snap", &game_state.shadowmap_lod_alpha_snap, 1, 0, 255, NULL);
	tuneInteger("LOD alpha snap entities", &game_state.shadowmap_lod_alpha_snap_entities, 1, 0, 255, NULL);

	tuneInteger("Number of Cascades", &game_state.shadowmap_num_cascades, 1, 0, SHADOWMAP_MAX_SHADOWS, NULL);
	tuneEnum("Cascade Split Mode", (int*)&game_state.shadowSplitCalcType, shadowSplitCalcModeValues, shadowSplitCalcModeNames, NULL);
	tuneInteger("Map Texture Size", &game_state.shadowMapTexSize, 512, 512, 4096, NULL);
	tuneEnum("Deflicker Mode", (int*)&game_state.shadowDeflickerMode, shadowDeflickerValues, shadowDeflickerNames, NULL);
	tuneFloat("Fade Over Last Cascade", &g_rtShadowMapOpts.lastCascadeFade, 0.01f, 0.01f, 1.0f, NULL);
	tuneFloat("Split Blend 2", &(game_state.shadowmap_split_overlap[1]), 0.1f, 0.0, 50.0f, NULL);
	tuneFloat("Split Blend 3", &(game_state.shadowmap_split_overlap[2]), 0.1f, 0.0, 50.0f, NULL);
	tuneFloat("Split Blend 4", &(game_state.shadowmap_split_overlap[3]), 0.1f, 0.0, 50.0f, NULL);

	tuneEnum("Light Dir", (int*)&game_state.shadowDirType, shadowDirValues, shadowDirNames, NULL);
	tuneFloat("Light Altitude", &game_state.shadowDirAltitude, 1.0f, 0.0, 89.0f, NULL);
	tuneFloat("Sun Angle Clamp", &game_state.shadowMaxSunAngleDeg, 1.0f, 1.0, 90.0f, NULL);

	tuneBit("Test Toggle", &game_state.shadowDebugFlags, kShadowTestToggle, NULL);

	tuneFloat("Split Lambda", &game_state.shadowSplitLambda, 0.01f, 0.0f, 1.0f, NULL);
	tuneFloat("Slope Bias", &g_rtShadowMapOpts.slopeBias, 0.1f, -100.0f, 100.0f, NULL);
	tuneFloat("Constant Bias", &g_rtShadowMapOpts.constantBias, 0.1f, -100.0f, 100.0f, NULL);

	tunePushMenu("Debugging");
		tuneBit("Test Toggle", &game_state.shadowDebugFlags, kShadowTestToggle, NULL);
		tuneBit("Orient On View", &game_state.shadowDebugFlags, orientOnView, NULL);
		tuneBit("Cull via Capsule", &game_state.shadowDebugFlags, kShadowCapsuleCull, NULL);
		tuneFloat("Capsule Cull Extra FOV (degrees)", &game_state.shadowmap_extra_fov, 0.01f, 0.0f, 90.0f, NULL);

		tuneBit("Draw Skinned Into Map", &game_state.shadowDebugFlags, kRenderSkinnedIntoMap, NULL);
		tuneBit("Draw Skinned in Interior", &game_state.shadowDebugFlags, kRenderSkinnedInteriorIntoMap, NULL);
		tuneBit("Upgrade Shadowed Per Pixel", &game_state.shadowDebugFlags, kUpgradeShadowedToPerPixel, NULL);
		tuneBit("Upgrade Always Per Pixel", &game_state.shadowDebugFlags, kUpgradeAlwaysToPerPixel, NULL);
		tuneBit("Use fallback materials", &game_state.shadowDebugFlags, kForceFallbackMaterial, NULL);

		tuneBit("Freeze Shadows", &game_state.shadowMapFreeze, 0x1, NULL);
		tuneBit("Show Maps", &game_state.shadowMapDebug, showMaps2D, NULL);
		tuneInteger("Show Map Size", &g_rtShadowMapOpts.debugShowMapSize, 32, 32, 2048, NULL);
		tuneInteger("Show Map Offset X", &g_rtShadowMapOpts.debugShowMapOffsetX, 1, -2048, 2048, NULL);
		tuneInteger("Show Map Offset Y", &g_rtShadowMapOpts.debugShowMapOffsetY, 1, -2048, 2048, NULL);
		tuneBit("Show Axis", &game_state.shadowMapDebug, showAxis, NULL);
		tuneBit("Show Frustum", &game_state.shadowMapDebug, showFrustum, NULL);
		tuneBit("Show Splits", &game_state.shadowMapDebug, showSplits, NULL);
		tuneBit("Show Overview", &game_state.shadowMapDebug, showOverview2D, NULL);
		tuneBit("Show Volumes", &game_state.shadowMapDebug, showVolumes3D, NULL);
		tuneBit("Show Extruded Casters", &game_state.shadowMapDebug, showExtrudeCasters, NULL);
		tuneBit("Show View Frust In Map", &g_rtShadowMapOpts.debugFlags, kShowViewFrustInMap, NULL);
		tuneEnum("View Bound", (int*)&game_state.shadowViewBound, shadowViewBounds, shadowViewBoundNames, NULL);
		tuneFloat("Debug Alpha", &g_rtShadowMapOpts.debugDrawAlpha, 0.05f, 0.0f, 1.0f, NULL);
		tuneFloat("Debug Scale", &g_rtShadowMapOpts.debugScale, 2.0f, 1.0f, 4000.0f, NULL);
		tuneFloat("Debug Percent 1", &game_state.shadowDebugPercent1, 0.001f, 0.0f, 2.0f, NULL);
		tuneFloat("Debug Percent 2", &game_state.shadowDebugPercent2, 0.001f, 0.0f, 2.0f, NULL);

		tuneFloat("Scale Quant Levels", &game_state.shadowScaleQuantLevels, 1.0f, 0.0f, 1024.0f, NULL);

		tuneEnum("Culling", (int*)&g_rtShadowMapOpts.cull, shadowCullValues, shadowCullNames, NULL);
		tuneEnum("PCF Shader", (int*)&g_rtShadowMapOpts.sampleMode, shadowSampleModeValues, shadowSampleModeNames, NULL);
		tuneBoolean("PCF on Fetch", &g_rtShadowMapOpts.enablePCF, NULL);
		//	tuneBit("Quantize Enable", &game_state.shadowDebugFlags, quantizeScaleOffset, NULL);
		tuneBit("Local Quant", &game_state.shadowDebugFlags, quantizeLocal, NULL);
		tuneFloat("Light Size XY", &game_state.shadowWidthMax, 1.0f, 10.0f, 20000.0f, NULL);
		tuneInteger("Number of Cascades", &game_state.shadowmap_num_cascades, 1, 0, SHADOWMAP_MAX_SHADOWS, NULL);
		tuneEnum("Cascade Split Mode", (int*)&game_state.shadowSplitCalcType, shadowSplitCalcModeValues, shadowSplitCalcModeNames, NULL);
		tuneFloat("split 1 z", &game_state.shadowSplit1, 1.0f, 1.0f, 2000.0f, NULL);
		tuneFloat("split 2 z", &game_state.shadowSplit2, 1.0f, 1.0f, 2000.0f, NULL);
		tuneFloat("split 3 z", &game_state.shadowSplit3, 1.0f, 1.0f, 2000.0f, NULL);
		tuneFloat("Max Shadow Vis Z", &game_state.shadowFarMax, 10.0f, 10.0f, 20000.0f, NULL);
		tuneFloat("Light Z Range", &game_state.shadowLightSizeZ, 10.0f, 10.0f, 20000.0f, NULL);
		tuneFloat("Split Blend 2", &(game_state.shadowmap_split_overlap[1]), 0.1f, 0.0, 50.0f, NULL);
		tuneFloat("Split Blend 3", &(game_state.shadowmap_split_overlap[2]), 0.1f, 0.0, 50.0f, NULL);
		tuneFloat("Split Blend 4", &(game_state.shadowmap_split_overlap[3]), 0.1f, 0.0, 50.0f, NULL);
		tuneFloat("Show Split Strength", &g_rtShadowMapOpts.debugShowSplits, 0.05f, 0.0f, 1.0f, NULL);
		tuneBit("Fast map gen", &game_state.shadowDebugFlags, kUseCustomRenderLoop, NULL);
		tuneBit("Single Drawcall Full Models", &game_state.shadowDebugFlags, kSingleDrawWhite, NULL);

		tuneFloat("Bias Scale 0", &game_state.biasScale[0], 0.1f, -100.0f, 100.0f, NULL);
		tuneFloat("Bias Scale 1", &game_state.biasScale[1], 0.1f, -100.0f, 100.0f, NULL);
		tuneFloat("Bias Scale 2", &game_state.biasScale[2], 0.1f, -100.0f, 100.0f, NULL);
		tuneFloat("Bias Scale 3", &game_state.biasScale[3], 0.1f, -100.0f, 100.0f, NULL);

		//	tuneBit("Draw Direct", &game_state.shadowDebugFlags, kShadowDrawDirect, NULL);
		//	tuneBit("Sort Draw Direct", &game_state.shadowDebugFlags, kShadowSortDrawDirect, NULL);
		//	tuneBit("No Thread Queue", &game_state.shadowDebugFlags, kNoQueueThread, NULL);
		//	tuneBit("Use New Model Thread Draw", &game_state.shadowDebugFlags, kNewThreadModelCmd, NULL);
		//	tuneBit("Block Thread Render", &game_state.shadowDebugFlags, kBlockRenderShadowMap, NULL);
		//	tuneBit("Main View Relative Vis/LOD", &game_state.shadowDebugFlags, kMainViewRelativeVis, NULL);
		//	tuneBit("Use Instance Counts", &g_rtShadowMapOpts.debugFlags, kDrawInstanced, NULL);
		//	tuneBit("Sort Models", &game_state.shadowDebugFlags, kSortModels, NULL);
		//	tuneBit("Sort Shadow Custom", &game_state.shadowDebugFlags, kSortShadowModelsCustom, NULL);
		//	tuneBit("Sort Custom", &game_state.shadowDebugFlags, kSortModelsCustom, NULL);
	tunePopMenu();

	tuneCallback("Reset to Defaults", rt_initShadowMapMenu);

	tunePopMenu();
	
	setBaseSceneCustomizations();
}

// update the shadowmap settings for the current scene
void rt_shadowmap_scene_customizations( void* SceneInfo_p )
{
	SceneInfo* sceneInfo = (SceneInfo*)SceneInfo_p;

	// We don't want to carry forward the customizations from the previous scene (if any),
	// so restore the last pre-customized values set in rt_initShadowMapMenu().
	clearSceneCustomizations();
	
	// If the supplied value equals the default "ignore" value (e.g., -1.0f) it's not a real
	// override and should be ignored.
	CustomizeIfValid( kShadowMapOverride_Off,							&sceneInfo->shadowMaps_Off,								-1 );
	CustomizeIfValid( kShadowMapOverride_modulationClamp_ambient,		&sceneInfo->shadowMaps_modulationClamp_ambient,			-1.0f );
	CustomizeIfValid( kShadowMapOverride_modulationClamp_diffuse,		&sceneInfo->shadowMaps_modulationClamp_diffuse,			-1.0f );
	CustomizeIfValid( kShadowMapOverride_modulationClamp_specular,		&sceneInfo->shadowMaps_modulationClamp_specular,		-1.0f );
	CustomizeIfValid( kShadowMapOverride_modulationClamp_vertex_lit,	&sceneInfo->shadowMaps_modulationClamp_vertex_lit,		-1.0f );
	CustomizeIfValid( kShadowMapOverride_modulationClamp_NdotL_rolloff,	&sceneInfo->shadowMaps_modulationClamp_NdotL_rolloff,	-1.0f );
	CustomizeIfValid( kShadowMapOverride_lastCascadeFade,				&sceneInfo->shadowMaps_lastCascadeFade,					-1.0f );
	CustomizeIfValid( kShadowMapOverride_farMax,						&sceneInfo->shadowMaps_farMax,							-1.0f );
	CustomizeIfValid( kShadowMapOverride_splitLambda,					&sceneInfo->shadowMaps_splitLambda,						-1.0f );
	CustomizeIfValid( kShadowMapOverride_maxSunAngleDegrees,			&sceneInfo->shadowMaps_maxSunAngleDeg,					-1.0f );
}


// pass the desired stipple alpha and the current stipple enable state
// this function will enable/disable stipple as appropriate and return the
// new state.
static bool rt_shadowmap_set_stipple( U8 alpha, bool bStippleEnabled )
{
	if (alpha != 0 && alpha != 255)
	{
		extern U8 *getStipplePatternSimple(int alpha);
		// TODO: Remove this once ATI memory leak regarding glEnable(GL_POLYGON_STIPPLE) is fixed (11/06/09)
		if (!(rdr_caps.chip & R200) || game_state.ati_stipple_leak)
		{
			if (!bStippleEnabled)
			{
				glEnable(GL_POLYGON_STIPPLE); CHECKGL;
				bStippleEnabled = true;
			}
			glPolygonStipple(getStipplePatternSimple(alpha)); CHECKGL;
		}
	}
	else if (bStippleEnabled)
	{
		glDisable(GL_POLYGON_STIPPLE); CHECKGL;
		bStippleEnabled = false;
	}
	return bStippleEnabled;
}

/*
Game engine prepares a block of data which contains all the data necessary
to render an entire shadowmap to an offscreen buffer. The majority of this
is the model batches and their xform matrices.

The engine thread which prepares the data collects and sorts it all in order
to minimize state changes (e.g. we batch up the instances which use the same model).
So it is efficient to pass that entire block of data down to the render thread and
let it chew on it. While this uses more memory in the rt queue it greatly increases
task granularity and reduces losses to overhead and synchronization.

It also makes the shadow map rendering simpler and thus easier to understand and maintain.
*/

void rt_shadowmap_render( void* data )
{
	RdrShadowmapItemsHeader* pHeader = (RdrShadowmapItemsHeader*)data;
	unsigned int i;
	int totalTris = 0;
	int	vboChanges = 0;
	int draw_calls = 0;
	int s_last_vtx_id = 0;
	int s_last_idx_id = 0;
	Vec3 *s_last_verts = (Vec3*)0xffffffff;
	int total_items = pHeader->batch_count_opaque + pHeader->batch_count_alphatest + pHeader->batch_count_skinned;

	rdrStatsClear(game_state.stats_cards);	// reset stats for this pass

	if ( total_items == 0 )
	{
		return; // bail out if nothing to do
	}

	rdrBeginMarker(__FUNCTION__ ": total batches [%d]", total_items );

	//***
	// Set common state once before looping over items
	// For now we try to use fixed function for speed
	WCW_DisableTexture( TEXLAYER_BASE );
	texEnableTexCoordPointer(TEXLAYER_BASE, false);

	WCW_DisableTexture( TEXLAYER_GENERIC );
	texEnableTexCoordPointer(TEXLAYER_GENERIC, false);

	WCW_DisableTexture( TEXLAYER_BUMPMAP1 );

	WCW_EnableClientState(GLC_VERTEX_ARRAY);
	WCW_DisableClientState(GLC_COLOR_ARRAY);
	WCW_DisableClientState(GLC_NORMAL_ARRAY);

	WCW_DisableClientState( GLC_VERTEX_ATTRIB_ARRAY1_NV );
	WCW_DisableClientState( GLC_VERTEX_ATTRIB_ARRAY5_NV );
	WCW_DisableClientState( GLC_VERTEX_ATTRIB_ARRAY6_NV );
	WCW_DisableClientState( GLC_VERTEX_ATTRIB_ARRAY7_NV );
	WCW_DisableClientState( GLC_VERTEX_ATTRIB_ARRAY11_NV );

	WCW_EnableGL_LIGHTING(0);
	WCW_EnableGL_NORMALIZE(0);

	WCW_DisableVertexProgram();
	WCW_DisableFragmentProgram();

	// Need to call this so that the shader programs actually get disabled
	// @todo make sure it's cheap in this case
	WCW_PrepareToDraw();

	//****
	// Render all the models/instances

	//****
	// First submit the solid opaque world models (no skinning, no alpha test)
	if ( pHeader->batch_count_opaque)
	{
		bool bStippleEnabled = false;
		RdrShadowmapModelOpaque* pItem = (RdrShadowmapModelOpaque*)(pHeader+1);
		rdrBeginMarker(__FUNCTION__ ": Opaque [%d]", pHeader->batch_count_opaque );

		// @todo alpha test state isn't managed which is probably cause
		// of flickering grass, etc.
		glDisable(GL_ALPHA_TEST); CHECKGL;

		for (i=0; i < pHeader->batch_count_opaque; ++i, pItem++ )
		{
			VBO* vbo = pItem->vbo;
			int batchTris = pItem->tri_count;
			int ele_count = batchTris*3;

			onlydevassert( pItem->tri_count <= vbo->tri_count );

			if (vbo->element_array_id != s_last_idx_id)
			{
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo->element_array_id); CHECKGL;
				s_last_idx_id = vbo->element_array_id;
			}
			if (vbo->vertex_array_id != s_last_vtx_id)
			{
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->vertex_array_id); CHECKGL;
				s_last_vtx_id = vbo->vertex_array_id;
				s_last_verts = (Vec3*)0xffffffff;
			}
			if (vbo->verts != s_last_verts)
			{
				glVertexPointer( 3,GL_FLOAT,0,vbo->verts ); CHECKGL;
				s_last_verts = vbo->verts;
			}

			{
				Mat44 local_modelview_matrix_4x4;
				mat43to44(pItem->xform, local_modelview_matrix_4x4);
				glLoadMatrixf((F32 *)local_modelview_matrix_4x4); CHECKGL;
			}

#ifndef FINAL
			bStippleEnabled = rt_shadowmap_set_stipple( pItem->lod_alpha, bStippleEnabled );
#endif
			#undef glDrawElements		// Call OpenGL directly and not through CoH engine wrapper
			glDrawElements(GL_TRIANGLES,ele_count,GL_UNSIGNED_INT,vbo->tris); CHECKGL;

			totalTris += batchTris;
		}
		glEnable(GL_ALPHA_TEST); CHECKGL;
		glDisable(GL_POLYGON_STIPPLE); CHECKGL;
		rdrEndMarker();
	}

	//**
	// Next we submit the static models that use alpha test (e.g. tree canopies)
	if (pHeader->batch_count_alphatest)
	{
		bool bStippleEnabled = false;
		RdrShadowmapModelAlphaTest* pItem = (RdrShadowmapModelAlphaTest*)((RdrShadowmapModelOpaque*)(pHeader+1) + pHeader->batch_count_opaque);
		rdrBeginMarker(__FUNCTION__ ": Alpha Test [%d]", pHeader->batch_count_alphatest );

		WCW_EnableTexture( GL_TEXTURE_2D, TEXLAYER_BASE );
		texEnableTexCoordPointer(TEXLAYER_BASE, true);
		glEnable(GL_ALPHA_TEST); CHECKGL;
		glAlphaFunc(GL_GREATER, 0.6); CHECKGL;

		for( i = 0 ; i < pHeader->batch_count_alphatest; i++, pItem++ ) 
		{
			if (pItem->tex_id)
			{
				VBO* vbo = pItem->vbo;
				int batchTris = pItem->tri_count;
				int ele_count = batchTris*3;
				int ele_base = pItem->tri_start*3;

				onlydevassert( pItem->tri_count <= vbo->tri_count );

				if (vbo->element_array_id != s_last_idx_id)
				{
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo->element_array_id); CHECKGL;
					s_last_idx_id = vbo->element_array_id;
				}
				if (vbo->vertex_array_id != s_last_vtx_id)
				{
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->vertex_array_id); CHECKGL;
					s_last_vtx_id = vbo->vertex_array_id;
					s_last_verts = (Vec3*)0xffffffff;
				}
				if (vbo->verts != s_last_verts)
				{
					glVertexPointer( 3,GL_FLOAT,0,vbo->verts ); CHECKGL;
					s_last_verts = vbo->verts;
				}

				// setup texture state for alpha test
				texBind( TEXLAYER_BASE, pItem->tex_id );
				texBindTexCoordPointer(TEXLAYER_BASE, pItem->tex_coord_idx == 1 ? vbo->sts2:vbo->sts );

				{
					Mat44 local_modelview_matrix_4x4;
					mat43to44(pItem->xform, local_modelview_matrix_4x4);
					glLoadMatrixf((F32 *)local_modelview_matrix_4x4); CHECKGL;
				}
				
#ifndef FINAL
				bStippleEnabled = rt_shadowmap_set_stipple( pItem->lod_alpha, bStippleEnabled );
#endif

				#undef glDrawElements		// Call OpenGL directly and not through CoH engine wrapper
				glDrawElements(GL_TRIANGLES,ele_count,GL_UNSIGNED_INT,&(vbo->tris[ele_base])); CHECKGL;

				totalTris += batchTris;
			}
		}

		glAlphaFunc(GL_GREATER, 0); CHECKGL;
		glDisable(GL_POLYGON_STIPPLE); CHECKGL;
		rdrEndMarker();
	}

	//**
	// Next we submit the skinned models (opaque)
	if (pHeader->batch_count_skinned)
	{
		RdrShadowmapModelAlphaTest* pItemAT = (RdrShadowmapModelAlphaTest*)((RdrShadowmapModelOpaque*)(pHeader+1) + pHeader->batch_count_opaque);
		RdrShadowmapModelSkinned*		pItem = (RdrShadowmapModelSkinned*)(pItemAT + pHeader->batch_count_alphatest);
		rdrBeginMarker(__FUNCTION__ ": Skinned [%d]", pHeader->batch_count_skinned );

		WCW_DisableTexture( TEXLAYER_BASE );
		texEnableTexCoordPointer(TEXLAYER_BASE, false);
		glDisable(GL_ALPHA_TEST); CHECKGL;
		glAlphaFunc(GL_GREATER, 0.0); CHECKGL;

		// setup simple skinning shader for shadowmap write
		{
			WCW_EnableVertexProgram();
			WCW_BindVertexProgram(shaderMgrVertexPrograms[DRAWMODE_DEPTH_SKINNED]);
			WCW_PrepareToDraw();
		}

		{
			Mat44 local_modelview_matrix_4x4;
			identityMat44( local_modelview_matrix_4x4 );
			glLoadMatrixf((F32 *)local_modelview_matrix_4x4); CHECKGL;
		}
		
		// enable weight and matrix varying vertex attributes from array
		// @todo really should change these const names as they are ARB and not NV specific any longer
		WCW_EnableClientState( GLC_VERTEX_ATTRIB_ARRAY1_NV );	// BLENDWEIGHT, ATTR1 weights
		WCW_EnableClientState( GLC_VERTEX_ATTRIB_ARRAY5_NV ); // bone matrix indices (use BLENDINDICES (ATTR7) instead?)

		for( i = 0 ; i < pHeader->batch_count_skinned; ++i ) 
		{
			{
				VBO* vbo = pItem->vbo;
				int batchTris = vbo->tri_count;
				int ele_count = batchTris*3;
				unsigned int mtx_block_size = pItem->num_bones * sizeof(Vec4)*3; // we send bones as 3x4 transposed

				if (vbo->element_array_id != s_last_idx_id)
				{
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo->element_array_id); CHECKGL;
					s_last_idx_id = vbo->element_array_id;
				}
				if (vbo->vertex_array_id != s_last_vtx_id)
				{
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->vertex_array_id); CHECKGL;
					s_last_vtx_id = vbo->vertex_array_id;
					s_last_verts = (Vec3*)0xffffffff;
				}
				if (vbo->verts != s_last_verts)
				{
					glVertexPointer( 3,GL_FLOAT,0,vbo->verts ); CHECKGL;
					s_last_verts = vbo->verts;
				}

				// setup varying vertex attributes, bone matrix indices and weights
				glVertexAttribPointerARB(1, 2, GL_FLOAT, GL_FALSE, 0, vbo->weights); CHECKGL; CHECKGL;
				glVertexAttribPointerARB(5, 2, GL_SHORT, GL_FALSE, 0, vbo->matidxs); CHECKGL; CHECKGL;

				// transfer the skinning matrix constants to the shader
				// more efficient with a uniform buffer block transfer?
				{
					int numConst = pItem->num_bones * 3;
					int nArbRegister = 8;						// hard coded in shader at local param c8 currently
					void* pBoneXforms = pItem + 1;				// bone matrices follow immediately after the item header

					// do the entire constant block w/ a single call if we can
					if ( GLEW_EXT_gpu_program_parameters )
					{
						rdrSetMarker("glProgramLocalParameters4fvEXT %d", numConst); // not intercepted by gdebugger?

						glProgramLocalParameters4fvEXT(GL_VERTEX_PROGRAM_ARB, nArbRegister, numConst, pBoneXforms ); CHECKGL;
					}
					else
					{
						int j;

						for (j=0; j<numConst; ++j)
						{
							glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, nArbRegister++, pBoneXforms); CHECKGL;
							pBoneXforms = (char*)pBoneXforms + sizeof(Vec4);
						}
					}
				}

				#undef glDrawElements		// Call OpenGL directly and not through CoH engine wrapper
				glDrawElements(GL_TRIANGLES,ele_count,GL_UNSIGNED_INT,vbo->tris); CHECKGL;

				totalTris += batchTris;

				// skip to next item to render
				pItem = (RdrShadowmapModelSkinned*)( (char*)pItem + sizeof(RdrShadowmapModelSkinned) + mtx_block_size );
			}
		}

		glEnable(GL_ALPHA_TEST); CHECKGL;
		rdrEndMarker();
	}

	//****
	// Update stats for this block of models
	RT_STAT_ADD(tri_count, totalTris);
	RT_STAT_ADD(drawcalls, total_items);
	RT_STAT_ADD(worldmodel_drawn, pHeader->batch_count_opaque + pHeader->batch_count_alphatest);
	RT_STAT_ADD(bonemodel_drawn,pHeader->batch_count_skinned);


#ifndef FINAL
	// @todo #ifdef this for debug and add proper state push and/or tracker dirty
	if (g_rtShadowMapOpts.debugFlags & kShowViewFrustInMap)
	{
		Mat44 local_modelview_matrix_4x4;
		glPushAttrib(GL_ALL_ATTRIB_BITS); CHECKGL;

		glMatrixMode(GL_MODELVIEW); CHECKGL;
		glPushMatrix(); CHECKGL;
		mat43to44(rdr_view_state.viewmat, local_modelview_matrix_4x4);

		glMatrixMode(GL_MODELVIEW); CHECKGL;
		glLoadMatrixf((F32 *)local_modelview_matrix_4x4); CHECKGL;
		glDisable(GL_CULL_FACE); CHECKGL;
		WCW_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		drawShadowedViewFrustum();

		glPopAttrib(); CHECKGL;
		glPopMatrix(); CHECKGL;

		WCW_FetchGLState();
		WCW_ResetState();
	}

	// collect render thread stats
	{
		RTStats* pStats = rdrStatsGetPtr(RTSTAT_TYPE, rdr_view_state.renderPass);
		rdrStatsGet( pStats );
		rdrStatsClear(game_state.stats_cards);
	}
#endif

	rdrEndMarker();
}
