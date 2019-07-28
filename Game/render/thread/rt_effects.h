#ifndef _RT_EFFECTS_H_
#define _RT_EFFECTS_H_

#include "stdtypes.h"

typedef struct RdrShrinkTextureParams
{
	int tex_handle, alphatex_handle, tex_width, tex_height, width, height, shrink_horizontal, shrink_vertical;
} RdrShrinkTextureParams;

typedef struct RdrDivTextureByAlpha
{
	int tex1_handle, tex2_handle, tex_width, tex_height;
} RdrDivTextureByAlpha;

typedef struct RdrAddTexturesParams
{
	int tex1_handle, tex1_uoffset, tex1_voffset, tex1_width, tex1_height, tex1_texwidth, tex1_texheight;
	int tex2_handle, tex2_uoffset, tex2_voffset, tex2_width, tex2_height, tex2_texwidth, tex2_texheight;
	int tex3_handle, tex3_texwidth, tex3_texheight;
} RdrAddTexturesParams;

typedef struct RdrIrradiateParams
{
	int positions_handle, irradiance_handle, radiance_handle, depth_handle, normals_handle, width, height;
	Mat4 mw;
	Mat44 mvp;
	Vec4 projectvec;
	Vec3 global_ray_dir;
	int *done;
	int nolightangle;
} RdrIrradiateParams;

typedef struct PBuffer PBuffer;

void rdrPostprocessingDirect(PBuffer *pbFrameBuffer);
void rdrHDRThumbnailDebugDirect(void);
void rdrOutlineDirect(PBuffer *pbInput, PBuffer *pbOutput);

typedef struct VBO VBO;

typedef struct RdrSunFlareParams
{
	VBO * vbo;
	Mat4 mat;
	float * visibility;
} RdrSunFlareParams;

void rdrSunFlareUpdateDirect(RdrSunFlareParams * params);

void rdrRenderScaledDirect(PBuffer *pbFrameBuffer);

void rdrDebugForceStall(void);

bool getSpecialShaderName(int shader, 
					char* nameBuff /*can be NULL if you don't want it*/, 
					size_t nameBuffLen, 
					U32* pnFlags, /* set to tShaderCompileFlags vals; can be NULL if you don't want it */
                    const char** extraDefineSet /* set to extra define string (space separated list); can be NULL if you don't want it */
				);	// returns false if <shader> is out of range

void rdrAddTexturesDirect(RdrAddTexturesParams *params);
void rdrMulTexturesDirect(RdrAddTexturesParams *params);

extern int shaderEffectsPrograms[];


#endif//_RT_EFFECTS_H_
