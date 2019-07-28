
#ifndef _MEGAGRID_H
#define _MEGAGRID_H

#include "stdtypes.h"

typedef struct MegaGridNode MegaGridNode;

MegaGridNode* createMegaGridNode(void* owner, F32 radiusF32, int use_small_cells);
void destroyMegaGridNode(MegaGridNode* node);
void mgUpdateNodeRadius(MegaGridNode* node, F32 radiusF32, int use_small_cells);

void mgVerifyAllGrids(MegaGridNode* excludeNode);

void mgUpdate(int grid, MegaGridNode* node, const Vec3 posF32);
void mgRemove(MegaGridNode* node);
int mgGetNodesInRange(int grid, const Vec3 posF32, void** nodeArray, int maxCount);
int mgGetNodesInRangeWithSize(int grid, Vec3 posF32, void** nodeArray, int size);

#endif