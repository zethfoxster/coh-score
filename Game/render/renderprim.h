#ifndef _RENDERPRIM_H
#define _RENDERPRIM_H

#include "stdtypes.h"
#include "rt_queue.h"

int drawEnablePrimQueue(int on);

void drawFilledBox4(int x1, int y1, int x2, int y2, int argb_ul, int argb_ur, int argb_ll, int argb_lr);
void drawFilledBox(int x1, int y1, int x2, int y2, int argb);
void drawLine2(int x1, int y1, int x2, int y2, int argb1, int argb2);
void drawLine(int x1, int y1, int x2, int y2, int argb);
void drawLine3dColorWidth(const Vec3 p1, int argb1, const Vec3 p2, int argb2, F32 width, bool queueit);
void drawLine3D_2(const Vec3 p1, int argb1, const Vec3 p2, int argb2);
void drawLine3D(const Vec3 p1, const Vec3 p2, int argb);
void drawLine3DWidth(const Vec3 p1, const Vec3 p2, int argb, F32 width);
void drawLine3D_2Queued(const Vec3 p1, int argb1, const Vec3 p2, int argb2);
void drawLine3DQueued(const Vec3 p1, const Vec3 p2, int argb);
void drawLine3DWidthQueued(const Vec3 p1, const Vec3 p2, int argb, F32 width);
void drawTriangle3D_3(const Vec3 p1, int argb1, const Vec3 p2, int argb2, const Vec3 p3, int argb3);
void drawTriangle3D(const Vec3 p1, const Vec3 p2, const Vec3 p3, int argb);
void drawQuad3D(const Vec3 a, const Vec3 b, const Vec3 c, const Vec3 d, U32 color, F32 line_width);
void drawBox3D(const Vec3 min, const Vec3 max, const Mat4 mat, U32 color, F32 line_width);

void rdrSetClipPlanes(int numPlanes, float planes[6][4]);
void rdrSetStencil(int enable,RdrStencilOp op,U32 func_ref);
void rdrSetFog(F32 near_dist, F32 far_dist, const Vec3 color);
void rdrGetFrameBufferAsync(F32 x, F32 y, int width, int height, GetFrameBufType type, void *buf);
void rdrGetFrameBuffer(F32 x, F32 y, int width, int height, GetFrameBufType type, void *buf);

void rdrSetBgColor(const Vec3 color);
void rdrClearScreen(void);
void rdrClearScreenEx(ClearScreenFlags clear_flags);
void rdrFreeBuffer(int id,char *mem_type);
void rdrFreeMem(void *data);
void rdrSetupStippleStencil(void);
void rdrStippleStencilNeedsRebuild(void);

void rdrGetTexSize(int texid, int *width, int *height);
void *rdrGetTex(int texid, int *width, int *height, int get_color, int get_alpha, int floating_point);

void queuedLinesDraw(void);
void queuedLinesReset(void);

void rdrSetAdditiveAlphaWriteMask(int on);

#endif
