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

/////########################## Bone Stuff ######################################
static void assignBoneNums(Node * root)
{
	//Rule: Bones should not have a lower id than their
	//parents.  This will, I think, make chopping stuff away for
	//LODs easier, cuz if the bones are in order, they can be removed
	//from the object in their order in the bone_ID array.  

	//Reorder to Ring, F2, F1, T3, T2, T1 and it will fit...

	//all GEO_ stuff has the ID GEO_ID (-2)
	//everything else gets the ID NOT_ANIM (-4)
	//Geo_Chest

	Node * node;

	if (!root)
		return;

	for (node = root; node; node = node->next)
	{
		node->anim_id = bone_IdFromName(node->name); // exact names only

		//Special Case for Scrolling Texture animations
		if (!stricmp(node->name, "TEX_1"))
			node->anim_id = 0;
		if (!stricmp(node->name, "TEX_2"))
			node->anim_id = 1;
		//End Special Case

		assignBoneNums(node->child);
	}
}
////################# end handle bones ######################################################




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


static void checkForGoodBones( Node * root )
{
	Node * node;
	
	for( node = root ; node ; node = node->next )
	{
		if(!bone_IdIsValid(node->anim_id)) 
		{
			printf("   #################################################################\n" );
			printf("   !!!! WARNING! a good bone, %s , is a child of a bad deleted bone!! \n", node->name);
			printf("   ( Maybe this should just crash ) \n", node->name);
		}
	}
}


static Node * ditchNonAnimStuff(Node * root )
{
	Node * node;
	Node * next;

	for( node = root ; node ; node = next )
	{
		next = node->next;
		if (node->child)
			ditchNonAnimStuff(node->child);

		if( !stricmp( node->name, "TEX_1" ) )
			continue;
		if( !stricmp( node->name, "TEX_2" ) )
			continue;
		if( bone_IdIsValid(node->anim_id) )
			continue;

		printf("      %s isn't a known bonename. Deleting\n", node->name);
		checkForGoodBones( node->child );
		treeDelete(node);
	}
	return getTreeRoot(); //(kindof a cheat)
}

//############################################################################################

static void setNodeMat(Node *node)
{
Quat	quat;
Mat4	rot_m,scale_m;

	// rotation
	axisAngleToQuat(&node->rotate[0],node->rotate[3],quat);
	quatToMat(quat,rot_m);
	zeroVec3(rot_m[3]);
	assert( !quatIsZero(quat) );

	// scale
	axisAngleToQuat(&node->scaleOrient[0],node->scaleOrient[3],quat);
	quatToMat(quat,scale_m);
	copyMat3(unitmat,scale_m); // ignore scaleOrientation for now
	zeroVec3(scale_m[3]);
	scaleMat3Vec3(scale_m,node->scale);

	// merge rotate + scale
	mulMat4(rot_m,scale_m,node->mat);

	// translation
	copyVec3(node->translate,node->mat[3]);

	//copyMat4(unitmat, node->mat);
}


static void setNodeMats( Node * root)
{
	Node * node;

	for( node = root ; node ; node = node->next)
	{	
		setNodeMat(node);
		if(node->child)
			setNodeMats(node->child);
	}
}


void getSkeleton( Node * root, SkeletonHeirarchy * skeletonHeirarchy )
{
	Node * node;

	for( node = root ; node ; node = node->next)
	{	
		assert( bone_IdIsValid( node->anim_id ) );
		skeletonHeirarchy->skeleton_heirarchy[ node->anim_id ].id = node->anim_id;

		if(node->next)	
		{
			assert( bone_IdIsValid( node->next->anim_id ) );
			skeletonHeirarchy->skeleton_heirarchy[ node->anim_id ].next = node->next->anim_id;
		}
		else
		{
			skeletonHeirarchy->skeleton_heirarchy[ node->anim_id ].next = -1;
		}

		if( node->child )
		{
			assert( bone_IdIsValid( node->child->anim_id ) );
			skeletonHeirarchy->skeleton_heirarchy[ node->anim_id ].child = node->child->anim_id;
			getSkeleton( node->child, skeletonHeirarchy );
		}
		else
		{
			skeletonHeirarchy->skeleton_heirarchy[ node->anim_id ].child = -1;
		}
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


