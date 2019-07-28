/******************************************************************************
Utilities used by animation processing
*****************************************************************************/
#ifndef _PROCESSANIM_H
#define _PROCESSANIM_H

#include "tree.h"
#include "animtrack.h"

// Coordinate frame conversions
void ConvertCoordsFrom3DSMAX( Vec3 vOut, const Vec3 vIn );
void ConvertCoordsGameTo3DSMAX( Vec3 vOut, const Vec3 vIn );
void ConvertCoordsVRMLTo3DSMAX( Vec3 vOut, const Vec3 vIn );

// Node Tree manipulation
void assignBoneNums(Node * root);
Node * ditchNonAnimStuff(Node * root );
void getSkeleton( Node * root, SkeletonHeirarchy * skeletonHeirarchy );
void setNodeMats( Node * root);

#endif
