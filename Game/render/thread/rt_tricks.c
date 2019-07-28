#define RT_PRIVATE
#include "rt_tricks.h"
#include "rt_prim.h"
#include "rt_model.h"
#include "rt_cgfx.h"
#include "ogl.h"
#include "wcw_statemgmt.h"
#include "rt_state.h"
#include "mathutil.h"
#include "tricks.h"
#include "animtrackanimate.h"
#include "HashFunctions.h"
#include "Quat.h"
#include "failtext.h"
#include "cmdgame.h"

Vec4	tex_scrolls[4];

void FaceCamMat(Mat4 mat, F32 maxpitch)
{
	Vec3 dvec;
	F32 yaw0,pitch0;
	F32 yaw,pitch;

	dvec[0] = -mat[3][0]; dvec[1] = -mat[3][1]; dvec[2] = -mat[3][2];
	if (maxpitch) {
		getVec3YP(mat[2], &yaw0, &pitch0);
		getVec3YP(dvec, &yaw, &pitch);
		yaw = yaw0 - yaw;
		pitch =  pitch - pitch0;
		if (maxpitch > 0) {
			if (pitch > maxpitch) pitch = maxpitch;
			else if (pitch < -maxpitch) pitch = -maxpitch;
		}
		yawMat3(yaw, mat);
		pitchMat3(pitch, mat);
	} else {
		yaw0 = getVec3Yaw(mat[2]);
		yaw = yaw0 - getVec3Yaw(dvec);
		yawMat3(yaw, mat);
	}
}

int animateStsTexture( Mat4 result, F32 time, const BoneAnimTrack * bt, F32 stScale, int frameSnap )
{
	int  intTime;
	F32  remainderTime;
	Quat rotation = {0};
	Vec3 position = {0};
	int animationLength;
	int frameCycles;
	F32 frameTime;

	animationLength = (MAX( bt->rot_count, bt->pos_count )) - 1;

	if( !bt || !animationLength )
	{
		copyMat4( unitmat, result );
		return 0;
	}
	
	//TO DO is this nonsense or is frame wrapping correctly?
	
		// Number of times this animation would have run had it been started at time zero.
	frameCycles = (int)time / animationLength;
		// How far the current time is beyond the most recent theoretical starting time --
		// again, based on animations cycles beginning at time zero.
	frameTime = (F32)time - (F32)((F32)frameCycles * animationLength);

	intTime =(int)frameTime; //this split is a dumb performance thing from the character animation
	remainderTime = frameTime - (F32)intTime; //just pretend you are using time

	animGetRotationValue( intTime, remainderTime, bt, rotation, frameSnap ); 
	animGetPositionValue( intTime, remainderTime, bt, position, frameSnap ); 

	//xyprintf( 30, 30, "FrameTime: %f", frameTime ); 
	//TO DO 
	//useRotation adn position to make a good sts matrix

	{
		Vec3	v,vx;
		F32		t, ut;
	
		//Swizzle rotation and position because MAX y is GL z, and artists set it up wrong
		
		//Swizzle rotation. 
		t = quatZ(rotation);
		quatZ(rotation) = quatY(rotation);
		quatY(rotation) = t;
		quatToMat(rotation, result);

		// bias rotation to be around s,t of 0.5 
		v[0] = -0.5;   
		v[1] = -0.5; 
		v[2] = 0;
		mulVecMat3( v, result, vx ); //this doesn't seem to be in the right place, but I'm only interested in backwards compatibility

		//scale position by stScale set in trick file	
		position[0] = stScale * position[0]; 
		position[2] = stScale * position[2]; 
		t = stScale * position[1];
		ut = fabs(t)/10.f + 1.f;
		if (t < 0)
			ut = 1.f / ut;
		position[1] = ut;

		//restore rotation bias, translate and scale  (notice swizzling position, and MAX y is used as scale 
		result[3][0] = vx[0] + ( 0.5 - position[0] );    
		result[3][1] = vx[1] + ( 0.5 - position[2] ); 
		result[3][2] = vx[2];   

		//copyMat3( unitmat, result ); 

		scaleMat3( result, result, position[1] );      
	}

	return 1;
}


//Use an animation to transform the texture matrices (StAnim is parsed in trickInfo, and animation is bound in setStAnim() )
void animateSts( StAnim * stAnim, F32* anim_age_p )
{
	int interpolate = (stAnim->flags & STANIM_FRAMESNAP ? 0 : 1);
	
	//
	// Unless the "LOCAL_TIMER" flag has been set in the trick for the StAnim, e.g...
	//
	//		StAnim 25.0 1.0 Example_TScroll_4x4_Snap FRAMESNAP,LOCAL_TIMER
	//
	// ...the global "client loop timer" is used. If we want the opposite default
	// we can change the code below to check for STANIM_GLOBAL_TIMER _not_ being set.
	// That would make the value of *anim_age_p the default if neither flag were set.
	//
	F32 time = ((( stAnim->flags & STANIM_LOCAL_TIMER ) && ( anim_age_p )) ? 
					*anim_age_p : rdr_view_state.client_loop_timer )
				* stAnim->speed_scale; 
				
	//don't bother supporting ping pong, just tell the artists if they want ping pong, 
	//put it in the animation, the lazy bastards.
	
	if( stAnim->btTex0 )   
	{ 
		Mat4 tex0AnimMat;
		animateStsTexture( tex0AnimMat, time, stAnim->btTex0, stAnim->st_scale, interpolate );
		WCW_LoadTextureMatrix( 0, tex0AnimMat ); //GL_TEXTURE0_ARB
	}

	if( stAnim->btTex1 ) 
	{
		Mat4 tex1AnimMat;
		animateStsTexture( tex1AnimMat, time, stAnim->btTex1, stAnim->st_scale, interpolate );
		WCW_LoadTextureMatrix( 1, tex1AnimMat ); //GL_TEXTURE1_ARB
	}

	//reset values back to expected. The fact that I gotta do this means there's a bug somewhere in our state manager
	glMatrixMode(GL_MODELVIEW); CHECKGL;
}

static void scrollTex( int tex_idx, Vec3 texScrollAmt )
{
	tex_scrolls[tex_idx][0] = texScrollAmt[0] * rdr_view_state.client_loop_timer;
	tex_scrolls[tex_idx][1] = texScrollAmt[1] * rdr_view_state.client_loop_timer;
	tex_scrolls[tex_idx][2] = 0.0f;
	WCW_TranslateTextureMatrix(tex_idx, tex_scrolls[tex_idx]);
}

static void scaleTex( Vec3 scale ) 
{
	WCW_ScaleTextureMatrix(1, scale);
	WCW_ScaleTextureMatrix(0, scale);
}

int alpha_write_mask=0;
void rdrSetAdditiveAlphaWriteMaskDirect(int on)
{
	alpha_write_mask = on;
}


Vec2 fog_dist;
int fogWasDeferring = 0;

/*needs to be after gfxNodeSetAlpha, after light is set, after color set set, after 
constant combiner colors are set, and after TEXLodBias is set because it can change them.  
Maybe rework so all these only get set once, instead of set, then changed by gfxNodeTricks
(Before the mat gets set)

Returns 0 if the object shouldn't be drawn
*/
int gfxNodeTricks(TrickNode *tricks, VBO *vbo, RdrModel *draw, BlendModeType blend_mode, bool is_fallback_material)
{
	int			flags1;
	int			flags2;
	bool		bDidStipple;

	flags1 = (tricks?tricks->flags1:0) | (rdr_view_state.force_double_sided?TRICK_DOUBLESIDED:0);
	flags2 = tricks?tricks->flags2:0;

	if (!flags1 && !flags2)
		return 1;

	if (flags1 & TRICK_NIGHTLIGHT) 
	{
		int	t;

		t = rdr_view_state.lamp_alpha_byte;
		if (curr_blend_state.shader != BLENDMODE_ADDGLOW)
		{
			if (!t)
				return 0;
			// Moved to drawModel():
			//modelSetAlpha((U8)(t * constColor0[3]));
		}
		else
		{
			// NightLight does nothing on AddGlow shaders
		}
	}

	//note animating sts doesn't blow away scrolling or scaling from the trick file anymore
	if ( flags2 & TRICK2_STANIMATE)   
	{
		assert( tricks->info && tricks->info->stAnim ); 
		animateSts( *tricks->info->stAnim, &tricks->st_anim_age );
	}
	 
	if (flags2 & TRICK2_STSCROLL0)        
		scrollTex(0, tricks->info->texScrollAmt[0] );
	if (flags2 & TRICK2_STSCROLL1)
		scrollTex(1, tricks->info->texScrollAmt[1] );

	if ( flags2 & TRICK2_STSSCALE )
	{
		Vec3 scale = {1,tricks->tex_scale_y,1};
		scaleTex(scale);
	}
	
	if (flags1 & TRICK_ADDITIVE)
	{
		WCW_BlendFuncPush(GL_SRC_ALPHA, GL_ONE);
		// Need to disable fog on additive
		if ((rdr_caps.chip & NV1X) && !(rdr_caps.chip & ARBFP)) {
			if (!(rdr_caps.chip & NV2X) && blend_mode.shader == BLENDMODE_COLORBLEND_DUAL &&  blend_mode.blend_bits & BMB_NV1XWORLD) {
				glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB); CHECKGL;
			} else {
				glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB); CHECKGL;
			}
		} else {
			WCW_FogPush(0);
		}
		if (alpha_write_mask) { // Don't write alpha 
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); CHECKGL;
		}
	}
	if (flags1 & TRICK_SUBTRACTIVE)
	{
		if (rdr_caps.supports_subtract) {
			glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT); CHECKGL;
			WCW_BlendFuncPush(GL_SRC_ALPHA, GL_ONE);
		} else {
			// this does not subtract, but fakes it by masking
			WCW_BlendFuncPush(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		}
		// Need to disable fog on additive
		if ((rdr_caps.chip & NV1X) && !(rdr_caps.chip & ARBFP)) {
			glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB); CHECKGL;
		} else {
			WCW_FogPush(0);
		}
		if (alpha_write_mask) { // Don't write alpha 
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); CHECKGL;
		}
	}
	if ( (flags1 & TRICK_FRONTYAW) && draw) {
		FaceCamMat(draw->mat,0.f);
	}
	if ( (flags1 & TRICK_LIGHTFACE) && draw)
	{
		F32		yaw,yaw0;
		Mat4	matx;
		Vec3	dv;

		copyVec3(rdr_view_state.shadow_direction,dv);
		dv[2] = -dv[2];
		mulMat3(rdr_view_state.inv_viewmat,draw->mat,matx);
		yaw0 = getVec3Yaw(matx[2]);
		yaw = -yaw0 - getVec3Yaw(dv);
		yawMat3(yaw, draw->mat);
	}
	if ( (flags1 & TRICK_FRONTFACE ) && draw)
	{
		F32	scale,offset,near_z;
		if (tricks->info && tricks->info->tighten_up==0) // Set default
			tricks->info->tighten_up=3;

		copyMat3(unitmat,draw->mat);
		draw->mat[0][0] = -1;
		near_z = -draw->mat[3][2] - 1.2;
		offset = MIN(near_z,tricks->info?tricks->info->tighten_up:3);
		if (offset > 0)
		{
			scale = (draw->mat[3][2] + offset) / draw->mat[3][2];
			scaleVec3(draw->mat[3],scale,draw->mat[3]);
		}
	}
	if (flags1 & TRICK_DOUBLESIDED || rdr_view_state.force_double_sided) {
		glDisable(GL_CULL_FACE); CHECKGL;
	}
	if (flags1 & TRICK_NOZTEST) {
		glDisable(GL_DEPTH_TEST); CHECKGL;
	}
	if (flags1 & TRICK_NOZWRITE)
	{
		WCW_DepthMask(0);
		WCW_StencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	}
	if (flags2 & TRICK2_NIGHTGLOW)
	{
		if (rdr_view_state.time > tricks->info->nightglow_times[0]
			 || rdr_view_state.time < tricks->info->nightglow_times[1])
		{
			static GLfloat ambient[4] = {0.25,0.25,0.25,1},diffuse[4] = {0,0,0,1};
			WCW_Light(GL_LIGHT0, GL_AMBIENT, ambient);
			WCW_Light(GL_LIGHT0, GL_DIFFUSE, diffuse);
		}
	}
	if (flags2 & TRICK2_HASADDGLOW)
	{
		int	t;

		if (curr_blend_state.shader == BLENDMODE_ADDGLOW) {
				// Only effects subs that are AddGlow
			t = rdr_view_state.lamp_alpha_byte;
			if (flags2 & TRICK2_ALWAYSADDGLOW) {
				t = 255;
			}
			if (rdr_caps.chip & ARBFP) {
				Vec4 ftvec = {0};
				if (flags2 & (TRICK2_NORANDOMADDGLOW|TRICK2_ALWAYSADDGLOW)) {
					ftvec[0] = !!t;
				} else {
					ftvec[0] = MIN(t/255.f, 0.5);
				}
				ftvec[1] = (hashCalc(&draw->uid,4,0)&(128-1))/128.f;
				WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_GlowParamFP, ftvec); // Last parameter must be 0
			} else {
				if (rdr_caps.chip & NV1X)
				{
					if (t < 100) {
						glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB); CHECKGL;
					} else {
						glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_TEXTURE1, GL_UNSIGNED_IDENTITY_NV, GL_RGB); CHECKGL;
					}
				}
			}
		}
	}
	if (flags2 & TRICK2_SETCOLOR)
	{
#if RDRDONTFIX
		constColor0[0] = tricks->rgba[0] * (1.f/255.f);
		constColor0[1] = tricks->rgba[1] * (1.f/255.f);
		constColor0[2] = tricks->rgba[2] * (1.f/255.f);
	
		constColor1[0] = tricks->rgba2[0] * (1.f/255.f);
		constColor1[1] = tricks->rgba2[1] * (1.f/255.f);
		constColor1[2] = tricks->rgba2[2] * (1.f/255.f);

		WCW_ConstantCombinerParamerterfv(0, constColor0);
		WCW_ConstantCombinerParamerterfv(1, constColor1);
#endif

	}
	if (flags1 & TRICK_NOFOG) {
		WCW_Fog(0);
	} else if(flags1 & TRICK_NOZTEST) {
		// Use immediate mode fog when noztest is on
		fogWasDeferring = WCW_FogDeferring();
		WCW_SetFogDeferring(0);
	}
	if (flags1 & (TRICK_REFLECT | TRICK_REFLECT_TEX1))
	{
	int		i,ref_flags[] = {TRICK_REFLECT,TRICK_REFLECT_TEX1};

		for(i=0;i<2;i++)
		{
			if (flags1 & ref_flags[i])
			{
				WCW_ActiveTexture(i);
#if RDRMAYBEFIX
				if (model->flags & OBJ_CUBEMAP)
				{
					Mat4	m;
					Mat44	matx;

					WCW_EnableTexture(GL_TEXTURE_CUBE_MAP_ARB,TEXLAYER_BASE+i);
					glMatrixMode(GL_TEXTURE); CHECKGL;
					copyMat3(cam_info.inv_viewmat,m);
					zeroVec3(m[3]);
					mat43to44(m,matx);
					glLoadMatrixf((F32 *)matx); CHECKGL;
					glMatrixMode(GL_MODELVIEW); CHECKGL;
				}
#endif
				glEnable(GL_TEXTURE_GEN_S); CHECKGL;
				glEnable(GL_TEXTURE_GEN_T); CHECKGL;
				glEnable(GL_TEXTURE_GEN_R); CHECKGL;

				glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_ARB); CHECKGL;
				glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_ARB); CHECKGL;
				glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP_ARB); CHECKGL;

			}
		}
	}
	bDidStipple = false;
	if (flags2 & (TRICK2_STIPPLE1|TRICK2_STIPPLE2) && draw)
	{
		U8 stipple_alpha = draw->stipple_alpha==255?draw->alpha:draw->stipple_alpha;
		bool bInvert = flags2&TRICK2_STIPPLE2;
#ifdef USE_GL_STIPPLE // Works on NVidia only
		// Actual stipple - works on NVidia only
		U8 *pat;
		extern U8 *getStipplePattern(int alpha, bool bInvert);
		glEnable(GL_POLYGON_STIPPLE); CHECKGL;
		pat = getStipplePattern(stipple_alpha, bInvert);
		glPolygonStipple((U8*)pat); CHECKGL;
		bDidStipple = true;
#else
		// Stencil stipple hack
		WCW_EnableStencilTest();
		if (bInvert) {
			// 14- -> 15
			// 15 -> 14
			// 255 needs to pass stencil on values of 1-16
			WCW_StencilFunc(GL_LESS, (255 - (stipple_alpha+1)) / 16, ~0);
		} else {
			// 14- -> 0
			// 15 -> 1
			// 255 needs to pass stencil on values of 1-16
			// n here and 255-n above need to yield identical values for all n
			WCW_StencilFunc(GL_GEQUAL, (stipple_alpha+1) / 16, ~0);
		}
		WCW_StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
#endif
	}
	if (flags1 & TRICK_ALPHACUTOUT)
	{
		//Just trees want this
		//WCW_EnableStencilTest();   
		//WCW_StencilFunc(GL_ALWAYS, 64, ~0); 
		//WCW_StencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
		if (constColor0[3] == 1.0 || bDidStipple) {
			glAlphaFunc(GL_GREATER, 0.6); CHECKGL;
			WCW_BlendFuncPush(GL_ONE, GL_ZERO);
		} else {
			F32 d = constColor0[3] - 0.4f;
			if (d < 0)
				d = 0;
			glAlphaFunc(GL_GREATER, d); CHECKGL;
			WCW_BlendFuncPushNop();
		}
	}
	if (flags2 & TRICK2_ALPHAREF) {
		glAlphaFunc(GL_GREATER,tricks->info->alpha_ref * constColor0[3]);
		CHECKGL;
	}
	if (flags2 & TRICK2_TEXBIAS)
	{
		WCW_TexLODBias(0, tricks->info->tex_bias);
	}

	if (is_fallback_material && (flags2 & TRICK2_FALLBACKFORCEOPAQUE))
	{
		WCW_BlendFuncPush(GL_ONE, GL_ZERO);
	}

	// Sanity checking
	if(WCW_FogDeferring() && (((flags1 & TRICK_NOFOG)!=0)) != ((flags1 & TRICK_NOZWRITE)!=0))
	{
		failText("NoFog(%d) != NoZWrite(%d) for %s in %s\n",
			(flags1 & TRICK_NOFOG)!=0, (flags1 & TRICK_NOZWRITE)!=0,
			tricks->info->name, tricks->info->file_name);
	}
	
	return 1;
}

void gfxNodeTricksUndo(TrickNode *tricks, VBO *vbo, bool is_fallback_material)
{
	int		flags1;
	int		flags2;

	flags1 = (tricks?tricks->flags1:0) | (rdr_view_state.force_double_sided?TRICK_DOUBLESIDED:0);
	flags2 = tricks?tricks->flags2:0;
	if (!(flags1 & (TRICK_ADDITIVE|TRICK_SUBTRACTIVE|TRICK_DOUBLESIDED|TRICK_NOZTEST|TRICK_NOZWRITE|TRICK_NOFOG|TRICK_REFLECT|TRICK_REFLECT_TEX1|TRICK_ALPHACUTOUT)) &&
		!(flags2 & (TRICK2_HASADDGLOW|TRICK2_STANIMATE|TRICK2_STSSCALE|TRICK2_STSCROLL0|TRICK2_STSCROLL1|TRICK2_ALPHAREF|TRICK2_TEXBIAS|TRICK2_NIGHTGLOW|TRICK2_SETCOLOR|TRICK2_STIPPLE1|TRICK2_STIPPLE2|TRICK2_FALLBACKFORCEOPAQUE)))
		return;
 
	if( flags2 & ( TRICK2_STANIMATE | TRICK2_STSSCALE | TRICK2_STSCROLL0 | TRICK2_STSCROLL1 ) )
	{
		WCW_LoadTextureMatrixIdentity(0);
		WCW_LoadTextureMatrixIdentity(1);

		memset(tex_scrolls[0],0,sizeof(tex_scrolls[0]));
		memset(tex_scrolls[1],0,sizeof(tex_scrolls[1]));

		glMatrixMode(GL_MODELVIEW); CHECKGL;
	}

	if (flags1 & TRICK_ADDITIVE)
		WCW_BlendFuncPop();

	if (flags1 & TRICK_SUBTRACTIVE)
		WCW_BlendFuncPop();

	if (flags1 & (TRICK_ADDITIVE|TRICK_SUBTRACTIVE))
	{
		if (rdr_caps.supports_subtract) {
			glBlendEquationEXT(GL_FUNC_ADD_EXT); CHECKGL;
		}
		if ((rdr_caps.chip & NV1X) && !(rdr_caps.chip & ARBFP))
		{
			//curr_draw_state = -1;
			curr_blend_state.intval = -1;
			//glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FOG, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
			//since we didn't record which state glFinalCombinerInputNV was coming from when we
			//set the TRICK_ADDITIVE, for now whatever comes next needs to reset the blend state
			//to make sure it's got hte right one
		} else {
			WCW_FogPop();
		}
		if (alpha_write_mask) {
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); CHECKGL;
		}
	}
	if (flags1 & TRICK_DOUBLESIDED)
	{
		glEnable(GL_CULL_FACE); CHECKGL;
	}
	if (flags1 & TRICK_NOZTEST)
	{
		glEnable(GL_DEPTH_TEST); CHECKGL;
	}
	if (flags1 & TRICK_NOZWRITE)
	{
		WCW_DepthMask(1);
		WCW_StencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
	}
	if (flags1 & TRICK_NOFOG) {
		WCW_Fog(1);
	}
	if(fogWasDeferring) {
		WCW_SetFogDeferring(fogWasDeferring);
		fogWasDeferring = 0;
	}
	if ((flags2 & TRICK2_HASADDGLOW) && curr_blend_state.shader == BLENDMODE_ADDGLOW)
	{
		if ((rdr_caps.chip & NV1X) && !(rdr_caps.chip & ARBFP)) {
			glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB); CHECKGL;
		}
	}
	if (flags2 & TRICK2_SETCOLOR)
	{
		if (rdr_caps.chip == TEX_ENV_COMBINE) {
			WCW_Color4(255,255,255,255);
		}
	}
	if (flags1 & (TRICK_REFLECT | TRICK_REFLECT_TEX1))
	{
	int		i,ref_flags[] = {TRICK_REFLECT,TRICK_REFLECT_TEX1};

		for(i=0;i<2;i++)
		{
			if (flags1 & ref_flags[i])
			{
				WCW_LoadTextureMatrixIdentity(i);
				WCW_EnableTexture(GL_TEXTURE_2D,TEXLAYER_BASE+i);
				glDisable(GL_TEXTURE_GEN_S); CHECKGL;
				glDisable(GL_TEXTURE_GEN_T); CHECKGL;
				glDisable(GL_TEXTURE_GEN_R); CHECKGL;
			}
		}
		glMatrixMode(GL_MODELVIEW); CHECKGL;
	}
	if (flags2 & TRICK2_ALPHAREF) {
		glAlphaFunc(GL_GREATER,0); CHECKGL;
	}
	if (flags1 & TRICK_ALPHACUTOUT)
	{
		glAlphaFunc(GL_GREATER,0); CHECKGL;
		WCW_BlendFuncPop();
		//WCW_EnableStencilTest();  
		//WCW_StencilFunc(GL_ALWAYS, 128, ~0);
		//WCW_StencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
	}
	if (flags2 & TRICK2_TEXBIAS)
	{
		WCW_TexLODBias(0, -0.5);
	}
	if (flags2 & TRICK2_NIGHTGLOW)
	{
		WCW_Light(GL_LIGHT0, GL_AMBIENT, rdr_view_state.sun_ambient);
		WCW_Light(GL_LIGHT0, GL_DIFFUSE, rdr_view_state.sun_diffuse);
	}
	if (flags2 & (TRICK2_STIPPLE1|TRICK2_STIPPLE2))
	{
#ifdef USE_GL_STIPPLE
		glDisable(GL_POLYGON_STIPPLE); CHECKGL;
#else
		WCW_DisableStencilTest();
#endif
	}

	if (is_fallback_material && (flags2 & TRICK2_FALLBACKFORCEOPAQUE))
	{
		WCW_BlendFuncPop();
	}
}

