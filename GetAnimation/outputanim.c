#include "stdtypes.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "gridpoly.h"
#include "mathutil.h"
#include "utils.h"
#include "error.h"
#include "outputanim.h" 
#include "assert.h"
#include "processanim.h"
#include "perforce.h"
#include "error.h"
#include "file.h"
#include <sys/stat.h>

extern int total_uncompressed_position_keys; 
extern int	total_added_rot_nodes;
extern int	total_added_rot_keys;
extern int	total_added_pos_nodes;
extern int	total_added_pos_keys;
extern int total_dummy_rotation_keys;
extern int total_dummy_position_keys;
extern int maxrun[2];

extern int end_run_due_to_too_much_delta_error[2];
extern int end_run_due_to_switched_missing_key[2];
extern int end_run_due_to_end_of_track[2];
extern int total_runs[2];
extern int total_useless_runs[2];
extern int runhist[2][10000];


extern int non_matching_counts;

static int total_tracks[2];
static int total_fullkey_data[2];
static int total_correction_data[2];

extern int total_saved_keys[2];
extern int total_keys[2];

extern g_no_checkout;

//################ Memory Blocks ###################################
typedef struct
{
	char	*name;
	int		total;
	U8		*data;
	int		used;
} MemBlock;


MemBlock mem_blocks[] =
{
	{ "ANIMDATA",			32000000 },
	{ "ANIMTRACKS",			16000000 },
};

static void *getMemBase(int block_num)
{
MemBlock	*mblock;

	mblock = &mem_blocks[block_num];
	if (!mblock->data)
		mblock->data = calloc(mblock->total,1);
	return &mblock->data[mblock->used];
}

static void *mallocBlockMem(int block_num,int amount)
{
void	*ptr;
MemBlock	*mblock;

	getMemBase(block_num);
	mblock = &mem_blocks[block_num];
	if (amount > (mblock->total - mblock->used))
	{
		printf("Out of memory in anim block %s! (%d used, %d needed)\n",
			mblock->name,mblock->used,amount);
		FatalErrorf("Need to recompile getvrml to fix this!\n");
	}
	ptr = &mblock->data[mblock->used];
	mblock->used += amount;
	return ptr;
}

void *allocBlockMem(int block_num,int amount)
{
void	*ptr;

	ptr = mallocBlockMem(block_num,amount);
	memset(ptr,0,amount);
	return ptr;
}
//############### End Manage Memeory Blocks ############################################


//Slightly half assed generalization, really all of outputPackSkeletonAnimTrack should be generalized so both
//vrml reader and old anim tracks cna use them. But this will do, and its better than before
void transferBoneData( BoneAnimTrack * bt, void * rot_fulls, void * pos_fulls )
{
	int track_size;

	//############ Add rotation data
	//Add Full Keys
	//// TO DO don't calculate all this here, do it in the processing step output shouldn't know about it
	if( bt->flags & ROTATION_UNCOMPRESSED )
		track_size = bt->rot_fullkeycount * SIZE_OF_ROTKEY_UNCOMPRESSED;
	else if( bt->flags & ROTATION_COMPRESSED_TO_8_BYTES )
		track_size = bt->rot_fullkeycount * SIZE_OF_ROTKEY_COMPRESSED_TO_8_BYTES;
	else if( bt->flags & ROTATION_COMPRESSED_TO_5_BYTES 
		|| bt->flags & ROTATION_COMPRESSED_NONLINEAR )
		track_size = bt->rot_fullkeycount * SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES;

	bt->rot_idx = allocBlockMem(MEM_ANIMDATA, track_size );
	memcpy( bt->rot_idx, rot_fulls, track_size );	//each key value

	total_fullkey_data[ROTATION_KEY] += track_size; //debug

	//############ Add position data
	//Add full keys
	//// TO DO don't calculate all this here, do it in the processing step output shouldn't know about it
	if( bt->flags & POSITION_UNCOMPRESSED )
		track_size = bt->pos_fullkeycount * SIZE_OF_UNCOMPRESSED_POSKEY;
	else if( bt->flags & POSITION_COMPRESSED_TO_6_BYTES )
		track_size = bt->pos_fullkeycount * SIZE_OF_POSKEY_COMPRESSED_TO_6_BYTES;
	else
		assert(0);
	bt->pos_idx = allocBlockMem(MEM_ANIMDATA, track_size );
	memcpy( bt->pos_idx, pos_fulls, track_size );	//each key value

	total_fullkey_data[POSITION_KEY] += track_size; //debug
}




/*Packs an node tree into an SkeletonAnimTrack and its animtracks
note that the name of the header is the full path of the .wrl file minus ".wrl"
*/
void outputPackSkeletonAnimTrack( SkeletonAnimTrack * skeleton, Node *root ) 
{
	int	 i;
	Node	**nodelist,*node;
	BoneAnimTrack * bt;
	F32	longest = 0;

	nodelist = treeArray( root, &skeleton->bone_track_count );

	skeleton->bone_tracks = allocBlockMem( MEM_BONEANIMTRACKS, skeleton->bone_track_count * sizeof( BoneAnimTrack ) );
	memset( skeleton->bone_tracks, 0, sizeof(BoneAnimTrack) * skeleton->bone_track_count );

	for(i=0 ; i < skeleton->bone_track_count ; i++ )
	{
		node = nodelist[i];
		bt = &(skeleton->bone_tracks[i]);
		
		bt->id = node->anim_id; 
		bt->flags |= node->poskeys.flags;
		bt->flags |= node->rotkeys.flags;
		bt->rot_count = node->rotkeys.count;
		bt->pos_count = node->poskeys.count;
		bt->rot_fullkeycount = node->rotkeys.fullkeycount;
		bt->pos_fullkeycount = node->poskeys.fullkeycount;

		checkKeysBeforeExport( &node->poskeys, &node->rotkeys ); //debug check
	
		transferBoneData( bt, node->rotkeys.vals, node->poskeys.vals );

		//########### Screwy shit I don't get fully at least move it out of the output file
		//Find lengths and times and write them to the skeleton
		/*this is to figure out how long the animation track is by picking the longest of the 
		rotation track and xlation track.  Don't get %100
		*/
		{ 
			if (bt->rot_count && ( bt->rot_count-1 > longest) )
				longest = bt->rot_count-1;

			if (bt->pos_count && ( bt->pos_count-1 > longest) )
				longest = bt->pos_count-1;
		}
	}

	skeleton->length = longest;
} 


/*reset the globals (called after outputData and outputGroupInfo are done) So this 
*/
void outputResetVars()
{
	int		i;

	for(i=0;i<ARRAY_SIZE(mem_blocks);i++)
		mem_blocks[i].used = 0;
}

/*Writes the data to .trk and .skel, subtracting pointers along the way.
Am I doing pointer arithmetic right, or does it start on word boundries or something crazy, see ftell
*/
void outputAnimTrackToAnimFile( SkeletonAnimTrack * skeleton, char * targetFilePath ) 
{
	int	i, heirarchySize; 
	BoneAnimTrack * bts;
	BoneAnimTrack * bt;
	SkeletonHeirarchy * skeletonHeirarchy;
	FILE *file;
	char *oldData, *newData;
	int oldSize, newSize;

	// #### open .anim file ##########
	if( !g_no_checkout )
	{
		int error;
		printf( "Trying To Checkout %s...", targetFilePath );
		perforceSyncForce(targetFilePath, PERFORCE_PATH_FILE);
		error = perforceEdit(targetFilePath, PERFORCE_PATH_FILE);
		if( error )
			printf( "But I failed.\n" );
		else
			printf( " Success.\n");
	} else {
		printf( " WOULD CHECK OUT: %s\n", targetFilePath);
		chmod(targetFilePath, _S_IREAD | _S_IWRITE );
	}

	mkdirtree(targetFilePath);
	oldData = fileAlloc(targetFilePath, &oldSize);
	file = fopen( targetFilePath, "wb" );
	if (!file)
		FatalErrorf("Can't open %s for writing! (Maybe data dir does not exist?)\n", targetFilePath );

	if( skeleton->skeletonHeirarchy )
	{
		heirarchySize = sizeof( SkeletonHeirarchy );
		//preserve pointer so I can write data below
		skeletonHeirarchy = skeleton->skeletonHeirarchy; 
		//then fix the pointer for the file write
		skeleton->skeletonHeirarchy = (void *)((U32)sizeof( SkeletonAnimTrack )); //skeletonHeirarchy will be after SkeletonAnimTrack
	}
	else
	{
		heirarchySize = 0;
	}

	// #### write skeleton ######
	skeleton->headerSize = sizeof(SkeletonAnimTrack) + heirarchySize + mem_blocks[MEM_BONEANIMTRACKS].used;

	skeleton->bone_tracks = (void *)( (U32)sizeof(SkeletonAnimTrack) + (U32)heirarchySize ); // how far into file bones block starts
			
	fwrite( skeleton, sizeof(SkeletonAnimTrack), 1, file );

	if( skeleton->skeletonHeirarchy )
		fwrite( skeletonHeirarchy, sizeof( SkeletonHeirarchy ), 1, file );

	// #### write bonetracks ######
	//  ( fix up all the pointers, then write the data ) 
	bts	= (void *)mem_blocks[MEM_BONEANIMTRACKS].data;

	for( i = 0 ; i < skeleton->bone_track_count ; i++ )
	{
		bt = &(bts[i]);
		assert( bt->rot_idx && bt->pos_idx );

		//pointer to this track, minus pointer to start of all track, plus size of header
		bt->rot_idx	= (void *)((U32)skeleton->headerSize + ((U32)bt->rot_idx - (U32)mem_blocks[MEM_ANIMDATA].data ) ); 
		bt->pos_idx	= (void *)((U32)skeleton->headerSize + ((U32)bt->pos_idx - (U32)mem_blocks[MEM_ANIMDATA].data ) );
	}

	fwrite( bts, mem_blocks[MEM_BONEANIMTRACKS].used, 1, file);

	//#### write data blocks ###
	fwrite( mem_blocks[MEM_ANIMDATA].data, mem_blocks[MEM_ANIMDATA].used, 1, file);

	fclose(file);

	// Check for no changes (and undo checkout)
	newData = fileAlloc(targetFilePath, &newSize);
	if (!newData || !oldData || newSize != oldSize)
		return;
	if (memcmp(newData, oldData, newSize)==0) {
		printf(" File did not change in processing%s\n", g_no_checkout ? "" : ", undoing checkout...");
		if (!g_no_checkout) {
			perforceRevert(targetFilePath, PERFORCE_PATH_FILE);
		}
	}
}
///################### End Write data to a file ##########################################

