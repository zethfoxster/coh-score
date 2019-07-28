/******************************************************************************
 Utilities used by animation processing
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stdtypes.h"
#include "mathutil.h"
#include "Quat.h"
#include "tree.h"
#include "processanim.h"
#include "outputanim.h"
#include "assert.h"
#include "file.h"
#include "error.h"
#include "utils.h"

/******************************************************************************
  Convert source data using the 3DS MAX coordinate frame (right handed Z up)
  into the in-game coordinate frame. This can be viewed as two xforms:
    - into the standard VRML Y-up right handed conversion: Z->Y and -Y->Z
    - into the in game left handed coord system with Y-up: X->-X
  Source and destination can be the same
*****************************************************************************/
void ConvertCoordsFrom3DSMAX( Vec3 vOut, const Vec3 vIn )
{
  F32 tmpV1 = vIn[1]; // use a temp so src and dst can be the same vector
  vOut[0] = -vIn[0];
  vOut[1] =  vIn[2];
  vOut[2] = -tmpV1;
}

/******************************************************************************
	Convert from in-game coord system back to 3DS MAX for export of debug info
	or conversion from old assets to new .SKELX, .ANIMX files (see comments in
	ConvertCoordsFrom3DSMAX().
*****************************************************************************/
void ConvertCoordsGameTo3DSMAX( Vec3 vOut, const Vec3 vIn )
{
	F32 tmpV1 = vIn[1]; // use a temp so src and dst can be the same vector
	vOut[0] = vIn[0] != 0.0f ? -vIn[0] : 0.0f; // -0 is ugly in text exports
	vOut[1] = vIn[2] != 0.0f ? -vIn[2] : 0.0f;
	vOut[2] =  tmpV1 != 0.0f ?   tmpV1 : 0.0f;;
}

/******************************************************************************
  Convert from VRML coords Y-up right handed to the 3DS MAX coordinate 
	frame (right handed Z up).
	Source and destination can be the same
*****************************************************************************/
void ConvertCoordsVRMLTo3DSMAX( Vec3 vOut, const Vec3 vIn )
{
	F32 tmpV1 = vIn[1]; // use a temp so src and dst can be the same vector
	vOut[0] = vIn[0];
	vOut[1] = vIn[2] != 0.0f ? -vIn[2] : 0.0f;	// -0 is ugly in text exports
	vOut[2] =  tmpV1 != 0.0f ?   tmpV1 : 0.0f;;
}



/////########################## Bone Stuff ######################################
void assignBoneNums(Node * root)
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


Node * ditchNonAnimStuff(Node * root )
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


void setNodeMats( Node * root)
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

