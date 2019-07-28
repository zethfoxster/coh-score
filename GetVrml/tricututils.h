#ifndef _TRICUTUTILS_H_
#define _TRICUTUTILS_H_

#include "stdtypes.h"
#include "GenericMesh.h"

GMeshReductions *makeGMeshReductions(GMesh *mesh, F32 distances[3], Vec3 min, Vec3 max, int scale_by_area, int use_optimal_placement, int maintain_borders);

#endif//_TRICUTUTILS_H_

