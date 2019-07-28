#ifndef _ANIMTRACKANIMATE_H
#define _ANIMTRACKANIMATE_H

#include "stdtypes.h"
#include "mathutil.h"
#include "animtrack.h"
#include "assert.h"


// GetAnimation only struct.  Converted into a BoneAnimTrack
typedef struct AnimKeys
{
	U16		* corrections;  
	int		count;	

	void	*vals;			//rename fullkeys?
	int		fullkeycount;

	F32		*times;			//thrown away 
	U8		flags;	//compression type

} AnimKeys;


//For animating textures with tricks or in fx
typedef struct StAnim
{
	const BoneAnimTrack * btTex0;  
	const BoneAnimTrack * btTex1;
	char	*name;				//skeleton name of animation in tscroll.anm, corresponds to a WRL file
	F32		speed_scale;
	F32		st_scale;
	int		flags;
} StAnim;


void animGetRotationValue( int intTime, F32 remainderTime, const BoneAnimTrack * bt, Quat newrot, int interpolate);
void animGetPositionValue( int intTime, F32 remainderTime, const BoneAnimTrack * bt, Vec3 newpos,   int interpolate);
void processAnimPosKeys(AnimKeys * keys, F32 * static_position);
void processAnimRotKeys(AnimKeys * keys, Quat static_rotation);
void checkKeysBeforeExport( AnimKeys * poskeys, AnimKeys * rotkeys );
void animExpand5ByteQuat( U8 * compressed, Quat eq );
void animExpandNonLinearQuat( U8 * compressed, Quat eq );
void animExpand6BytePosition( S16 * compressed, Vec3 expanded );
const BoneAnimTrack * animFindTrackInSkeleton( const SkeletonAnimTrack * skeleton, int id );
int setStAnim( StAnim *stAnim );

#define QERROR 0.001

S16 quantizePositionValue( F32 value );
int makeBiggestValuePositive( F32 * quat );
void compressQuatToFiveBytes( F32 * quat, char * cquat );

#endif