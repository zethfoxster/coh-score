#ifndef _ANIMTRACK_H
#define _ANIMTRACK_H

#include "bones.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

//######################################################################
//#################  Animations #######################################
#define BONES_ON_DISK		100
									STATIC_ASSERT(BONEID_COUNT <= BONES_ON_DISK);
#define MAX_ANIMSETS		1000	//so big so they can be reloaded in development mode
#define MAX_ANIMDATATRACKS	100

#define SIZE_OF_ROTKEY_UNCOMPRESSED			 ( 16 )
#define SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES ( 5 )	//quantized quaternion
#define SIZE_OF_ROTKEY_COMPRESSED_TO_8_BYTES ( 8 )	//stuff quat into shorts
#define SIZE_OF_UNCOMPRESSED_POSKEY ( 4 * 3 )		//full size Vec3 (3 floats)
#define SIZE_OF_POSKEY_COMPRESSED_TO_6_BYTES (2 * 3) //3 floats quantized to 2 byte chunks
#define SIZE_OF_TIME   (4 * 1) //1 float
#define SIZE_OF_KEYCORRECTION (2)

#define CFACTOR_8BYTE_QUAT 10000
#define EFACTOR_8BYTE_QUAT 0.0001

#define ROTATION_KEY 0
#define POSITION_KEY 1

#define CFACTOR_6BYTE_POS 32000
#define EFACTOR_6BYTE_POS 0.00003125 //((F32)((F32)1/(F32)CFACTOR_6BYTE_POS))

#define ROTATION_UNCOMPRESSED			(1 << 0)
#define ROTATION_COMPRESSED_TO_5_BYTES	(1 << 1)
#define ROTATION_COMPRESSED_TO_8_BYTES	(1 << 2)
#define POSITION_UNCOMPRESSED			(1 << 3)
#define POSITION_COMPRESSED_TO_6_BYTES	(1 << 4)
#define ROTATION_DELTACODED				(1 << 5)
#define POSITION_DELTACODED				(1 << 6)
#define ROTATION_COMPRESSED_NONLINEAR	(1 << 7)

#define POS_BIGGEST 2000000 //debug thingy

#define LOOKUPSIZE12BIT 4096
#define COMPRESSEDQUATSIZE 5

#define MAX_ANIM_FILE_NAME_LEN 256


enum
{
	LOAD_SKELETONS_ONLY		= 1 << 0, // why are these bitfields?
	LOAD_ALL				= 1 << 1,
	LOAD_ALL_SHARED			= 1 << 2,
} ;

typedef struct BoneLink
{
	int	child;
	int next;
	BoneId id; //kinda redundant
} BoneLink;

typedef struct SkeletonHeirarchy
{
	int			heirarchy_root; //entrance to skeleton_heirarchy array
	BoneLink	skeleton_heirarchy[BONES_ON_DISK]; //tree structure
} SkeletonHeirarchy;

typedef struct
{
	const void	*rot_idx;	//Full keys
	const void	*pos_idx;	//Full keys
	U16		rot_fullkeycount;
	U16		pos_fullkeycount; //only needed for debug		

	U16		rot_count;
	U16		pos_count;	//why can't I figure out how to get rid of this?
	
	char    id;			//I am this bone's animation track
	char	flags;		//

	unsigned short pack_pad;	// make padding explicit
} BoneAnimTrack;

typedef struct SkeletonAnimTrack SkeletonAnimTrack;

struct SkeletonAnimTrack
{
	int			headerSize;  //size of everything but the data (got to be first)
	char		name[MAX_ANIM_FILE_NAME_LEN]; //File name
	char		baseAnimName[MAX_ANIM_FILE_NAME_LEN]; //.anim file to get missing bones, and heirarchy target wil heve a skeleton_heirarchy and have skel_ in the name

	F32			max_hip_displacement;//for visibility code
	F32			length;				//of animation track, if any, in case the sequencer doesn't specify, kinda hack.
	BoneAnimTrack * bone_tracks;
	int			bone_track_count;
	int			rotation_compression_type;
	int			position_compression_type;
	const SkeletonHeirarchy * skeletonHeirarchy; //only the _skel file in each folder needs one of these in data file, every one else just points to this file

	//for In-Engine use 
	const SkeletonAnimTrack * backupAnimTrack; //contains defaults and the heirarchy
	int			loadstate;
	F32			lasttimeused;  
	int			fileAge;		//development mode
	int			spare_room[9];
};

//#################### End Animations ######################################

typedef struct AnimationEngine
{
	int tracksLoadedCount;
	StashTable tracksLoadedHT;
} AnimationEngine;

SkeletonAnimTrack * animGetAnimTrack( const char * name, int loadType, const char *requestor );
void animExpand8ByteQuat( S16 * compressed, Quat expanded );
void initLookUpTable( F32 * table );
void unPack5ByteQuat( U8 * cquat, U32 * qidxs, int * missing );

// used by tools/debugging
void animPatchLoadedRawData( SkeletonAnimTrack * skeleton );
void animValidateTrackData( SkeletonAnimTrack * skeleton );

SkeletonAnimTrack * animLoadAnimFile( const char* fileName );

#endif
