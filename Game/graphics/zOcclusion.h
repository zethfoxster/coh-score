#ifndef _ZOCCLUSION_H_
#define _ZOCCLUSION_H_

#include "stdtypes.h"

typedef struct Model Model;
typedef struct BasicTexture BasicTexture;

int zoIsSupported(void);

void zoInitFrame(Mat44 matProjection, Mat4 matWorldToEye, Mat4 matEyeToWorld, F32 nearPlaneDist);
void zoSetOccluded(int uid, int num_children);
int zoTest(Vec3 minBounds, Vec3 maxBounds, Mat4 matLocalToEye, int isZClipped, int uid, int num_children);
int zoTest2(Vec3 eyespaceBounds[8], int isZClipped, int uid, int num_children);
int zoTestSphere(Vec3 centerEyeCoords, float radius, int isZClipped);
int zoCheckAddOccluder(Model *m, Mat4 matModelToEye, int uid); // make sure zoTest is called before zoCheckAddOccluder

void zoNotifyModelFreed(Model *m);
void zoClearOccluders(void);

// debug
BasicTexture *zoGetZBuffer(void);
void zoShowDebug(void);

#endif

