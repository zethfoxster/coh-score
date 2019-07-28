#ifndef FOG_H
#define FOG_H

typedef struct FogVals FogVals;
void fogGather(void);
F32 fogUpdate(F32 scene_fog_near, F32 scene_fog_far, U8 scene_color[3], bool forceFogColor, bool doNotOverrideFogNodes, FogVals *fog_vals);

#endif