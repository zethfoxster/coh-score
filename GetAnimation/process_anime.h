/******************************************************************************
 A .ANIME is the text file format exporter by the animation exporter in 3DS MAX
 The purpose of this code is to convert that source data into the game runtime
 animation format. The name is derived from: ANIMation Export.
*****************************************************************************/
#ifndef _PROCESS_ANIME_H
#define _PROCESS_ANIME_H

#include "processanim.h"

bool animConvert_ANIME_To_AnimTrack( SkeletonAnimTrack * pAnimTrack, char * sourcepath, Node * skeletonRoot );

#endif // _PROCESS_ANIME_H
