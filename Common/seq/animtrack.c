#include "wininclude.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "mathutil.h"
#include "file.h"
#include "fileutil.h"
#include "error.h"
#include "utils.h"
#include "animtrack.h"
#include "StashTable.h"
//#include "animtrackanimate.h"
#include "strings_opt.h" //	For faster stricmp calls
#include "assert.h"
#include "cmdcommon.h"
#include "SharedHeap.h"
#include "timing.h"

AnimationEngine animEngine = {0};

static void animDebugCheckBoneTrackOnLoad( BoneAnimTrack * bt, const char *fileName )
{
	int i;

	/////////////// Debug ///////////////////////////////////////////////////
	if(
		bt->flags & ROTATION_COMPRESSED_TO_5_BYTES
		|| bt->flags & ROTATION_COMPRESSED_NONLINEAR
		) //TEMP DEBUG
	{
		for(i = 0 ; i < bt->rot_fullkeycount ; i++ )
		{
			if ( ((U8*)bt->rot_idx)[ i * 5 ] >= 64 )
				FatalErrorFilenamef(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
		}
	}
	else if(bt->flags & ROTATION_UNCOMPRESSED ) //TEMP DEBUG
	{
		F32 * quat;
		for(i = 0 ; i < bt->rot_fullkeycount ; i++ )
		{
			quat = (F32*)&((F32*)bt->rot_idx)[ i * 4 ];
			if (!finiteVec4( quat ))
				FatalErrorf(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
			if ( !quat[ 0 ] && !quat[ 1 ] && !quat[ 2 ] && !quat[ 3 ] )
				FatalErrorf(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
		}
	}
	if( bt->flags & POSITION_UNCOMPRESSED ) //TEMP DEBUG 
	{
		F32 * pos;
		for( i = 0 ; i < bt->pos_fullkeycount ; i++ )
		{
  			pos = (F32*)&(((F32*)bt->pos_idx)[i*3]);
			if (!finiteVec3( pos ))
				FatalErrorf(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
			if (!( pos[0] < POS_BIGGEST && pos[1] < POS_BIGGEST && pos[2] < POS_BIGGEST ))
				FatalErrorf(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
		}
	}
}

static SkeletonAnimTrack * animReadTrackFile( FILE * file, int load_type, const char* fileName, SharedHeapAcquireResult* ret ) 
{
	SkeletonAnimTrack * skeleton;
	int file_size, i;
	BoneAnimTrack * bt;
	void * track_data = 0;
	SharedHeapHandle* pHandle = NULL;

	if ( load_type == LOAD_ALL_SHARED )
	{
		*ret = sharedHeapAcquire(&pHandle, fileName);
		if ( *ret == SHAR_DataAcquired )
		{
			// the data is already there, just return the pointer from the handle
			assert( pHandle );
			return pHandle->data;
		}
		else if ( *ret == SHAR_Error )
		{
			// Just load all normally
			load_type = LOAD_ALL; 
		}
		else // SHAR_FirstCaller
		{

		}
	}
	else
	{
		// HYPNOS TODO: Looks like this function was written to support
		// both 'shared' load and unshared, but now fails for anything but
		// shared. Could simplify if that is a requirement now
		*ret = SHAR_Error;
	}


	if( load_type == LOAD_ALL || load_type == LOAD_ALL_SHARED )
	{
		file_size = fileGetSize( file );
	}
	else //LOAD_SKELETONS_ONLY
	{
		fread( &file_size, sizeof( int ), 1, file );	
		fseek( file, 0, SEEK_SET );
	}

	if ( load_type == LOAD_ALL_SHARED )
	{
		bool bSuccess = sharedHeapAlloc( pHandle, file_size );

		if ( !bSuccess )
		{
			sharedHeapMemoryManagerUnlock();
			pHandle = NULL;
			load_type = LOAD_ALL;
			*ret = SHAR_Error;
			skeleton = malloc(file_size);
		}
		else
		{
			skeleton = pHandle->data;
		}
	}
	else 
		skeleton = malloc( file_size );

	assert( skeleton );
	fread( skeleton, 1, file_size, file );
	skeleton->backupAnimTrack = NULL;

	skeleton->bone_tracks = (void *)((int)skeleton->bone_tracks + (int)skeleton);

	if( load_type == LOAD_ALL || load_type == LOAD_ALL_SHARED ) //Fix up the data ptrs.
	{
		for( i = 0 ; i < skeleton->bone_track_count ; i++ )
		{
			bt = &(skeleton->bone_tracks[i]);

			bt->pos_idx	= (void *)((U32)bt->pos_idx	+ (U32)skeleton);
			bt->rot_idx	= (void *)((U32)bt->rot_idx	+ (U32)skeleton);

			// HYPNOS TODO: This is slowing down streaming and should
			// be done in a batch sweep and at tool conversion time
			animDebugCheckBoneTrackOnLoad( bt, fileName );
		}
	}

	if( skeleton->skeletonHeirarchy )
		skeleton->skeletonHeirarchy = (void *)((int)skeleton->skeletonHeirarchy + (int)skeleton);

	if ( load_type == LOAD_ALL_SHARED )
		sharedHeapMemoryManagerUnlock();
	return skeleton;
}

// fixup pointers and patch data post load
void animPatchLoadedRawData( SkeletonAnimTrack * skeleton )
{
	int i;

	skeleton->backupAnimTrack = NULL;
	skeleton->bone_tracks = (void *)((int)skeleton->bone_tracks + (int)skeleton);

	for( i = 0 ; i < skeleton->bone_track_count ; i++ )
	{
		BoneAnimTrack* bt = &(skeleton->bone_tracks[i]);

		bt->pos_idx	= (void *)((U32)bt->pos_idx	+ (U32)skeleton);
		bt->rot_idx	= (void *)((U32)bt->rot_idx	+ (U32)skeleton);
	}

	if( skeleton->skeletonHeirarchy )
		skeleton->skeletonHeirarchy = (void *)((int)skeleton->skeletonHeirarchy + (int)skeleton);
}

// walk the track data looking for anomalies
void animValidateTrackData( SkeletonAnimTrack * skeleton )
{
	int i;

	for( i = 0 ; i < skeleton->bone_track_count ; i++ )
	{
		BoneAnimTrack* bt = &(skeleton->bone_tracks[i]);
		animDebugCheckBoneTrackOnLoad( bt, skeleton->name );
	}
}

SkeletonAnimTrack * animLoadAnimFile( const char* fileName )
{
	SkeletonAnimTrack* pTrack = NULL;
	int file_size;
	FILE* f;

	f = fopen(fileName, "rb");
	if (!f)
	{
		Errorf("Could not load anim file");
		return NULL;
	}

	file_size = fileGetSize( f );
	pTrack = malloc(file_size);
	if (!pTrack)
	{
		Errorf("Not enough memory to load anim file");
		return NULL;
	}
	fread( pTrack, 1, file_size, f );

	animPatchLoadedRawData( pTrack );
	animValidateTrackData( pTrack );

	fclose(f);

	return pTrack;
}

static void UpCase(char *s)
{
	for(;*s;s++) *s = toupper(*s);
}

static SkeletonAnimTrack* animDevCheckUpdated(SkeletonAnimTrack* animTrack, const char* fileName, const char* nameClean)
{
	if(!global_state.no_file_change_check && isDevelopmentMode())
	{  
		if( fileHasBeenUpdated( fileName, &animTrack->fileAge ) ) //can change fileAge
		{
			stashRemovePointer( animEngine.tracksLoadedHT, nameClean , &animTrack);
			animTrack = 0; //Don't bother freeing the old one, someone might be using it
			animEngine.tracksLoadedCount--;
			assert( animEngine.tracksLoadedCount >= 0 );
		}
	}
	
	return animTrack;
}

static void animDevSetFileAge(SkeletonAnimTrack* animTrack, const char* fileName)
{
	if(!global_state.no_file_change_check && isDevelopmentMode())
	{
		animTrack->fileAge = 0;
		fileHasBeenUpdated( fileName, &animTrack->fileAge );
	}
}

static void animLinkToBaseSkeleton(SkeletonAnimTrack* animTrack, int loadType, const char* fileName, int useSharedMemory)
{
	SkeletonAnimTrack * backupAnimTrack;

	if ( useSharedMemory )
		sharedHeapMemoryManagerUnlock();
	backupAnimTrack = animGetAnimTrack( animTrack->baseAnimName, loadType, fileName );
	if ( useSharedMemory )
		sharedHeapMemoryManagerLock();
	animTrack->backupAnimTrack = backupAnimTrack;
	animTrack->skeletonHeirarchy = backupAnimTrack->skeletonHeirarchy;

	assert( backupAnimTrack && backupAnimTrack->skeletonHeirarchy );

	animTrack->max_hip_displacement = 4.0; //temp for testing visibility
}


SkeletonAnimTrack * animGetAnimTrack( const char * name, int loadType, const char *requestor )
{
	StashElement element;
	SkeletonAnimTrack * animTrack;
	char nameClean[MAX_ANIM_FILE_NAME_LEN];
	char fileName[MAX_ANIM_FILE_NAME_LEN];
	bool bUsingSharedMemory = (loadType == LOAD_ALL_SHARED);

	if( !name || !name[0] )
	{
		ErrorFilenamef(requestor, "ANIM: animGetAnimTrack called with no name\n" );
		return 0;
	}
	
	PERFINFO_AUTO_START("animGetAnimTrack", 1);

	PERFINFO_AUTO_START("top", 1);
	
	assert( strlen(name) < MAX_ANIM_FILE_NAME_LEN );
	strcpy( nameClean, name );
	UpCase(nameClean);

	//nameClean = something like "MALE/SKEL_READY" 
	//sprintf( fileName, "player_library/animations/%s.anim", nameClean );
	STR_COMBINE_BEGIN(fileName);
	STR_COMBINE_CAT("player_library/animations/");
	STR_COMBINE_CAT(nameClean);
	STR_COMBINE_CAT(".anim");
	STR_COMBINE_END();

	if( !animEngine.tracksLoadedHT )
	{
		animEngine.tracksLoadedHT = stashTableCreateWithStringKeys(10000, StashDeepCopyKeys);
	}
	
	animTrack = 0;

	/*Look in the hash table for this already loaded.  If so, return that.*/
	stashFindElement( animEngine.tracksLoadedHT, nameClean , &element);
	if( element )
	{
		animTrack = (SkeletonAnimTrack*)stashElementGetPointer(element);

		if(animTrack && !bUsingSharedMemory)
		{
			//Development-Only animation reloader.
			
			animTrack = animDevCheckUpdated(animTrack, fileName, nameClean);
		}
	}
	
	PERFINFO_AUTO_STOP_START("middle", 1);

	//If you haven't loaded it yet, or it's been updated, then load it up.
	if(!animTrack)
	{
		FILE * file = 0;
		bool bShouldWriteToSharedMemory = false;
		SharedHeapAcquireResult ret = SHAR_Error;

		PERFINFO_AUTO_STOP_START("middle2", 1);

		//TO DO the half opening thing for svr
		file = fileOpen( fileName, "rb");
		if( !file )
		{
			ErrorFilenamef(requestor, "ANIM: animation file '%s' doesn't exist!\n", fileName);
			PERFINFO_AUTO_STOP();
			PERFINFO_AUTO_STOP();
			return 0;
		}

		animTrack = animReadTrackFile( file, loadType, fileName, &ret );

		fileClose( file );

		if ( ret == SHAR_Error )
		{
			bUsingSharedMemory = false;
			bShouldWriteToSharedMemory = false;
		}
		else if ( ret == SHAR_DataAcquired )
		{
			bUsingSharedMemory = true;
			bShouldWriteToSharedMemory = false;
		}
		else // ret == SHAR_FirstCaller
		{
			bUsingSharedMemory = true;
			bShouldWriteToSharedMemory = true;
		}
		
		PERFINFO_AUTO_STOP_START("middle3", 1);

		//########### add it to the list of loaded stuff and check dev
		//Do before animGetAnimTrack below so you don't get caught in loop 
		stashAddPointer( animEngine.tracksLoadedHT, nameClean, animTrack, false);
		animEngine.tracksLoadedCount++;

		if ( bUsingSharedMemory && !bShouldWriteToSharedMemory && !animTrack->backupAnimTrack)
		{
			// We should always have a backupanimtrack... something is wrong, so write to shared memory to fix it
			bShouldWriteToSharedMemory = true;
		}

		if ( bShouldWriteToSharedMemory )
		{
			PERFINFO_AUTO_STOP_START("middle4", 1);

			// Lock the heap memory manager, so we can finish writing to this animtrack
			sharedHeapMemoryManagerLock();
		}

		// only do development mode stuff if we're not using shared memory
		//Development mode initialize fileAge
		if(!bUsingSharedMemory)
		{
			PERFINFO_AUTO_STOP_START("middle5", 1);

			animDevSetFileAge(animTrack, fileName);
		}
		//end development

		//########### A few minor initializations...
		//link to baseSkeleton
		if (!animTrack->backupAnimTrack && ( !bUsingSharedMemory || bShouldWriteToSharedMemory ) )
		{
			PERFINFO_AUTO_STOP_START("middle6", 1);

			animLinkToBaseSkeleton(animTrack, loadType, fileName, bShouldWriteToSharedMemory);
		}

		assert( animTrack->backupAnimTrack && animTrack->backupAnimTrack->skeletonHeirarchy );

		if ( bShouldWriteToSharedMemory )
		{
			PERFINFO_AUTO_STOP_START("middle7", 1);

			// Unlock the heap memory manager
			sharedHeapMemoryManagerUnlock();
		}


	}

	PERFINFO_AUTO_STOP();

	PERFINFO_AUTO_STOP();

	return animTrack;
}
