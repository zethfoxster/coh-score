#include "wininclude.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "assert.h"
#include "mathutil.h"
#include "file.h"
#include "error.h"
#include "utils.h"
#include "animtrack.h"
#include "animtrackanimate.h"
#include "StashTable.h"
#include "strings_opt.h"
#include "netcomp.h"
#include "quat.h"

int total_uncompressed_position_keys = 0;
int	total_added_rot_nodes;
int	total_added_rot_keys;
int	total_added_pos_nodes;
int	total_added_pos_keys;
int total_dummy_rotation_keys;
int total_dummy_position_keys;
int runhist[2][10000];
int maxrun[2];
int non_matching_counts;

int total_saved_keys[2];
int total_keys[2];

int end_run_due_to_too_much_delta_error[2];
int end_run_due_to_switched_missing_key[2];
int end_run_due_to_end_of_track[2];
int total_runs[2];
int total_useless_runs[2];


#define CORRECTIONBITS 5
#define CORRECTIONSIZE 32
#define ACCEPTABLEROTERROR 15
#define ACCEPTABLEPOSERROR 0.015

//#ifdef assert
//	#undef assert
//#endif
//	#define assert(exp)				((void)0)


//negates quat if biggest value is negative, and returns idx of bigget value
int makeBiggestValuePositive( F32 * quat )
{	
	F32 biggest;
	int biggestidx;
	int i;
	
	//Find the value to leave out
	//(I think I need to check for biggest and littlest)
	biggest = 0.0;
	biggestidx = -1;
	for( i = 0 ; i < 4 ; i++)
	{
		if( ABS(quat[i]) > biggest )
		{
			biggest = ABS(quat[i]);
			biggestidx = i;
		}
	}
	assert(biggestidx != -1);

	//If the biggest value is negative, negate the quaternion so we don't have to store the sign
	if( quat[biggestidx] < 0 )
	{
		quat[0] *= -1.0; 
		quat[1] *= -1.0; 
		quat[2] *= -1.0; 
		quat[3] *= -1.0; 
	}

	return biggestidx;
}


// ### Rotation Keys ############

//12 bit lookup table

U32 glob_cidx[4];
int cmissing;




void compressQuatToFiveBytes( F32 * quat, char * cquat )
{
	static F32 table[LOOKUPSIZE12BIT]; 
	static F32 table_inited = 0;
	//F32 biggest = 0;
	int i ,next, j;
	int biggestidx;
	U32 qidxs[4];
	F32 dist_from_min;
	F32 dist_from_max;
	F32 min, max;

	//Initialize lookup table
	F32 max_value;

	max_value = (1.0/sqrtf(2.0));

	if(!table_inited)
	{
		initLookUpTable( table );
		table_inited = 1;
	}

	//find biggest value and negate quat to make it positive if needed
	biggestidx = makeBiggestValuePositive( quat );

	//find the lookup values of the other 3
	next = 0;
	for( i = 0 ; i < 4 ; i++ )
	{
		if( i != biggestidx)
		{
			qidxs[next] = -1;
			assert( ABS(quat[i]) <= max_value + 0.001 );
			for(j = 0 ; j < LOOKUPSIZE12BIT - 1 ; j++ )
			{
				if( table[j] >= quat[i]  ) 
				{
					min = table[j];
					max = table[j + 1];

					dist_from_min = ABS(min - quat[i]);
					dist_from_max = ABS(max - quat[i]);

					if( dist_from_min < dist_from_max )
						qidxs[next] = j;
					else
						qidxs[next] = j + 1;

					break;
				}
			}

			if( qidxs[next] == -1 )
			{
				qidxs[next] = (LOOKUPSIZE12BIT - 1); //fence post problem...
			}
			assert( qidxs[next] != -1 );
			next++;
		}
	}

	assert( qidxs[0] >= 0 && qidxs[0] < LOOKUPSIZE12BIT );
	assert( qidxs[1] >= 0 && qidxs[1] < LOOKUPSIZE12BIT );
	assert( qidxs[2] >= 0 && qidxs[2] < LOOKUPSIZE12BIT );

	glob_cidx[0] = qidxs[0];	//temp debug
	glob_cidx[1] = qidxs[1];	
	glob_cidx[2] = qidxs[2];	
	glob_cidx[3] = qidxs[3];	
	cmissing = biggestidx;

	//pack lookups and missing quat qidxs in this format:
	//NULL (2 bits), biggestidx (2 bits), qidxs[0] (12 bits), qidxs[1] (12 bits), qidxs[2] (12 bits)
	{
		U32 * bytes;
		U8 * top;
		U32 missing;

		missing = biggestidx;
	
		memset( cquat, 0, COMPRESSEDQUATSIZE );
		
		//Pack low bytes
		bytes = (U32*)&cquat[1];
		*bytes = qidxs[2];
		*bytes |= qidxs[1] << 12;
		*bytes |= qidxs[0] << 24;

		//Pack high byte
		top = (U8*)&cquat[0];
		*top = missing << 4;
		*top |= qidxs[0] >> 8;

		//assert( (*bytes) <= 0x00000fff );
		//assert( ((*bytes) == 0 || (*bytes) > 0x00000fff) &&  (*bytes) <= 0x00fff000 );
		//assert( (*bytes) <= 0x00ffffff );
		//assert( (*bytes) <= 0xfffff000 ); 

		assert( (*top) <= 0x3f ); //only low six bits are used... 
		assert( cquat[0] < 64 );
	
	}
}

static void compressQuatNonLinear( F32 * quat, char * cquat )
{
	//F32 biggest = 0;
	int i , iCompressedIdx;
	int biggestidx;
	U32 compressedQuatElem[3];


	//find biggest value and negate quat to make it positive if needed
	biggestidx = makeBiggestValuePositive( quat );

	//find the lookup values of the other 3
	iCompressedIdx = 0;
	for( i = 0 ; i < 4 ; i++ )
	{
		if( i != biggestidx)
		{
			compressedQuatElem[iCompressedIdx++] = packQuatElemQuarterPi(quat[i], 12);
		}
	}

	//pack lookups and missing quat qidxs in this format:
	//NULL (2 bits), biggestidx (2 bits), qidxs[0] (12 bits), qidxs[1] (12 bits), qidxs[2] (12 bits)
	{
		U32 * bytes;
		U8 * top;
		U32 missing;

		missing = biggestidx;

		memset( cquat, 0, COMPRESSEDQUATSIZE );

		//Pack low bytes
		bytes = (U32*)&cquat[1];
		*bytes = compressedQuatElem[2];
		*bytes |= compressedQuatElem[1] << 12;
		*bytes |= compressedQuatElem[0] << 24;

		//Pack high byte
		top = (U8*)&cquat[0];
		*top = missing << 4;
		*top |= compressedQuatElem[0] >> 8;

		assert( (*top) <= 0x3f ); //only low six bits are used... 
		assert( cquat[0] < 64 );

	}
}

//This file is all the math of actually doing the animation, compressing, decompressing


//This function doesn't do much anymore
static INLINEDBG void animCalcTrackIdxs( int frame_count, int intTime, F32 remainderTime, int *minp, int *maxp, F32 *fracp, int interpolate )
{
	if( ((F32)intTime + remainderTime) > frame_count - 1 ) //PTOR if time overruns the length of the thing, something's wrong, I think
	{
		intTime  = frame_count - 1;
		remainderTime = 0;
	}

	*minp = intTime;

	if( interpolate ) //does all the work, so don't expect performance increase
	{
		*fracp = remainderTime;
		*maxp = *minp + 1;
		if( *maxp >= frame_count )
			*maxp = 0;
	}
	else
	{
		*fracp = 0;
		*maxp = *minp;
	}
}


/* Try to think of how to generalize the most possible */
static INLINEDBG void deriveMinAndMaxRotKeys( int low, int high, const BoneAnimTrack * bt, U8 * value_min, U8 * value_max )
{
	const U8 * fullkeys;
	U16 idx;

	fullkeys = bt->rot_idx;

	idx = low; /*CORRECTION FIX bt->rot_corrections[low];*/
	//assert( 0x8000 & idx );
	//idx = (0x7fff & idx );
	assert( idx >= 0 && idx < bt->rot_fullkeycount );

	memcpy( value_min, &fullkeys[ idx * SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES ], SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES );

	idx = high; /*CORRECTION FIX bt->rot_corrections[high];*/
	//assert( 0x8000 & idx );
	//idx = (0x7fff & idx );
	assert( idx >= 0 && idx < bt->rot_fullkeycount );

	memcpy( value_max, &fullkeys[ idx * SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES ], SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES );

}

static INLINEDBG void deriveMinAndMaxPosKeys( int low, int high, const BoneAnimTrack * bt, U8 * value_min, U8 * value_max )
{
	const U8 * fullkeys;
	U16 idx;

	fullkeys = bt->pos_idx;

	idx = low; /*CORRECTION FIX bt->pos_corrections[low];*/
	//assert( 0x8000 & idx );
	//idx = (0x7fff & idx );
	assert( idx >= 0 && idx < bt->pos_fullkeycount );

	if( bt->flags & POSITION_UNCOMPRESSED )
		memcpy( value_min, &fullkeys[ idx * SIZE_OF_UNCOMPRESSED_POSKEY ], SIZE_OF_UNCOMPRESSED_POSKEY );
	else if ( bt->flags & POSITION_COMPRESSED_TO_6_BYTES )
		memcpy( value_min, &fullkeys[ idx * SIZE_OF_POSKEY_COMPRESSED_TO_6_BYTES ], SIZE_OF_POSKEY_COMPRESSED_TO_6_BYTES );

	idx = high; /*CORRECTION FIX bt->pos_corrections[high];*/
	//assert( 0x8000 & idx );
	//idx = (0x7fff & idx );
	assert( idx >= 0 && idx < bt->pos_fullkeycount );

	if( bt->flags & POSITION_UNCOMPRESSED )
		memcpy( value_max, &fullkeys[ idx * SIZE_OF_UNCOMPRESSED_POSKEY ], SIZE_OF_UNCOMPRESSED_POSKEY );
	else if ( bt->flags & POSITION_COMPRESSED_TO_6_BYTES )
		memcpy( value_max, &fullkeys[ idx * SIZE_OF_POSKEY_COMPRESSED_TO_6_BYTES ], SIZE_OF_POSKEY_COMPRESSED_TO_6_BYTES );

}

static F32 table[LOOKUPSIZE12BIT];
static int table_inited = 0;
U32 glob_eidx[4]; //Debug 
int emissing;		//Debug

void animExpand5ByteQuat( U8 * cquat, Quat eq )
{
	F32	sqr;
	U32 qidxs[3];

	Quat quat;
	U32 i, missing, idx = 0;

	if(!table_inited) //TO DO move out
	{
		initLookUpTable( table );
		table_inited = 1;
	}
	
	unPack5ByteQuat( cquat, qidxs, &missing );

	//Debug 
	glob_eidx[0] = qidxs[0];	
	glob_eidx[1] = qidxs[1];	
	glob_eidx[2] = qidxs[2];		
	emissing = missing;

	//lookup idxs
	for( i = 0 ; i < 4 ; i++ )
	{
		if( i != missing )
		{
			quat[i] = table[qidxs[idx++]];
		}
	}

	quat[missing] = 0;
	assert( finiteVec4( quat ) );

	//calc missing element //TO DO do fast root
	if( missing == 3 )
	{ 
		sqr = ((1 - quat[2]*quat[2]) - quat[1]*quat[1]) - quat[0]*quat[0];
		assert( sqr > -0.1 );
		if(sqr < 0 )
			sqr = 0;
		quat[3] = sqrt( sqr );
		assert( quat[3] >= quat[0]-QERROR && quat[3] >= quat[1]-QERROR && quat[3] >= quat[2]-QERROR );
	}
	else if (missing == 2)
	{
		sqr = ((1 - quat[3]*quat[3]) - quat[1]*quat[1]) - quat[0]*quat[0]+0.001;
		assert( sqr > -0.001 );
		if(sqr < 0 )
			sqr = 0;
		quat[2] = sqrt( sqr );
		assert( quat[2] >= quat[0]-QERROR && quat[2] >= quat[1]-QERROR && quat[2] >= quat[3]-QERROR );
	}
	else if (missing == 1)
	{
		sqr = ((1 - quat[3]*quat[3]) - quat[2]*quat[2]) - quat[0]*quat[0];
		assert( sqr > -0.001 );
		if(sqr < 0 )
			sqr = 0;
		quat[1] = sqrt( sqr );
		assert( quat[1] >= quat[0]-QERROR && quat[1] >= quat[2]-QERROR && quat[1] >= quat[3]-QERROR );
	}
	else if (missing == 0)
	{
		sqr = ((1 - quat[3]*quat[3]) - quat[2]*quat[2]) - quat[1]*quat[1];
		assert( sqr > -0.001 );
		if(sqr < 0 )
			sqr = 0;
		quat[0] = sqrt( sqr );
		assert( quat[0] >= quat[1]-QERROR && quat[0] >= quat[2]-QERROR && quat[0] >= quat[3]-QERROR );
	}

	assert( finiteVec4( quat ) );
	
	//copy into the quat (is there any value in having the quat be a struct like this, not a Vec4?)
	copyQuat(quat, eq);
}

void animExpandNonLinearQuat( U8 * cquat, Quat eq )
{
	F32	sqr;
	U32 compressedQuatElem[3];

	Quat quat;
	U32 i, missing, idx = 0;
	int iCompressedQuatIdx = 0;

	unPack5ByteQuat( cquat, compressedQuatElem, &missing );


	//lookup idxs
	for( i = 0 ; i < 4 ; i++ )
	{
		if( i != missing )
		{
			quat[i] = unpackQuatElemQuarterPi(compressedQuatElem[iCompressedQuatIdx++], 12);
		}
	}

	quat[missing] = 0;
	assert( finiteVec4( quat ) );

	//calc missing element //TO DO do fast root
	if( missing == 3 )
	{ 
		sqr = ((1 - quat[2]*quat[2]) - quat[1]*quat[1]) - quat[0]*quat[0];
		assert( sqr > -0.1 );
		if(sqr < 0 )
			sqr = 0;
		quat[3] = sqrt( sqr );
		assert( quat[3] >= quat[0]-QERROR && quat[3] >= quat[1]-QERROR && quat[3] >= quat[2]-QERROR );
	}
	else if (missing == 2)
	{
		sqr = ((1 - quat[3]*quat[3]) - quat[1]*quat[1]) - quat[0]*quat[0]+0.001;
		assert( sqr > -0.001 );
		if(sqr < 0 )
			sqr = 0;
		quat[2] = sqrt( sqr );
		assert( quat[2] >= quat[0]-QERROR && quat[2] >= quat[1]-QERROR && quat[2] >= quat[3]-QERROR );
	}
	else if (missing == 1)
	{
		sqr = ((1 - quat[3]*quat[3]) - quat[2]*quat[2]) - quat[0]*quat[0];
		assert( sqr > -0.001 );
		if(sqr < 0 )
			sqr = 0;
		quat[1] = sqrt( sqr );
		assert( quat[1] >= quat[0]-QERROR && quat[1] >= quat[2]-QERROR && quat[1] >= quat[3]-QERROR );
	}
	else if (missing == 0)
	{
		sqr = ((1 - quat[3]*quat[3]) - quat[2]*quat[2]) - quat[1]*quat[1];
		assert( sqr > -0.001 );
		if(sqr < 0 )
			sqr = 0;
		quat[0] = sqrt( sqr );
		assert( quat[0] >= quat[1]-QERROR && quat[0] >= quat[2]-QERROR && quat[0] >= quat[3]-QERROR );
	}

	assert( finiteVec4( quat ) );

	//copy into the quat (is there any value in having the quat be a struct like this, not a Vec4?)
	copyQuat(quat, eq);
}

void animGetRotationValue( int intTime, F32 remainderTime, const BoneAnimTrack * bt, Quat newrot, int interpolate)
{
	int			min,max;
	F32			frac;
	Quat		expanded_q1;
	Quat		expanded_q2;
	U8			cquat_min[SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES];
	U8			cquat_max[SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES];

	assert(bt);

	//########### Get rotation key ####################################	
	if( bt->rot_count == 1 )
	{
		//TO DO for performance, should I pre-expand this one, and just copy it in?
		min = max = frac = 0;
	}
	else
	{
		animCalcTrackIdxs(bt->rot_count, intTime, remainderTime, &min, &max, &frac, interpolate );
	}

	deriveMinAndMaxRotKeys( min, max, bt, cquat_min, cquat_max );

	if( bt->flags & ROTATION_COMPRESSED_TO_5_BYTES )
	{
		U8 * cquats8;
		cquats8 = (U8*)bt->rot_idx;
		animExpand5ByteQuat( &cquats8[min * 5], expanded_q1 );
		animExpand5ByteQuat( &cquats8[max * 5], expanded_q2 );
	}
	else if( bt->flags & ROTATION_COMPRESSED_TO_8_BYTES )
	{
		S16	* cquats16; 
		cquats16 = (S16*)bt->rot_idx; 
		animExpand8ByteQuat( &cquats16[min * 4], expanded_q1 );
		animExpand8ByteQuat( &cquats16[max * 4], expanded_q2 );
	}
	else if( bt->flags & ROTATION_UNCOMPRESSED )
	{
		memcpy( &expanded_q1, &(((F32*)bt->rot_idx)[ min * 4 ]), 16 );
		memcpy( &expanded_q2, &(((F32*)bt->rot_idx)[ max * 4 ]), 16 );
	}
	else if ( bt->flags & ROTATION_COMPRESSED_NONLINEAR )
	{
		U8 * cquats8;
		cquats8 = (U8*)bt->rot_idx;
		animExpandNonLinearQuat( &cquats8[min * 5], expanded_q1 );
		animExpandNonLinearQuat( &cquats8[max * 5], expanded_q2 );
	}
	else
		assert(0);

	assert( !vec4IsZero(expanded_q1));
	assert( !vec4IsZero(expanded_q2));

	quatInterp(frac, expanded_q1, expanded_q2, newrot );
}


INLINEDBG void animExpand6BytePosition( S16 * compressed, Vec3 expanded )
{ 
	expanded[0] = EFACTOR_6BYTE_POS * (F32)compressed[0];
	expanded[1] = EFACTOR_6BYTE_POS * (F32)compressed[1];
	expanded[2] = EFACTOR_6BYTE_POS * (F32)compressed[2];
}


void animGetPositionValue( int intTime, F32 remainderTime, const BoneAnimTrack * bt, Vec3 newpos, int interpolate)
{
	int			min,max;
	F32			frac;
	Vec3		expanded_pos1;
	Vec3		expanded_pos2;

	//################## Get Translation key ##################################
	if( bt->pos_count == 1 )
	{
		//TO DO for performance, should I pre-expand this one, and just write it in?
		min = max = frac = 0;
	}
	else
	{
		animCalcTrackIdxs(bt->pos_count, intTime, remainderTime, &min, &max, &frac, interpolate );
	}

	if( bt->flags & POSITION_COMPRESSED_TO_6_BYTES )
	{
		S16 pos_min[ 6 ]; //POSITION_COMPRESSED_TO_6_BYTES/2
		S16 pos_max[ 6 ];

		deriveMinAndMaxPosKeys( min, max, bt, (U8*)pos_min, (U8*)pos_max );

		animExpand6BytePosition( pos_min, expanded_pos1 );
		animExpand6BytePosition( pos_max, expanded_pos2 );
		
		//animExpand6BytePosition( &(((S16*)bt->pos_idx )[ min * 3 ]), expanded_pos1 );
		//animExpand6BytePosition( &(((S16*)bt->pos_idx )[ max * 3 ]), expanded_pos2 );
	}
	else if( bt->flags & POSITION_UNCOMPRESSED )
	{
		F32 pos_min[3]; //POSITION_UNCOMPRESSED/2
		F32 pos_max[3];

		//pos_min = &(((F32*)bt->pos_idx)[ min * 3 ]);
		//pos_max = &(((F32*)bt->pos_idx)[ max * 3 ]);

		deriveMinAndMaxPosKeys( min, max, bt, (U8*)pos_min, (U8*)pos_max );

		copyVec3( pos_min, expanded_pos1 );
		copyVec3( pos_max, expanded_pos2 );
		assert( finiteVec3( pos_min ) );
		assert( pos_min[0] < POS_BIGGEST && pos_min[1] < POS_BIGGEST && pos_min[2] < POS_BIGGEST );
	}
	else
		assert(0);

	posInterp( frac, expanded_pos1, expanded_pos2, newpos );
}


static void compress5ByteRotationKeys( AnimKeys * keys )
{
	F32 * oldvals;
	U8 * newvals;
	int i;
	static F32 max_pos = 0;
	
	oldvals = (F32 *)keys->vals;
	
	newvals = calloc( keys->count, COMPRESSEDQUATSIZE ); 
	
	for( i = 0 ; i < keys->count ; i++ )
	{
		F32 * fquat_in;
		U8 * cquat; 
		Quat fquat_out; //debug

		fquat_in = &(oldvals[i*4]);
		cquat = &(newvals[ i*COMPRESSEDQUATSIZE ]);

		assert( finiteVec4( fquat_in ) );
		compressQuatToFiveBytes( fquat_in, cquat );

		//debug
		animExpand5ByteQuat( cquat, fquat_out );
	}

	free(keys->vals); 
	keys->vals = newvals;
				
	return;
}


static void compress5ByteRotationKeysNonLinear( AnimKeys * keys )
{
	F32 * oldvals;
	U8 * newvals;
	int i;
	static F32 max_pos = 0;

	oldvals = (F32 *)keys->vals;

	newvals = calloc( keys->count, COMPRESSEDQUATSIZE ); 

	for( i = 0 ; i < keys->count ; i++ )
	{
		F32 * fquat_in;
		U8 * cquat; 
		Quat fquat_out; //debug

		fquat_in = &(oldvals[i*4]);
		cquat = &(newvals[ i*COMPRESSEDQUATSIZE ]);

		assert( finiteVec4( fquat_in ) );
		compressQuatNonLinear( fquat_in, cquat );

		//debug
		animExpandNonLinearQuat( cquat, fquat_out );
	}

	free(keys->vals); 
	keys->vals = newvals;

	return;
}


//// ###################### Pack Animaton Tracks #########################

// GETANIM ######## Get rid of Times 
static int roundOffToInt( F32 x )
{
	int choppedx;
	F32 decimal;

	choppedx = (int)x;

	decimal = ( x - ((F32)x) );
	if( decimal > .5 )
		x += 1;

	return (int)x;
}

#define MAX_MULT 300  //temporary memory for getvrml to alloc for extra normals

static void normalizeKeyTimes( AnimKeys * oldkey, int keysize )
{
	F32 * newvals;
	F32 * oldvals;
	int * normalizedtimes;
	int newcount, i, temp_alloc_size;
	
	//get a normalized list of times
	normalizedtimes = calloc( oldkey->count, sizeof( int ) );
	for( i = 0 ; i < oldkey->count ; i++ )
	{
		normalizedtimes[i] = roundOffToInt( oldkey->times[i] );
	}

	//get old and new values
	temp_alloc_size = MAX_MULT * oldkey->count;
	newvals = calloc( temp_alloc_size, sizeof( F32 ) * keysize ); 
	assert(newvals);

	oldvals = (F32 *)oldkey->vals;

	//add keys for each time step.
	newcount = 0;

	for( i = 0 ; i < oldkey->count ; i++ )
	{
		while( normalizedtimes[i] >= newcount )
		{
			//TO DO smoothly interp here...
			memcpy( &newvals[ newcount * keysize ], &oldvals[ i * keysize ], sizeof(F32) * keysize );
			newcount++;

			//Just to be safe...
			if( newcount >= temp_alloc_size )
			{
				temp_alloc_size *= 2;
				newvals = realloc( newvals, temp_alloc_size );
				assert( temp_alloc_size < 10000000 ); //ten megs means something broke for sure 
			}

			//printf(" %d ", newcount ); 
		}
	}

	if( newcount - oldkey->count > 0 )
		printf( "\nAdded %d keys", newcount - oldkey->count);
	if(keysize == 4)
	{
		total_added_rot_nodes++;
		total_added_rot_keys += newcount - oldkey->count ;
	}
	else
	{
		total_added_pos_nodes++;
		total_added_pos_keys += newcount - oldkey->count ;
	}

	newvals = realloc( newvals, newcount * sizeof( F32 ) * keysize );   
	//now copy back to the old key
	free( oldkey->vals );
	free( oldkey->times );
	oldkey->times = 0;
	oldkey->vals = newvals;
	oldkey->count = newcount;

	//clean up
	free(normalizedtimes);
}


static F32 smallestKeyStep(AnimKeys *keys)
{
int		i;
F32		min_step = 1000000,d;

	for(i=0;i<keys->count-1;i++)
	{
		d = keys->times[i+1] - keys->times[i];
		if (d && d <= min_step)
			min_step = d;
	}
	return min_step;
}


/*This is strange. Finds the key frame that is the smallest, and figures that that key must be 
1/30th of a second because I don't know why.  But given that's true, the rest makes some sense.
Dividing all the key times by the 1/30th number converts them all to 1/30th numbers, and adding the 
step_size/4 helps the rounding.  The result should be 0,1,2,3... which this seems like a lot of work 
to get.  But in reality, it will sometimes go more like this 0,1,2,79,80,81... I don't know why.  Until I figure out why,
or figure out what to do about it, I can't really remove the timecodes.
*/
static void keysToFrames(AnimKeys *keys)
{
	int		i,x;
	F32		step_size;

	step_size = smallestKeyStep(keys);  // supposed to be 1/30th of a second

	for(i=0;i<keys->count;i++)
	{
		x = (keys->times[i] + step_size/4) / step_size;
		keys->times[i] = x;
	}
}


//Interpolate 3 int vector //TO DO scaling
int * interpIntVec3( F32 scale, int * start, int * end )
{
	static int r[3];
	Vec3 floatr;
	
	subVec3( end, start, floatr );
	scaleVec3( floatr, scale, floatr);
	addVec3(start, floatr, floatr);
	r[0]  = (int)(floatr[0] + (0.5 * (ABS(floatr[0]))/floatr[0])); //fix rounding vs truncating problem
	r[1]  = (int)(floatr[1] + (0.5 * (ABS(floatr[0]))/floatr[0]));
	r[2]  = (int)(floatr[2] + (0.5 * (ABS(floatr[0]))/floatr[0]));
	return r;
}  


void quantizeCorrectionsToInts( F32 * corrections, int * corrections_quantized_to_fivebit, int count )
{
	int i;

	for( i = 0 ; i < count ; i++ )
	{
		assert( ABS(corrections[i]) <= ACCEPTABLEPOSERROR );

		corrections_quantized_to_fivebit[i] = corrections[i]/ACCEPTABLEPOSERROR * 15;
	}
}


///########################################
//#######################
//######

//New full keys array return
U8 * getRidOfUneededFullKeys(  U8 * oldfullkeys, int oldfullkeycount, U16 * corrections,  int newfullkeycount, int keysize )
{
	int i;
	U8 * newfullkeys;
	int debug_full_count = 0;

	newfullkeys = calloc( keysize, newfullkeycount );

	for( i = 0 ; i < oldfullkeycount ; i++ )
	{
		if( corrections[i] & (1<<15) )
		{
			memcpy( &newfullkeys[debug_full_count*keysize], &oldfullkeys[i*keysize], keysize );
			debug_full_count++;
		}
	}

	assert( debug_full_count == newfullkeycount );

	free( oldfullkeys);

	//Debug makes some assumptions about what good values are, probably ought to remove this when done debugging
	for( i = 0 ; i < newfullkeycount ; i++ )
	{
		if( keysize == SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES )
		{
			assert( newfullkeys[i * keysize ] < 64 );
		}
		else if(keysize == SIZE_OF_POSKEY_COMPRESSED_TO_6_BYTES )
		{
			//um, I guess any number is good?  Maybe check for, um I don't know
		}
		else if( keysize ==  SIZE_OF_UNCOMPRESSED_POSKEY )
		{
			assert( finiteVec3( (F32*)&newfullkeys[ i * keysize ] ) );
		}
		else
			assert(0);
	}
	//end debug

	return newfullkeys;
}

void checkGoodCompressedVal( F32 * vec, int type)
{}

void checkGoodVal( F32 * vec, int type)
{
	if( type == POSITION_KEY )
	{
		assert(	finiteVec3( vec ) );
		//assert( vec[0] < 1000 );
		//assert( vec[1] < 1000 );
		//assert( vec[2] < 1000 );
	}
	else if(type == ROTATION_KEY )
	{
		F32 mag;
		assert( finiteVec4( vec ) );
		mag = vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]+vec[3]*vec[3] ;
		assert( mag > 0.9999 && mag < 1.0001 ); 
	}
	else
		assert(0);
}


int calcPositionRun( F32 * corrections, F32 * keys, int keysize, F32 acceptable_error, int max_run_len )
{
	F32 * correct_idxs;
	F32 interped_idxs[3];
	F32 correction_needed[3];
	F32 correction_delta_needed[3];
	F32 curr_run_error[3];
	int trial_run_len, i;
	int run_len, success;

	run_len = 1;		//you can always have a run of one, that means the next key is a full key
	success = 0; //PTOR temp remove while I check basics.  
	trial_run_len = 2;

	while( success ) 
	{
		trial_run_len = run_len + 1;  //try a higher run length
		if( trial_run_len > max_run_len ) //max_run_length always assumes there is at least one good key in the array after it to do lerping on
		{
			success = 0;
			end_run_due_to_end_of_track[POSITION_KEY]++;
		}
		/*Else test each key with the proposed new end point.  Success means the lerp correction delta 
		was <= the max value the correction array can hold.  */
		else
		{
			curr_run_error[0] = 0; //the correction value of the last frame. Hopefully it will not change much this frame
			curr_run_error[1] = 0;
			curr_run_error[2] = 0;
			for( i = 1 ; i < trial_run_len ; i++ ) //i = 1: no need to test the full key
			{
				//find correction delta needed for this frame
				correct_idxs	= &keys[ i * keysize ];	//idxs without any error

				posInterp( (F32)i/((F32)trial_run_len), &keys[0], &keys[trial_run_len * keysize ], interped_idxs ); //idxs lerping generates  
				subVec3( correct_idxs, interped_idxs, correction_needed );  //works for ints (cheesy, yes)
				subVec3( correction_needed, curr_run_error, correction_delta_needed ); 

				//calc new curr_run_error for next frame
				addVec3(curr_run_error, correction_delta_needed, curr_run_error ); 
				//assert( curr_run_error[0] == correction_needed[0] && curr_run_error[1] == correction_needed[1] && curr_run_error[2] == correction_needed[2]);

				if( ABS(correction_delta_needed[0]) > acceptable_error ||
					ABS(correction_delta_needed[1]) > acceptable_error ||
					ABS(correction_delta_needed[2]) > acceptable_error )
				{ 
					success = 0; //this trial run failed, this run is over with run_len one less than this
					end_run_due_to_too_much_delta_error[POSITION_KEY]++;
					break;
				}
			}
		}
		if(success)
			run_len = trial_run_len;
	}
	run_len = trial_run_len - 1;

	//Now that we have the actual acceptable run length, commit it
	//exactly same code as above with run_len not trial_run_len
	curr_run_error[0] = 0; //the correction value of the last frame. Hopefully it will not change much this frame
	curr_run_error[1] = 0;
	curr_run_error[2] = 0;

	for( i = 1 ; i < run_len ; i++ ) //first error is always 0
	{
		//find correction delta needed for this frame
		correct_idxs	= &keys[ i * keysize ];	//idxs without any error
		posInterp( (F32)i/((F32)run_len), &keys[0], &keys[run_len * keysize ], interped_idxs ); //idxs lerping generates  
		subVec3( correct_idxs, interped_idxs, correction_needed );  //works for ints (cheesy, yes)
		subVec3( correction_needed, curr_run_error, correction_delta_needed ); 

		//calc new curr_run_error for next frame
		addVec3(curr_run_error, correction_delta_needed, curr_run_error ); 
		assert( nearSameVec3Tol( curr_run_error, correction_needed, 0.000001 ) );

		//test if this correction delta is within range //debug only at this point
		if( ABS(correction_delta_needed[0]) > acceptable_error ||
			ABS(correction_delta_needed[1]) > acceptable_error ||
			ABS(correction_delta_needed[2]) > acceptable_error )
		{
			assert(0);
		}//*/

		//copy this correction delta to the correction run 
		copyVec3( correction_delta_needed, (&corrections[ i * keysize ]) );
	} 	

	return run_len;
}


//Simple inplementation at first doesn't allow for changes in CORRECTIONBITS, assumed 5
//will work for positions and rotations
void compressCorrectionArray( U16 * compressed, int * full_corrections, U8 * fullkeyidxs, int keysize, int count )
{
	int * correction;
	int i;
	U16 curr_full_idx = 0;

	for( i = 0 ; i < count ; i++ )
	{
		if( fullkeyidxs[i] ) //either tag it as a full key and write the full key idx 
		{
			assert( compressed[i] == 0 );

			compressed[i] |= (fullkeyidxs[i] & 0x00000001) << 15; //set whether this is a full key
			compressed[i] |= curr_full_idx;
			curr_full_idx++; //later when I compress the full key array, they magically match up again
		}
		else //or write in the correction
		{
			//Then get the compressed bit pattern in ranged_corr
			correction = &full_corrections[i*keysize];

			correction[0] += 16;
			correction[1] += 16;
			correction[2] += 16;

			assert(correction[0] <= 31 && correction[0] >=0);
			assert(correction[1] <= 31 && correction[1] >=0);
			assert(correction[2] <= 31 && correction[2] >=0);

			compressed[i] |= (correction[0] & 0x0000001f) << 0;
			compressed[i] |= (correction[1] & 0x000003e0) << 5;
			compressed[i] |= (correction[2] & 0x00007c00) << 10;
			compressed[i] |= (fullkeyidxs[i] & 0x00000001) << 15; //set whether this is a full key
		}
	}
}


int calcRun( int * corrections, U32 * keys, U32 * missing_key, int keysize, int acceptable_error, int max_run_len )
{
	int * correct_idxs;
	int * interped_idxs;
	int correction_needed[3];
	int correction_delta_needed[3];
	int curr_run_error[3];
	int trial_run_len, i;
	int run_len, success;

	run_len = 1;		//you can always have a run of one, that means the next key is a full key
	success = 0;//1;  //PTOR turn off the thing for a little bit
	trial_run_len = 2;

	while( success ) 
	{
		trial_run_len = run_len + 1;  //try a higher run length
		if( trial_run_len > max_run_len ) //max_run_length always assumes there is at least one good key in the array after it to do lerping on
		{
			success = 0;
			end_run_due_to_end_of_track[ROTATION_KEY]++;
		}
		/*If you changed the missing idx, the run is over: you need a full key*/
		else if( missing_key[trial_run_len] != missing_key[0] )
		{
			success = 0;
			end_run_due_to_switched_missing_key[ROTATION_KEY]++;
		}
		/*Else test each key with the proposed new end point.  Success means the lerp correction delta 
		was <= the max value the correction array can hold.  */
		else
		{
			curr_run_error[0] = 0; //the correction value of the last frame. Hopefully it will not change much this frame
			curr_run_error[1] = 0;
			curr_run_error[2] = 0;
			for( i = 1 ; i < trial_run_len ; i++ ) //i = 1: no need to test the full key
			{
				//find correction delta needed for this frame
				correct_idxs	= &keys[ i * keysize ];	//idxs without any error

				interped_idxs	= interpIntVec3( (F32)i/((F32)trial_run_len), &keys[0], &keys[trial_run_len * keysize ] ); //idxs lerping generates  
				subVec3( correct_idxs, interped_idxs, correction_needed );  //works for ints (cheesy, yes)
				subVec3( correction_needed, curr_run_error, correction_delta_needed ); 
				
				//calc new curr_run_error for next frame
				addVec3(curr_run_error, correction_delta_needed, curr_run_error ); 
				assert( curr_run_error[0] == correction_needed[0] && curr_run_error[1] == correction_needed[1] && curr_run_error[2] == correction_needed[2]);

				//if( i == 1 ) // key SUPERTEMP TO SEE IF DOING A SIMPLE FIRST GUESS WOULD HELP
				//	acceptable_error = 128 ; 

				if( ABS(correction_delta_needed[0]) > acceptable_error ||
					ABS(correction_delta_needed[1]) > acceptable_error ||
					ABS(correction_delta_needed[2]) > acceptable_error )
				{ 
					success = 0; //this trial run failed, this run is over with run_len one less than this
					end_run_due_to_too_much_delta_error[ROTATION_KEY]++;
					break;
				}//

				//SUPERTEMP 
				//acceptable_error = 15;
			}
		}
		if(success)
			run_len = trial_run_len;
	}
	run_len = trial_run_len - 1;

	//Now that we have the actual acceptable run length, commit it
	curr_run_error[0] = 0; //the correction value of the last frame. Hopefully it will not change much this frame
	curr_run_error[1] = 0;
	curr_run_error[2] = 0;

	for( i = 1 ; i < run_len ; i++ ) //first error is always 0
	{
		//find correction delta needed for this frame
		correct_idxs	= &keys[ i * keysize ];	//idxs without any error
		interped_idxs	= interpIntVec3( (F32)i/((F32)run_len), &keys[0], &keys[run_len * keysize ] ); //idxs lerping generates  
		subVec3( correct_idxs, interped_idxs, correction_needed );  //works for ints (cheesy, yes)
		subVec3( correction_needed, curr_run_error, correction_delta_needed ); 
		
		//calc new curr_run_error for next frame
		addVec3(curr_run_error, correction_delta_needed, curr_run_error );

		//SUPERTEMP 
		//acceptable_error = 15;

		//test if this correction delta is within range //debug only at this point
		if( ABS(correction_delta_needed[0]) > acceptable_error ||
			ABS(correction_delta_needed[1]) > acceptable_error ||
			ABS(correction_delta_needed[2]) > acceptable_error )
		{
			// key SUPERTEMP to check shit
			//correction_delta_needed[0] = acceptable_error;
			//correction_delta_needed[1] = acceptable_error;
			//correction_delta_needed[2] = acceptable_error;

			assert(0); //SUPERTEMP taking this out 
		}//*/

		//SUPERTEMP 
		assert( curr_run_error[0] == correction_needed[0] && curr_run_error[1] == correction_needed[1] && curr_run_error[2] == correction_needed[2]);
		//copy this correction delta to the correction run 
		
		copyVec3( correction_delta_needed, (&corrections[ i * keysize ]) );
	} 	

	return run_len;
}

/*unpacks an array of 5 byte rots into an array of four ints: three rots and a missing idx.*/
void unpack5ByteRotKeys( U8 * packed, U32 * unpacked, U32 * unpackedmissing, int count )
{
	int i;
	for(i = 0 ; i < count ; i++ )
	{
		unPack5ByteQuat( &packed[i*5], &unpacked[i*3], &unpackedmissing[i] );	
	}
}

void buildPositionCorrectionArray( AnimKeys * keys, int type )
{
	F32 * fullkeys;
	F32 * corrections;	
	int * corrections_quantized_to_fivebit;
	U8 * fullkeyidxs;
	int run_len;
	int i;
	int keysize;	
	int fullkeycount = 0; //number of full keys needed by this run
 
	keysize = 3; 
	fullkeys		= keys->vals;
	fullkeyidxs		= calloc( sizeof(U8), keys->count );			//1=this frame has a full key associated with it.  
	corrections		= calloc( sizeof(F32) * keysize, keys->count );	//Delta to last correction from lerp
	
	for( i = 0 ; i < keys->count - 1 ; i += run_len ) //
	{
		fullkeyidxs[i] = 1;
		fullkeycount++;
		run_len = calcPositionRun(
					&corrections[ i * keysize ],
					&fullkeys[i * keysize],
					keysize,  
					ACCEPTABLEPOSERROR,
					(keys->count - i) - 1
					);

		//########  Debug info #######
		total_saved_keys[type]+=run_len-1;
		runhist[POSITION_KEY][run_len]++;
		maxrun[POSITION_KEY] = MAX(run_len, maxrun[POSITION_KEY]);
		total_runs[POSITION_KEY]++;
		if(run_len == 1)
			total_useless_runs[POSITION_KEY]++; 
	}

	fullkeyidxs[ keys->count-1 ] = 1; //last key is always full key
	fullkeycount++;

	//convert the crrections to ints for packing up
	corrections_quantized_to_fivebit = calloc(sizeof(U32) * keysize, keys->count );
	quantizeCorrectionsToInts( corrections, corrections_quantized_to_fivebit, keys->count * keysize );

	keys->corrections = calloc( sizeof(U16), keys->count );
	keys->fullkeycount = fullkeycount;

	compressCorrectionArray( 
		keys->corrections, 
		corrections_quantized_to_fivebit, 
		fullkeyidxs, 
		keysize, 
		keys->count );

	free(corrections);
	free(fullkeyidxs);

	total_keys[type]+=keys->count;
}


/* Build the array of correction deltas to the slerp. 
Build the array of frames that need full keys
Compress these down to one or two bytes ( 1 2 2 2 or 1 5 5 5 )*/
void buildRotationCorrectionArray( AnimKeys * keys, int type )
{
	U32 * unpackedkeys;		
	U32 * unpackedmissing;  
	int * corrections;		
	U8 * fullkeyidxs;
	int run_len;
	int i;
	int keysize;
	int fullkeycount = 0;  //number of full keys needed by this run

	keysize = 3;  //ROTATIONKEYS

	unpackedkeys	= calloc( sizeof(U32) * keysize, keys->count);	//unpacked but not yet uncompressed keys (the 3 indexes)
	unpackedmissing = calloc( sizeof(U32), keys->count );			//unpacked missing bits
	fullkeyidxs		= calloc( sizeof(U8), keys->count );			//1=this frame has a full key associated with it.  
	corrections		= calloc( sizeof(int) * keysize, keys->count );	//Delta to last correction from slerp
	
	unpack5ByteRotKeys( keys->vals, unpackedkeys, unpackedmissing, keys->count );

	for( i = 0 ; i < keys->count - 1 ; i += run_len ) //
	{
		fullkeyidxs[i] = 1; //first key in a run is always a fullkey
		fullkeycount++;
		run_len = calcRun( 
						&corrections[ i * keysize ],	//run to write corrections to  
						&unpackedkeys[ i * keysize ],	//run of compression values
						&unpackedmissing[ i ],			//run of missing idx
						keysize, 
						ACCEPTABLEROTERROR, 
						(keys->count - i) - 1
						);

		//########  Debug info #######
		total_saved_keys[type]+=run_len-1;
		runhist[ROTATION_KEY][run_len]++;
		maxrun[ROTATION_KEY] = MAX(run_len, maxrun[ROTATION_KEY]);
		total_runs[ROTATION_KEY]++;
		if(run_len == 1)
			total_useless_runs[ROTATION_KEY]++; 
	}

	fullkeyidxs[ keys->count-1 ] = 1; //last key is always full key
	fullkeycount++;

	keys->corrections = calloc( sizeof(U16), keys->count );

	keys->fullkeycount = fullkeycount;

	compressCorrectionArray( keys->corrections, corrections, fullkeyidxs, keysize, keys->count ); //TO DO implement

	free(corrections);
	free(unpackedkeys);
	free(unpackedmissing);
	free(fullkeyidxs);

	total_keys[type]+=keys->count;
}


void processAnimRotKeys(AnimKeys * keys, Quat static_rotation)
{
	int i;
	F32 * spot;
	Quat quat; //mm anim
	F32 * vals;

	//convert keys->times from absolute times to 1/30th of a frame increments
	keysToFrames(keys);

	//Add keys that are the default if this node has no animation, just a basic rotation
	//Currently untested, cuz no anims have this prblem that I know of
	if(!keys->count)
	{
		keys->vals = malloc( sizeof(F32) * 4 );
		vals = keys->vals;
		copyVec4( static_rotation, ((F32*)&(vals[0])) );

		keys->times = malloc( sizeof(F32) );
		keys->times[0] = 0;

		keys->count = 1;

		total_dummy_rotation_keys+= 1; //Debug
	}

	//Convert axisAngle keys->vals to nice, normalized, biggest val positive quaternions
	//now any rotation only has one corresponding quaternion representation
	spot = (F32*)keys->vals;
	for(i=0;i<keys->count;i++)
	{
		axisAngleToQuat( &spot[i * 4], spot[i * 4 + 3], quat);
		memcpy(&spot[i * 4], &quat, sizeof(quat));
		quatNormalize(&spot[i*4]);
		makeBiggestValuePositive( &(spot[ i*4 ]) );

		assert( !(!(&spot[i * 4])[0]&& !(&spot[i * 4])[1] && !(&spot[i * 4])[2] && !(&spot[i * 4])[3] ) );
		assert( finiteVec4( &(spot[i*4]) ) );
	}
	
	//Throw away key->times by adding in dummy keys whenever the key times have gaps.  Now vals[frame] just works without time indirection
	normalizeKeyTimes( keys, 4 );

	//Conpress rotations to 5 bytes each
	keys->flags |= ROTATION_COMPRESSED_TO_5_BYTES; 
	compress5ByteRotationKeys( keys );

	// This is not ready for primetime
	/*
	keys->flags |= ROTATION_COMPRESSED_NONLINEAR;
	compress5ByteRotationKeysNonLinear( keys );
	*/

	keys->fullkeycount = keys->count;
	//At this point these are the valid values:
	//  keys->vals
	//	keys->count  / keys->fullkeycount
	//  keys->flags

	///*CORRECTION FIX 
	/*
	buildRotationCorrectionArray( keys, ROTATION_KEY );

	keys->vals = getRidOfUneededFullKeys( keys->vals, keys->count, keys->corrections, keys->fullkeycount, SIZE_OF_ROTKEY_COMPRESSED_TO_5_BYTES );

	//Debug
	{
		int i;
		for(i = 0 ; i < keys->count ; i++ )
		{
			U16 is_idx, idx;
			is_idx = (0x8000 & keys->corrections[i]);
			idx = (0x7fff & keys->corrections[i]);
			if( is_idx )
			{
				assert( idx < keys->fullkeycount );
			}
		}
	}

	//Important: Anything that happens after this point, keys->vals has been reduced and must use 
	//full key count instead of count, which only refers to the number of corrections.
	//My instinct tells me this is going to confuse someone sometime, think of a way to make it more clear...
	*/
}


// ### Position Keys ############

S16 quantizePositionValue( F32 value )
{
  S16 value16;
  assert( FINITE( value ) );
  assert( fabs( value*CFACTOR_6BYTE_POS ) <= 32000.0f );
  value16 = (S16)( value * CFACTOR_6BYTE_POS );
  return value16;
}

static void compress6BytePositionKeys( AnimKeys * keys, F32 compressionfactor )
{
	F32 * oldvals;
	S16 * newvals;
	int i;
	int vals_per_key = 3;
	
	oldvals = (F32 *)keys->vals;
	
	newvals = malloc( keys->count * sizeof( S16 ) * vals_per_key );
	
	for( i = 0 ; i < keys->count * vals_per_key ; i++ )
	{
		assert( FINITE( oldvals[i] ) );
		assert( oldvals[i] * compressionfactor <= 32000 && ( oldvals[i] * -1 * compressionfactor ) >= -32000 );
		newvals[i] = (S16)( oldvals[i] * compressionfactor );
	}
	free(keys->vals);
	keys->vals = newvals;
				
	return;
}


void processAnimPosKeys(AnimKeys * keys, F32 * static_position)
{
	F32 * vals;
	int keysHaveValuesOverOne;

	//convert keys->times from absolute times to 1/30th of a frame increments
	keysToFrames(keys);

	//Add keys that are the default if this node has no animation, just a basic position
	//currently untested cuz no anims have this problem
	if(!keys->count)
	{
		keys->vals = malloc( sizeof(F32) * 3 ); //two Vec3 keys
		vals = keys->vals;

		copyVec3( static_position, ((F32*)&(vals[0])) );

		keys->times = malloc( sizeof(F32) );
		keys->times[0] = 0; 

		keys->count = 1;

		total_dummy_position_keys+= 1; //debug
	}

	//Throw away key->times by adding in dummy keys whenever the key times have gaps.  Now vals[frame] just works without time indirection
	normalizeKeyTimes( keys, 3 );

	//Only compress the position keys if no key is more than 1.0
	{
		int i;
		F32 * vals;

		keysHaveValuesOverOne = FALSE;
		vals = (F32 *)keys->vals;
		for( i = 0 ; i < keys->count * 3 ; i++ )
		{
			assert(vals[i] < POS_BIGGEST);
			if( vals[i] >= 1.0 || vals[i] <= -1.0 )
			{
				keysHaveValuesOverOne = TRUE;
				break;
			}
		}
	}

	/*CORRECTION FIX */
	//doing just this 
	if( TRUE == keysHaveValuesOverOne )
	{
		keys->flags |= POSITION_UNCOMPRESSED;
		total_uncompressed_position_keys++; //debug
	}
	else
	{
		keys->flags |= POSITION_COMPRESSED_TO_6_BYTES;
		compress6BytePositionKeys(keys, CFACTOR_6BYTE_POS );
	}
	keys->fullkeycount = keys->count;
	//Now the valid values are:
	//  keys->vals
	//	keys->count / keys->fullkeycount
	//  keys->flags


	//instead of all this
	/*
	buildPositionCorrectionArray( keys, POSITION_KEY );

	if( keys->flags & POSITION_UNCOMPRESSED )
	{
		total_uncompressed_position_keys++;
		printf("..no compression on this key..");
		keys->vals = getRidOfUneededFullKeys( keys->vals, keys->count, keys->corrections, keys->fullkeycount, SIZE_OF_UNCOMPRESSED_POSKEY );
	}
	else
	{
		keys->flags |= POSITION_COMPRESSED_TO_6_BYTES;
		compress6BytePositionKeys(keys, CFACTOR_6BYTE_POS );
		keys->vals = getRidOfUneededFullKeys( keys->vals, keys->count, keys->corrections, keys->fullkeycount, SIZE_OF_POSKEY_COMPRESSED_TO_6_BYTES );
	}

	//Debug	
	{ 
		int i;
		for(i = 0 ; i < keys->count ; i++ )
		{
			U16 is_idx, idx;
			is_idx = (0x8000 & keys->corrections[i]);
			idx = (0x7fff & keys->corrections[i]);
			if( is_idx )
			{
				assert( idx < keys->fullkeycount );
			}
		}
	}

	//Important: Anything that happens after this point, keys->vals has been reduced and must use 
	//full key count instead of count, which only refers to the number of corrections.
	//My instinct tells me this is going to confuse someone sometime, think of a way to make it more clear...
	*/
}


//#############################################################################
//##################  Packing and Unpacking Stuff ############################



void initLookUpTable( F32 * table )
{
	F32 max_value;
	int i;

	max_value = (1.0/sqrtf(2.0));

	for( i = 0 ; i < LOOKUPSIZE12BIT ; i++)
	{
		table[i] = ( 2.0 * max_value * ( (F32)i / (F32)LOOKUPSIZE12BIT ) ) - max_value ;
	}
}


void unPack5ByteQuat( U8 * cquat, U32 * qidxs, int * missing )
{
	U8 top;
	U32 bytes;

	assert( cquat[0] < 64 );

	//decompress idxs (mirrors code in compressQuatToFiveBytes)
	//Unpack low bytes
	bytes		= *((U32*)&cquat[1]);
	qidxs[2]    = (0x00000fff & bytes );
	qidxs[1]	= (0x00fff000 & bytes ) >> 12;
	qidxs[0]	= (0xff000000 & bytes ) >> 24;

	//Unpack high byte
	top			= *((U8*)&cquat[0]);
	*missing	= (0xf0 & top) >> 4;
	qidxs[0]	|= (0x0f & top) << 8;

	assert(*missing >= 0 && *missing <=3 ); 
}


void animExpand8ByteQuat( S16 * compressed, Quat expanded ) 
{ 
	expanded[0] = EFACTOR_8BYTE_QUAT * (F32)compressed[0];
	expanded[1] = EFACTOR_8BYTE_QUAT * (F32)compressed[1];
	expanded[2] = EFACTOR_8BYTE_QUAT * (F32)compressed[2];
	expanded[3] = EFACTOR_8BYTE_QUAT * (F32)compressed[3];
}


	
void checkKeysBeforeExport( AnimKeys * poskeys, AnimKeys * rotkeys )
{
	if(poskeys->count != rotkeys->count) 
	{
		non_matching_counts++;
		//printf("Strange Poskeys different from Rotkeys count\n");
	}

	assert(poskeys->count != 0 && rotkeys->count != 0);

	if( rotkeys->flags & ROTATION_UNCOMPRESSED ) //DEBUG
	{
		int k;
		for( k = 0 ; k < rotkeys->fullkeycount ; k++ )
		{
			F32 * quat;
			quat = (F32*)&(((F32*)rotkeys->vals)[k * 4]);
			assert( !(!quat[0] && !quat[1] && !quat[2] && !quat[3] ) );
		}
	}
	if( poskeys->flags & POSITION_UNCOMPRESSED )
	{
		int k;
		for( k = 0 ; k < poskeys->fullkeycount ; k++ )
		{
			F32 * pos;
			pos = (F32*)&(((F32*)poskeys->vals)[k*3]);
			assert( pos[0] < POS_BIGGEST && pos[1] < POS_BIGGEST && pos[2] < POS_BIGGEST );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
//////////// General Animation stuff : where to put?

//TO DO: do this faster?
const BoneAnimTrack * animFindTrackInSkeleton( const SkeletonAnimTrack * skeleton, int id )
{
	int i;
	const BoneAnimTrack * bt = 0;

	for( i = 0 ; i < skeleton->bone_track_count ; i++ )
	{
		if( id == skeleton->bone_tracks[i].id )
		{
			bt = &skeleton->bone_tracks[i];
			break;
		}
	}

	return bt;
}

// Bind the boneAnimTracks for the texture animations
int setStAnim( StAnim *stAnim )
{
#if CLIENT || SERVER

	SkeletonAnimTrack * skeleton = 0 ;
	char buf[256];
	int loadType = LOAD_ALL;

	if ( !stAnim )
	{
		return 0 ;
	}

	//sprintf( buf, "tscroll/%s", stAnim->name );
	STR_COMBINE_BEGIN(buf);
	STR_COMBINE_CAT("tscroll/");
	STR_COMBINE_CAT(stAnim->name);
	STR_COMBINE_END();

#ifdef SERVER 
	loadType = LOAD_ALL_SHARED;
#endif
	skeleton = animGetAnimTrack( buf, loadType, NULL );

	if( !skeleton )
	{
		Errorf("Can't find anim: player_library/animations/tscroll/%s.anim", stAnim->name);
		return 0 ;
	}
	stAnim->btTex0 = animFindTrackInSkeleton( skeleton, 0 ); //bone zero( "hips" or "tex1" ) (hack in getAnimation supports "tex1" name)
	stAnim->btTex1 = animFindTrackInSkeleton( skeleton, 1 ); //bone one ( "waist" or "tex2" )  
	
	if( !stAnim->btTex0 && !stAnim->btTex1 )
	{
		Errorf( "Texture anim %s has no animation on bone 0 (texture 0) bone 1 (texture 1)", stAnim->name );
		return 0;
	}

	return 1;
#else
	return 0;

#endif
}
