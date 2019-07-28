#ifndef _OCTREE_H_
#define _OCTREE_H_

#include "stdtypes.h"

typedef struct Octree Octree;
typedef struct OctreeIdxList *OctreeEntry;
typedef struct Frustum Frustum;

typedef enum OctreeGranularity
{
	OCT_FINE_GRANULARITY,
	OCT_MEDIUM_GRANULARITY,
	OCT_ROUGH_GRANULARITY,
	OCT_VERY_ROUGH_GRANULARITY,
} OctreeGranularity;

Octree *octreeCreateEx(int mempool_size, char *filename, int linenumber);
#define octreeCreate(mempool_size) octreeCreateEx(mempool_size, __FILE__, __LINE__)
void octreeDestroy(Octree *octree);

int octreeAddBox(Octree *octree, void *node, const Vec3 min, const Vec3 max, OctreeEntry *entry, OctreeGranularity granularity);
void octreeAddSphere(Octree *octree, void *node, const Vec3 point, F32 radius, OctreeEntry *entry, OctreeGranularity granularity);

void octreeRemove(Octree *octree, OctreeEntry *entry);
void octreeRemoveAll(Octree *octree);

typedef void (*OctreeFindCallback)(void *node, void *user_data);
typedef int (*OctreeSphereVisCallback)(void *node, const Vec3 scenter, F32 sradius);
typedef int (*OctreeFrustumVisCallback)(void *node, const Frustum *frustum);

void octreeFindInSphereCB(Octree *octree, OctreeFindCallback callback, void *user_data, const Vec3 point, F32 radius);
void octreeFindInSphereEA(Octree *octree, void ***node_array, const Vec3 point, F32 radius, OctreeSphereVisCallback vis_callback);
void **octreeFindInSphere(Octree *octree, const Vec3 point, F32 radius, OctreeSphereVisCallback vis_callback);

void octreeFindInFrustumCB(Octree *octree, OctreeFindCallback callback, void *user_data, const Vec3 offset, const Frustum *frustum);
void octreeFindInFrustumEA(Octree *octree, void ***node_array, const Vec3 offset, const Frustum *frustum, OctreeFrustumVisCallback vis_callback);
void **octreeFindInFrustum(Octree *octree, const Vec3 offset, const Frustum *frustum, OctreeFrustumVisCallback vis_callback);


#endif //_OCTREE_H_
