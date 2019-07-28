#ifndef _RENDERCLOTH_H
#define _RENDERCLOTH_H

#include "model.h"
#include "gfxtree.h"
#include "Cloth.h"
#include "ClothMesh.h"

#include "rt_cloth.h"

void modelDrawClothObject( GfxNode * node, BlendModeType blend_mode );
void modelDrawClothObjectDirect( RdrCloth *rc );
#endif
