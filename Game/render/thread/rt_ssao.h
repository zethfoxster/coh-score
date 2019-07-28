#ifndef RT_SSAO_H
#define RT_SSAO_H

typedef struct RdrSsaoParams
{
	S32 width;
	S32 height;
	F32 sceneAmbientOcclusionWeight;
	U8 strength;
	U8 ambientDownsample;
	U8 blurDownsample;
	U8 depthDownsample;
	U8 blur;
} RdrSsaoParams;

void rdrAmbientSetParamsDirect(RdrSsaoParams * params);
void rdrAmbientRenderDirect();
void rdrAmbientRenderDebugDirect();
void rdrHandleEffectReloadDirect( const char* szEffectName );

#endif
