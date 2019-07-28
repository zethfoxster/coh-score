#ifndef RT_FILTER_H
#define RT_FILTER_H

#include "rt_pbuffer.h"

typedef enum OptionFilterBlur {
	FILTER_NO_BLUR,
	FILTER_FAST_BLUR,
	FILTER_FAST_ALPHA_BLUR,
	FILTER_GAUSSIAN,
	FILTER_BILATERAL,
	FILTER_GAUSSIAN_DEPTH,
	FILTER_BILATERAL_DEPTH,
	FILTER_TRILATERAL,
	FILTER_BLUR_MAX
} OptionFilterBlur;

// filtering function used in production build to filter planar reflections
void rdrFilterHandleEffectReloadDirect( const char* szEffectName );
void rdrFilterRenderTargetDirect( PBuffer* pRT, int blurMode );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// PROTOTYPE CODE: originally used to evaluate filtering the reflection viewports
// SHOULD NOT BE USED FOR PRODUCTION FEATURE
// Currently still used to filter dynamic cubemap faces if enabled via the tweak menu
// (but not used in production)
// @todo rework to use the more efficient ping-pong filtering or just remove the feature.
typedef struct RdrFilterParams
{
	S32 width;
	S32 height;
	unsigned int srcTex;
	unsigned int srcTarget;	// e.g., GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, etc
	U8	mode;

} RdrFilterParams;

void rdrFilterSetParamsDirect(RdrFilterParams * params);
void rdrFilterRenderDirect( RdrFilterParams* params );

#endif
