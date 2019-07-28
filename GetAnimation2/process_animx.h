/******************************************************************************
 A .ANIMX is the text file format exporter by the animation exporter in 3DS MAX
 The purpose of this code is to convert that source data into the game runtime
 animation format. The name is derived from: ANIMation eXport.
*****************************************************************************/
#ifndef _PROCESS_ANIMX_H
#define _PROCESS_ANIMX_H

#include "processanim.h"

bool animConvert_ANIMX_To_AnimTrack( SkeletonAnimTrack * pAnimTrack, char * sourcepath, Node * skeletonRoot );

#endif // _PROCESS_ANIMX_H
