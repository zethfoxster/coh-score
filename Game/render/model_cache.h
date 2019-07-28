#ifndef _MODEL_CACHE_H
#define _MODEL_CACHE_H

#include "anim.h"
#include "rt_state.h"

typedef struct VBO VertexBufferObject;

void modelCacheInitUnpackCS(void);

void modelFreeVertexObject(VertexBufferObject *vbo);
void modelSetupVertexObject(Model *model, int useVbos, BlendModeType blend_mode );
void *modelConvertRgbBuffer(U8 *rgbs,int vert_count);
void modelFreeRgbBuffer(U8 *rgbs);

void modelUnpackAll(Model *model);
void modelUnpackReductions(Model *model);
void modelSetupZOTris(Model *model);
void modelSetupPRQuads(Model *model);
void modelSetupRadiosity(Model *model);
void modelFreeAllUnpacked(Model *model);

#endif
