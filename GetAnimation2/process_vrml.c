/******************************************************************************
This code encapsulates the LEGACY processing of VRML .WRL animation exports
previously used for CoH animation intermediate assets.

This code is used for conversions from those older .WRL assets to the newer
.SKELX and .ANIMX animation formats to jump start the transition to the new
animation pipeline (and in some circumstances the original MAX files are
not available (or readily available) to generate new exports from.
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stdtypes.h"
#include "mathutil.h"
#include "Quat.h"
//#include "grid.h"
#include "tree.h"
#include "processanim.h"
#include "vrml.h"
#include "outputanim.h"
#include "assert.h"
#include "file.h"
#include "error.h"
#include "utils.h"

void orientPos(Vec3 v)
{
	v[0] = -v[0];
}

void orientAngle(Vec4 v)
{
	v[0] = -v[0];
}


/*Visits each node and converts animation on that node to game useable data.*/
static void processAnimKeys(Node * root)
{
	Node * node;
	Quat quat;
	int i;
	F32 * spot; //mm anim

	for( node = root ; node ; node = node->next )
	{
		// ###### Do Position Keys ############
		//Done before add keys because added keys are already oriented
		spot = (F32*)node->poskeys.vals;
		for(i=0;i<node->poskeys.count;i++)
			orientPos(&spot[i*3]);

		processAnimPosKeys( &node->poskeys, node->translate );

		// ###### Do Rotation Keys ############
		axisAngleToQuat(&node->rotate[0],node->rotate[3],quat);
		assert( !quatIsZero(quat));

		//Done before adding defaults cuz they are already oriented
		spot = (F32*)node->rotkeys.vals;
		for(i=0;i<node->rotkeys.count;i++)
			orientAngle(&spot[i*4]);

		processAnimRotKeys(&node->rotkeys, quat );

		// ###### Do children #########
		if(node->child)
			processAnimKeys(node->child);
	}
}


//Base skeleton is the skeleton from which the heirarchy is gotten, and the skeleton 
//you use to find nodes you need, but the move you are on doesn't have that node
//Thus we can throw away nonanimated ones on the nonbase skeleton identical to those on the base skeleton
/// I just assume that if you dont have ant keys, you aren't anywhere different than the base
//*
void cullNodesExactlyTheSameAsTheBaseNodes(  Node * root )
{
	Node * node;

	for( node = root ; node ; node = node->next)
	{	
		if(!node->poskeys.count && !node->rotkeys.count)
		{
			//node->has_no_animation = 1;
		}
		if(node->child)
			cullNodesExactlyTheSameAsTheBaseNodes(node->child);
	}
}//*/

/*read one VRML file animation into a node tree and massage it into shape for the game.*/
Node * animConvertVrmlFileToAnimTrack( char * sourcepath, SkeletonHeirarchy * skeletonHeirarchy ) 
{
	Node	*root;

	//Read Vrml file into tree of root
	root = readVrmlFile(sourcepath);
	if(!root)
		FatalErrorf("Can't open %s for reading!\n",sourcepath);

	setNodeMats(root); //don't know if I need this

	assignBoneNums(root);

	root = ditchNonAnimStuff(root);

	//ditch bones with no anim track altogether, and read value from the base. 
	if( skeletonHeirarchy )
	{
		memset( skeletonHeirarchy, 0, sizeof( SkeletonHeirarchy ) ); //BUTODO is 1 the right number?
		skeletonHeirarchy->heirarchy_root = root->anim_id;
		getSkeleton( root, skeletonHeirarchy );	
	}
	else
	{
		cullNodesExactlyTheSameAsTheBaseNodes( root );  //must call before process anim keys
	}

	processAnimKeys(root);  

	return root;

}
#if 0

{
	Node * root;
	char baseSkeletonName[256];
	char animName[256];
	SkeletonAnimTrack * skeleton;
	SkeletonHeirarchy * skeletonHeirarchy = 0;

	printf( "Processing %s to %s...", sourcepath, targetpath ); 

	skeleton = calloc( 1, sizeof( SkeletonAnimTrack ) );

	outputResetVars(); 

	//Generate name of this anim "male/ready2" for example would be this name of .anim the file
	//data/player_library/animations/male/ready2.anim 
	//that came from src/player_library/animations/male/models/ready2.wrl

	convertSourceFilePathToAnimName( sourcepath, animName );
	strcpy( skeleton->name, animName );

	convertSourceFilePathToAnimName( baseSkeletonPath, baseSkeletonName );
	strcpy( skeleton->baseAnimName, baseSkeletonName );

	if( isBaseSkeleton == IS_BASE_SKELETON )
	{
		skeletonHeirarchy = calloc( 1, sizeof( SkeletonHeirarchy ) ); 
		skeleton->skeletonHeirarchy = skeletonHeirarchy;
		root = animConvertVrmlFileToAnimTrack( sourcepath, skeleton->skeletonHeirarchy );
	}
	else //NOT_BASE_SKELETON
	{
		root = animConvertVrmlFileToAnimTrack( sourcepath, 0 );
	}

	outputPackSkeletonAnimTrack( skeleton, root ); //turn tree of nodes into BoneAnimTracks for this skeleton

	treeFree();

	//Don't try to save base skeleton every time, if the base skeleton has changed, it should be in the changed list and come thrrough as not a base skeleton
	if( writeResults ) 
		outputAnimTrackToAnimFile( skeleton, targetpath );  //write data blocks to the binary

	free( skeletonHeirarchy ); //cant do skeleton->skeletonHeirarchy cuz its ruined in output
	free( skeleton );
	printf( "...done\n");
}

// This function process the single skeleton VRML export (and its associated base animation).
// The node tree is returned for use in processing additional ANIME exports that reference this
// skeleton
static Node * processSkeletonVRML( char * sourcepath, char * animName, int writeResults )
{
	Node * root;
	SkeletonAnimTrack * skeleton;
	SkeletonHeirarchy * skeletonHeirarchy = 0;

	printf( "Processing SKELETON: \"%s\"\n", sourcepath ); 

	skeleton = calloc( 1, sizeof( SkeletonAnimTrack ) );

	outputResetVars(); 

	// for a skeleton VRML this same name goes in both header fields
	strcpy( skeleton->name, animName );
	strcpy( skeleton->baseAnimName, animName );

	skeletonHeirarchy = calloc( 1, sizeof( SkeletonHeirarchy ) ); 
	skeleton->skeletonHeirarchy = skeletonHeirarchy;
	root = animConvertVrmlFileToAnimTrack( sourcepath, skeleton->skeletonHeirarchy );

	outputPackSkeletonAnimTrack( skeleton, root ); //turn tree of nodes into BoneAnimTracks for this skeleton

	// Don't need to save base skeleton anim every time
	// Caller will tell us when we should do it
	if( writeResults )
	{
		char targetpath[1000];
		convertAnimNameToTargetFilePath( animName, targetpath );
		printf( "\nWriting SKELETON output animation: \"%s\"\n", targetpath ); 
		outputAnimTrackToAnimFile( skeleton, targetpath );  //write data blocks to the binary
	}

	free( skeletonHeirarchy ); // cant do skeleton->skeletonHeirarchy cuz its ruined in output
	free( skeleton );          // free the animtrack

	// but pass the skeleton node tree to the caller for use in subsequent processing
	return root;
}
#endif