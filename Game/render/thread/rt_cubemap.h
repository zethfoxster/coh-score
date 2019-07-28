//
//	rt_cubemap.h
//
//
#ifndef __rt_cubemap_h__
#define __rt_cubemap_h__

#include "viewport.h"

void rt_initCubeMapMenu();

void rt_cubemapViewportPreCallback(ViewportInfo** ppViewport);
void rt_cubemapViewportPostCallback(ViewportInfo** ppViewport);

void rt_cubemapSetInvCamMatrixDirect(void);
void rt_cubemapSet3FaceMatrixDirect(void);
void rt_reflectionViewportPostCallback(ViewportInfo* viewport);

#endif //  __rt_cubemap_h__
