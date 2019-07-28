#ifndef RENDERSHADOWMAP_H
#define RENDERSHADOWMAP_H

void rdrShadowMapSetDefaults(void);
void rdrShadowMapUpdate(void);
void rdrShadowMapDebug2D(void);
void rdrShadowMapDebug3D(void);
void rdrShadowMapBeginUsing(void);
void rdrShadowMapFinishUsing(void);

void calcFrustumPoints( Vec3* points, Mat4 view_to_world, float z_near, float z_far, float fov_x, float fov_y );
bool isShadowExtrusionVisible(Vec3 mid_vs, F32 radius);


unsigned int rdrShadowMapCanWorldRenderWithShadows(void);
unsigned int rdrShadowMapCanEntityRenderWithShadows(void);
#endif

