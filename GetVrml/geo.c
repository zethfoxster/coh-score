#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mathutil.h"
#include "Quat.h"
#include "grid.h"
#include "tree.h"
#include "geo.h"
#include "vrml.h"
#include "output.h"
#include "tricks.h"
#include "texsort.h"
#include "assert.h"
#include "file.h"
#include "error.h"
#include "earray.h"
#include "NVMeshMenderWrapper.h"
#include "tricututils.h"
#include "utils.h"
#include "manifold.h"
#include "uvunwrap.h"
#include "textparser.h"
#include "AutoLOD.h"

extern int do_meshMend;

ModelLODInfo *getPlayerLibraryDefaultLODs(void)
{
	static ModelLODInfo lod_info = {0};
	AutoLOD *lod;

	if (eaSize(&lod_info.lods) > 0)
		return &lod_info;

	lod = allocAutoLOD();
	lod->max_error = 0;
	eaPush(&lod_info.lods, lod);

	lod = allocAutoLOD();
	lod->max_error = 65;
	eaPush(&lod_info.lods, lod);

	lod = allocAutoLOD();
	lod->max_error = 73;
	eaPush(&lod_info.lods, lod);

	return &lod_info;
}

void orientPos(Vec3 v)
{
	v[0] = -v[0];
}

void orientAngle(Vec4 v)
{
	v[0] = -v[0];
}

/////########################## Bone Stuff ######################################

static int isAnimNode(Node *node)
{
	if (node->rotkeys.count || node->poskeys.count)
		return 1;
	if (strncmp(node->name,"geo_",4)==0)
		return 1;
	if (strncmp(node->name,"GEO_",4)==0) //mm
		return 1;
	return 0;
}

#define GEO_ID -2
#define NOT_ANIM -4

static int getBoneID(char * name)
{
	BoneId bone = bone_IdFromText(name);
	if(bone != BONEID_INVALID)
		return bone;

	if (!strncmp(name, "GEO_", 4))
		return GEO_ID;

	return NOT_ANIM;
}

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
		node->anim_id = getBoneID(node->name);
		assignBoneNums(node->child);
	}
}
////################# end handle bones ######################################################

///##########################Normal Trick ###################################################
/*This is a hack to handle the fact that 3ds Max doesn't allow you to individually tweak normals
*/
#define SAME 0

static int isNormalObject(Node * node)
{
	int len;

	len = strlen(node->name);

	if( stricmp(node->name+len-2, "_N") == SAME || stricmp(node->name+len-3, "_SN") == SAME )
		return 1;
	
	return 0;
}

/*go through tree and find nodes with the same name as the normals node (minus prefix), 
replace every normal in the matching node with the first normal in the normals node
*/
static void replaceAllNormals(Node * root, Node * normals)
{
	Node * node;
	int len, i;

	len = strlen(normals->name);
	len -= strlen("_SN");

	for( node = root ; node ; node = node->next)
	{	
		if( !isNormalObject(node) && _strnicmp(normals->name, node->name, len) == SAME )
		{
			for(i = 0 ; i < node->mesh.vert_count ; i++)
			{
				copyVec3(normals->mesh.normals[0], node->mesh.normals[i] );
			}
			printf("Replaced all %d normals from %s with the first normals from %s\n", 
				node->mesh.vert_count, node->name, normals->name );
		}
		replaceAllNormals(node->child, normals);
	}
}

/*used by replaceCloseNormals
*/
static int doCloseNormalReplacement(GMesh * s, GMesh * n)
{	
	int swapped = 0;
	int i, j, hit;
	for(i = 0 ; i < n->vert_count ; i++)
	{
		hit = 0;
		for(j = 0 ; j < s->vert_count ; j++)
		{
			if( nearSameVec3Tol(n->positions[i], s->positions[j], 0.01 ) )
			{
				hit++;
				copyVec3( n->normals[i], s->normals[j] );
				swapped++;
			}
		}
	}
	return swapped;
}

/*go through tree and find nodes with the same name as the normals node (minus prefix), 
in them, replace normals from the matching node with corresponding normals from the normals node.
(Corresponding means they have the same relative position.) 
*/
static void replaceCloseNormals(Node * root, Node * normals)
{
	Node * node;
	int len, swapped_cnt;

	len = strlen(normals->name);
	len -= strlen("_N");

	for( node = root ; node ; node = node->next)
	{	
		if( !isNormalObject(node) && _strnicmp(normals->name, node->name, len) == SAME )
		{
			swapped_cnt = doCloseNormalReplacement(&node->mesh, &normals->mesh);
			printf("Replaced %d of %d normals from %s with normals from %s\n", 
				swapped_cnt, node->mesh.vert_count, node->name, normals->name );
		}
		replaceCloseNormals(node->child, normals);
	}
}

/*Fills results with all nodes in the tree with this suffix.
*/
static int getNodesWithThisSuffix( Node * root, char * suffix, Node * results[] )
{
	int count = 0;
	char * c;
	Node * node;

	for( node = root ; node ; node = node->next)
	{
		c  = node->name + strlen(node->name) - strlen(suffix);
		if( c && _stricmp( c, suffix ) == SAME )
		{
			results[count++] = node;
		}
		getNodesWithThisSuffix( node->child, suffix, results );
	}
	return count;
}

/*See my thing in n:docs for how this works
*/
static void crazyNormalTrick(Node * root)
{
	Node * normals[1000];
	int count = 0, i;

	count = getNodesWithThisSuffix(root, "_N", normals );
	assert(count < 1000);

	for(i = 0 ; i < count ; i++)
	{
		replaceCloseNormals(root, normals[i]);
		treeDelete(normals[i]);
	}

	count = getNodesWithThisSuffix(root, "_SN", normals );
	assert(count < 1000);

	for(i = 0 ; i < count ; i++)
	{
		replaceAllNormals(root, normals[i]);
		treeDelete(normals[i]);
	}

	return;
}
///#######End crazy normals trick ##########################################

static void setNodeMat(Node *node)
{
Quat	quat;
Mat4	rot_m,scale_m;

	// rotation
	axisAngleToQuat(&node->rotate[0],node->rotate[3],quat);
	quatToMat(quat,rot_m);
	zeroVec3(rot_m[3]);

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

//##########Crazy Alternate Pivots thingy ################################

static void assignAltPivot(Node * node)
{
	char * c;
	int i;

	if(node->parent)
	{
		c = strrchr(node->name, '_');
		assertmsg(c, "Altpivot name needs to have an underscore before the number!");
		c++;
		i = atoi(c);
		assertmsgf(i <= MAX_ALTPIVOTS && i > 0, "invalid altpivot value %d", i);
		i--;
	
		copyMat4(node->mat, node->parent->altMat[i]);
		node->parent->altMatUsed[i] = 1;
	}
}

static void getAltPivots(Node * root)
{
	
	Node * node;
	
	for( node = root ; node ; node = node->next)
	{	
		setNodeMat(node);
		if(!_strnicmp("AltPiv", node->name, 6)) 
		{
			assignAltPivot(node);	 
		}
		getAltPivots(node->child);
	}
}

static Node *cleanAltPivots(Node *root)
{
	Node * node;
	Node * next;
	int i;

	for( node = root ; node ; )
	{
		next = node->next;
		for(i = 0 ; i < MAX_ALTPIVOTS ; i++)
		{
			if(node->altMatUsed[i])
			{
				if (node->altMatCount != i) {
					Errorf("Geometry named %s has a misnamed or duplicate AltPivot", node->name);
				}
				//assert(node->altMatCount == i);
				node->altMatCount = i + 1;
			}
		}
		if(!_strnicmp("AltPiv", node->name, 6))
		{
			if (node == root)
				root = next;
			treeDelete(node);
		}
		else
		{
			node->child = cleanAltPivots(node->child);
		}
		node = next;
	}
	return root;
}

static Node *doAltPivots(Node *root)
{
	getAltPivots(root);
	return cleanAltPivots(root);
}
//#######End crazy Alternate Pivot Thingy ##################################


typedef struct _TriSet
{	
	int *bones;
	int *tri_idx;
} TriSet;

static TriSet ** trisets;
static int num_trisets=0;

static void optimizeBoneReset(void)
{
	num_trisets=0;
}

static TriSet *optimizeBoneNewTriSet(void)
{
	TriSet *ret;
	int r2;
	if (num_trisets < eaSize(&trisets)) {
		ret = trisets[num_trisets++];
	} else {
		num_trisets++;
		ret = calloc(sizeof(TriSet),1);
		r2 = eaPush(&trisets, ret);
		assert(num_trisets - 1 == r2);
	}
	eaiSetSize(&ret->bones, 0);
	eaiSetSize(&ret->tri_idx, 0);
	return ret;
}

static void destroyTriSet(TriSet *triset)
{
	eaiDestroy(&triset->bones);
	eaiDestroy(&triset->tri_idx);
}

static void optimizeBoneCleanup(void)
{
	eaClearEx(&trisets, NULL);
	eaDestroy(&trisets);
}

typedef struct BoneSection
{
	int bones[MAX_OBJBONES];
	int num_bones;
	int start_idx;
	int run_length;
} BoneSection;

#define MAX_BS_TRISETS 80
#define MAX_BONE_SECTIONS 24 //mm    // fpe moved from anim.h and bumped from 20 to 24 to accomodate new geo.  Used for book keeping here only.

static void optimizeBone(Node *node, char buf[], int *buf_offset, int tri_index_start, int tri_index_stop)
{
	GMesh *mesh = &node->mesh;
	BoneData *bones = &node->bones;
	int i, j, k ,l;
	int bones_in_tri[BONES_ON_DISK];
	int num_bones_in_tri;
	int match;
	int vert;
	int bone;
	int done;
	int a, b, temp;
	TriSet * triset;
	BoneSection * bs;
	int tri_offset;
	int bonesection_trisets[MAX_BONE_SECTIONS][MAX_BS_TRISETS]; //bookkeeping, don't x-fer
	int num_bts[MAX_BONE_SECTIONS]; //bookkeeping 
	GTriIdx *temp_tris;
	int final_tri_list[MAX_TRIIDX];

	int			num_bonesections;
	BoneSection	bonesections[MAX_BONE_SECTIONS] = {0};

	if (!bones->numbones)
		return;

	optimizeBoneReset();

	if (mesh->tri_count > MAX_TRIIDX)
		printf("This has more than %d tris, get a programmer to increase the constant MAX_TRIIDX", MAX_TRIIDX);
	assert(mesh->tri_count <= MAX_TRIIDX);

	//1. *****Build a Catalog (trisets) of all combinations of bones tris represent.

	//for each tri
	for (i = tri_index_start; i < tri_index_stop; i++)
	{
		//A. Get a list of all the bones in that tri
		num_bones_in_tri = 0;
		for (j = 0; j < 3; j++)
		{
			vert = mesh->tris[i].idx[j];
			//get a list of each bone that has any weight in that tri. 
			for (k = 0; k < 2; k++)
			{
				bone = mesh->bonemats[vert][k] / 3;
				if (mesh->boneweights[vert][k] != 0.0)
				{
					for (l = 0; l < num_bones_in_tri; l++)
					{
						if (bone == bones_in_tri[l])
							break;
					}
					if (l == num_bones_in_tri)
					{
						bones_in_tri[num_bones_in_tri] = bone;
						num_bones_in_tri++;
						assert(num_bones_in_tri < BONES_ON_DISK);
					}
				}
			}
		}
	
		//sort the bones_in_tri for ease
		for (;;)
		{
			done = 1;
			for (a = 0, b = 1; b < num_bones_in_tri; a++, b++)
			{
				if (bones_in_tri[a] > bones_in_tri[b])
				{
					temp = bones_in_tri[b];
					bones_in_tri[b] = bones_in_tri[a];
					bones_in_tri[a] = temp;
					done = 0;
				}
			}
			if (done == 1)
				break;
		}
			
		//tri_idx (i) bones_in_tri[] and num_bones in tri 
		//B. Find the TriSet that matches this triangle, or add a new triset.
		match = -1;
		for (j = 0; j < num_trisets && match == -1; j++)
		{
			//match = checkForMatch(trisets[j], bones_in_tri, num_bones)
			if (num_bones_in_tri == eaiSize(&trisets[j]->bones))
			{
				match = j;
				for (k = 0; k < num_bones_in_tri; k++)
				{
					if (bones_in_tri[k] != trisets[j]->bones[k])
						match = -1;
				}
			}
		}
		if (match == -1)
		{
			TriSet *triset = optimizeBoneNewTriSet();
			for (j = 0; j < num_bones_in_tri; j++)
			{
				eaiPush(&triset->bones, bones_in_tri[j]);
			}
			assert(eaiSize(&triset->tri_idx)==0);
			match = num_trisets-1;
		}
	
		//C. Add this tri to it's new triset
		eaiPush(&trisets[match]->tri_idx, i);
	}


	//sort the tri_sets for ease
	for (;;)
	{
		done = 1;
		for (a = 0, b = 1; b < num_trisets; a++, b++)
		{
			if (eaiSize(&trisets[a]->bones) > eaiSize(&trisets[b]->bones))
			{
				TriSet *temp_triset = trisets[b];
				trisets[b] = trisets[a];
				trisets[a] = temp_triset;
				done = 0;
			}
		}
		if (done == 1)
			break;
	}

	//Print the results
	*buf_offset += sprintf( buf + *buf_offset, "\n\n########%s" , node->name); 

	for (i = 0 ; i < num_trisets ; i++)
	{
		*buf_offset += sprintf(buf + *buf_offset, "\n\nSet %d:  \n" , i); 

		*buf_offset += sprintf(buf + *buf_offset, "NumBones: %d Bones: ", eaiSize(&trisets[i]->bones));
		for(j = 0 ; j < eaiSize(&trisets[i]->bones); j++)
			*buf_offset += sprintf(buf + *buf_offset, " %d ", trisets[i]->bones[j]);

		*buf_offset += sprintf(buf + *buf_offset, "NumTris: %d \n", eaiSize(&trisets[i]->tri_idx));
		for(j = 0 ; j < eaiSize(&trisets[i]->tri_idx); j++)
			*buf_offset += sprintf(buf + *buf_offset, "%d.", trisets[i]->tri_idx[j]);
	}	
   
	assert(*buf_offset < MAX_BUFFER - 10000);

//*********************************************************
	//2. Now that we have the whole object cataloged 
	//Divide it up in to two bone groups and a final, all the rest bone group.
	
	//A. Init stuff
	for (i = 0; i < ARRAY_SIZE(num_bts); i++)
		num_bts[i] = 0;
	num_bonesections = 0; 
	tri_offset = tri_index_start;

	//B. For each triset, identify which BoneSection it should be a part of
	//   and add it to the BoneSection bookkeeping list:
	//			int bonesection_trisets[MAX_BONE_SECTIONS][MAX_BS_TRISETS]; 	
	//			int num_bts[MAX_BONE_SECTIONS]; 
	//   This is where to later change the way the list built.
	//   For the future:  Combine orphaned one boners?  Add a minimum number of tris?

	for (i = num_trisets - 1; i >= 0; i--)
	{
		// make sure we haven't overflowed array bounds
		assert(num_bonesections < MAX_BONE_SECTIONS);
		assert(num_bts[num_bonesections] <= MAX_BS_TRISETS);

		//If 3 or more add to the first, software set
		if (eaiSize(&trisets[i]->bones) >= 3)
		{
			bonesection_trisets[0][num_bts[0]] = i;
			num_bts[0]++;
			if (!num_bonesections)
				num_bonesections = 1;
		}
		//If two, add to it's own hardware set
		else if (eaiSize(&trisets[i]->bones) == 2)
		{	
			bonesection_trisets[num_bonesections][num_bts[num_bonesections]] = i;
			num_bts[num_bonesections]++;
			num_bonesections++;
		}

		//If one, Find a twoer already in use, and add it to that: if that fails, make it it's own
		
		else if (eaiSize(&trisets[i]->bones) == 1)
		{
			match = -1;
			for (j = 0; j < num_bonesections ; j++)
			{
				for (l = 0; l < num_bts[j]; l++)
				{
					if (eaiSize(&trisets[bonesection_trisets[j][l]]->bones) == 2)
					{
						for (k = 0; k < 2; k++)
						{
							if (trisets[bonesection_trisets[j][l]]->bones[k] == trisets[i]->bones[0])
							{
								match = j;
								break;
							}
						}
					}
					if (match != -1)
						break;
				}
				if (match != -1)
					break;
			}
			if (match == -1)
			{
				match = num_bonesections;
				num_bonesections++;
			}

			bonesection_trisets[match][num_bts[match]] = i;
			num_bts[match]++;
		}
		else if (eaiSize(&trisets[i]->bones) == 0)
		{
			bonesection_trisets[num_bonesections][num_bts[num_bonesections]] = i;
			num_bts[num_bonesections]++;
			num_bonesections++;
		}
		else 
		{
			assert(0);
		}
	}
						
	//C.  Take the Resulting list of
	//		int bonesection_trisets[MAX_BONE_SECTIONS][MAX_BS_TRISETS]
	//		int num_bonesection_trisets; 
	//	  And use it to write to the shape's bonesection array and to the 
	//	  Array of tri indexes.

	for (i = 0; i < num_bonesections; i++)
	{
		bs = &(bonesections[i]);
		bs->start_idx  = tri_offset;
	
		assert(num_bts[i]);
		for (l = 0; l < num_bts[i]; l++)
		{
			triset = trisets[bonesection_trisets[i][l]];
			//For each bone in the triset
			//Compare it to each bone in the bonesection
			//If the bone isn't in the bonesection yet, add it
			//assert(triset->num_bones);
			for (j = 0; j < eaiSize(&triset->bones); j++)
			{	
				match = 0;
				for (k = 0; k < bs->num_bones; k++)
				{
					if (bs->bones[k] == triset->bones[j])
					{
						match = 1;
					}
				}
				if (!match)
				{
					bs->bones[bs->num_bones] = triset->bones[j];
					bs->num_bones++;
				}	
			}
			
			//Then update the tri_set and write this triset's tris to the new tri_idx	
			bs->run_length += eaiSize(&triset->tri_idx);
			
			for (j = 0; j < eaiSize(&triset->tri_idx); j++)
			{
				final_tri_list[tri_offset] = triset->tri_idx[j];
				tri_offset++;
			}
		}	
	}
	assert(tri_index_stop == tri_offset);

	//D. Reorder the tris in the order final_tri_list specifies
	temp_tris = calloc(sizeof(GTriIdx),mesh->tri_count);
	for (i = tri_index_start; i < tri_index_stop; i++)
		temp_tris[i] = mesh->tris[i];
	for (i = tri_index_start; i < tri_index_stop; i++)
		mesh->tris[i] = temp_tris[final_tri_list[i]];
	free(temp_tris);

	//E. Shape should now be all set: Print the results
	*buf_offset += sprintf(buf + *buf_offset, "\n\nResulting %s BoneSections: %d" , node->name, num_bonesections); 
	k = 0;
	for (i = 0; i < num_bonesections; i++)
	{
		k += bonesections[i].run_length;
		*buf_offset += sprintf(buf + *buf_offset, " \nBS %d: ", i); 
		*buf_offset += sprintf(buf + *buf_offset, " RunLength %d ", bonesections[i].run_length); 
		*buf_offset += sprintf(buf + *buf_offset, " StartIdx %d ", bonesections[i].start_idx); 
		*buf_offset += sprintf(buf + *buf_offset, " NumBones %d ", bonesections[i].num_bones); 
		for(j = 0 ; j < bonesections[i].num_bones ; j++)
			*buf_offset += sprintf(buf + *buf_offset, "b: %d" , bonesections[i].bones[j]); 
	}
	assert(k == tri_index_stop - tri_index_start);
	*buf_offset += sprintf(buf + *buf_offset, " \nTotalRun: %d \n" , k); 
	assert(*buf_offset < MAX_BUFFER - 10000);//*/

	return;
}

static void optimizeTree2(Node * node, char buf[], int * buf_offset)
{
	for(;node;node = node->next)
	{
		int tri_start_index=0;
		while (tri_start_index != node->mesh.tri_count)
		{
			int texid = node->mesh.tris[tri_start_index].tex_id;
			int tri_stop_index=0;
			for (tri_stop_index = tri_start_index; tri_stop_index < node->mesh.tri_count && node->mesh.tris[tri_stop_index].tex_id == texid; tri_stop_index++);
			//if(isAnimNode(node))
				optimizeBone(node, buf, buf_offset, tri_start_index, tri_stop_index);
			tri_start_index = tri_stop_index;
		} 
		if(node->child)
		{
			optimizeTree2(node->child, buf, buf_offset);
		}
	}
}
					
static void optimizeBones(Node * node)
{
	FILE	*	file;
	char buf2[100];
	char buf[MAX_BUFFER];
	int buf_offset;

	sprintf(buf2,"%s/chopper.txt",fileDataDir());
	file = 0;//fileOpen(buf2,"wt"); //don't do this, it's old, designed for the old vert extension

	buf_offset = 0;
	optimizeTree2(node, buf, &buf_offset);

	if (file)
	{
		fwrite( buf, sizeof( char ), buf_offset, file );
		fclose(file);
	}
	optimizeBoneCleanup();
}

static void reverseCharacter(Vec4 v) //mm
{
	Quat	quat;
	Mat3	rot_m;
	Mat3    y180;
	Mat3    result;

	copyMat3(unitmat, y180);
	yawMat3(RAD(180), y180);

	axisAngleToQuat(&v[0],v[3],quat);
	quatToMat(quat,rot_m);
	mulMat4(y180, rot_m, result);

	//now extract axis-angle rotation from result and set v[0-4] to that	
	return;
}


//####################### Build Shape Normals ##################################################
// Not used right now

static F32 calcWeight(Vec3 v0,Vec3 v1,Vec3 v2)
{
	Vec3	tvec1, tvec2, tvec3;

	subVec3(v1,v0,tvec1);
	normalVec3(tvec1);
	subVec3(v2,v0,tvec2);
	normalVec3(tvec2);
	crossVec3(tvec1,tvec2,tvec3);
	return normalVec3(tvec3) * 0.5f;
}

static void	calcFaceNormal(Vec3 v0,Vec3 v1,Vec3 v2,Vec3 norm)
{
	Vec3	tvec1,tvec2;

	subVec3(v1,v0,tvec1);
	subVec3(v2,v1,tvec2);
	crossVec3(tvec1,tvec2,norm);
	normalVec3(norm);
}

static void calcVertexNormal(GMesh *mesh,int tri_idx,int tvert_idx,F32 ang)
{
	int		i,j,idx;
	F32		weight=0, tweight;
	Vec3	tvec,norm,tri_norm,test_norm,dv;
	F32		dp,cosang,size;
	F32		*vert,*v0,*v1,*v2;
	PolyCell	*pcell;

	zeroVec3(norm);
	cosang = cosf(ang);
	vert = mesh->positions[mesh->tris[tri_idx].idx[tvert_idx]];
	calcFaceNormal(mesh->positions[mesh->tris[tri_idx].idx[0]],
					mesh->positions[mesh->tris[tri_idx].idx[1]],
					mesh->positions[mesh->tris[tri_idx].idx[2]],
					tri_norm);

	subVec3(vert,mesh->grid.pos,dv);
	pcell = polyGridFindCell(&mesh->grid,dv,&size);
	if (!pcell)
		return;
	for (idx = 0; idx < pcell->tri_count; idx++)
	{
		i = pcell->tri_idxs[idx];
		for (j=0; j<3; j++)
		{
			if (nearSameVec3(vert,mesh->positions[mesh->tris[i].idx[j]]))
			{
				v0 = mesh->positions[mesh->tris[i].idx[0]];
				v1 = mesh->positions[mesh->tris[i].idx[1]];
				v2 = mesh->positions[mesh->tris[i].idx[2]];
				calcFaceNormal(v0,v1,v2,test_norm);
				dp = dotVec3(tri_norm, test_norm);
				if (dp <= -1.0f)
					dp = -1.f;
				if (dp > 0.999f)
					dp = 1.f;
				if (dp >= cosang)
				{
					if (!weight)
					{
						copyVec3(test_norm,norm);
						weight = calcWeight(v0,v1,v2);
						continue;
					}
					tweight = calcWeight(v0,v1,v2);
					scaleVec3(norm, weight/(weight+tweight), norm);
					scaleVec3(test_norm, tweight/(weight+tweight), tvec);
					addVec3(norm,tvec,norm);
					weight += tweight;
				}
			}
		}
	}
	normalVec3(norm);
	copyVec3(norm,mesh->normals[mesh->tris[tri_idx].idx[tvert_idx]]);
}

static void buildShapeNormals(GMesh *mesh)
{
	int		i,j;
	GMesh	simple={0};

	gmeshCopy(&simple, mesh, 1);
	gmeshFreeData(mesh);

	gmeshSetUsageBits(&simple, simple.usagebits | USE_NORMALS);
	for (i = 0; i < mesh->tri_count; i++)
	{
		for(j=0;j<3;j++)
			calcVertexNormal(&simple,i,j,RAD(89.f));
	}

	gmeshCopy(mesh, &simple, 1);
	gmeshFreeData(&simple);
}

static Node * mergeNode(Node *node,GMesh *parent_mesh)
{
	int		i;
	Vec3	v;
	Node	*next;

	for(;node;node = next)
	{
		next = node->next;
		setNodeMat(node);

		if (node->child)
			mergeNode(node->child, &node->mesh);

		if (!isAnimNode(node) && !node->child)
		{
			zeroVec4(node->rotate);
			for (i = 0; i < 3; i++)
				node->scale[i] = 1.f;

			for (i = 0; i < node->mesh.vert_count; i++)
			{
				mulVecMat4(node->mesh.positions[i],node->mat,v);
				copyVec3(v,node->mesh.positions[i]);
			}
			if (!node->mesh.vert_count)
			{
				treeDelete(node);
			}
			else if (parent_mesh)
			{
				gmeshMerge(parent_mesh, &node->mesh, 0, 0);
				treeDelete(node);
			}
		}
	}
	return getTreeRoot(); //(kindof a cheat)
}

static F32 checkrad(Vec3 *verts,int count,Vec3 mid)
{
int		i;
F32		rad = 0;
Vec3	dv;

	for(i=0;i<count;i++)
	{
		subVec3(verts[i],mid,dv);
		if (lengthVec3(dv) > rad)
			rad = lengthVec3(dv);
	}
	return rad;
}

static void centerNode(Node *node,const Mat4 parent_mat)
{
	int		i,j;
	Vec3	mid,dv;
	Vec3	v,min = {10e10,10e10,10e10},max={-10e10,-10e10,-10e10};

	F32		rad;

	for (;node;node = node->next)
	{
		setVec3(min,10e10,10e10,10e10);
		setVec3(max,-10e10,-10e10,-10e10);
		for (i=0;i<node->mesh.vert_count;i++)
		{
			copyVec3(node->mesh.positions[i],v);
			for (j=0;j<3;j++)
			{
				if (v[j] < min[j])
					min[j] = v[j];
				if (v[j] > max[j])
					max[j] = v[j];
			}
		}
		if (!node->mesh.vert_count)
		{
			zeroVec3(min);
			zeroVec3(max);
		}
		copyVec3(min,node->min);
		copyVec3(max,node->max);
		subVec3(max,min,mid);
		scaleVec3(mid,0.5f,mid);
		addVec3(mid,min,mid);

		//This basically undoes any center changing
		copyVec3(node->pivot,node->translate);
		copyVec3(node->translate,mid);

		setNodeMat(node);
		
		for (rad = 0,i=0;i<node->mesh.vert_count;i++)
		{
			subVec3(node->mesh.positions[i],mid,dv);
			if (!isAnimNode(node))
				copyVec3(dv,node->mesh.positions[i]);
	
			if (lengthVec3(dv) > rad)
				rad = lengthVec3(dv);
		}
		node->radius = rad;
			
		if (node->child)
			centerNode(node->child,parent_mat);
	}
}

/*###### Vert Cache ##############
*/
static int	vcache[16];
static int	all_cache[1000000];
int			all_cache_count;

static int allCacheIdx(int idx)
{
int		i;

	for(i=0;i<all_cache_count;i++)
	{
		if (all_cache[i] == idx)
			return i;
	}
	return -1;
}

static void allCacheAdd(int idx)
{
	if (allCacheIdx(idx) >= 0)
		return;
	all_cache[all_cache_count++] = idx;
	assert(all_cache_count < ARRAY_SIZE(all_cache));
}

static int vertCacheIdx(int idx)
{
int		i;

	for(i=0;i<ARRAY_SIZE(vcache);i++)
	{
		if (vcache[i] == idx)
			return 0;
	}
	return -1;
}

static int print_verts,last_vidx,nonseq_count;

static int vertCacheAdd(int idx)
{

	if (vertCacheIdx(idx) >= 0)
		return 0;
	memmove(vcache,vcache+1,sizeof(vcache) - sizeof(vcache[0]));
	vcache[ARRAY_SIZE(vcache)-1] = idx;
	if (print_verts)
		printf("%d\n",idx);
	if (idx-1 != last_vidx)
		nonseq_count++;
	last_vidx = idx;
	return 1;
}

static int vertCacheGetTri(GMesh *mesh,int *tri_idxs,int *tris_used,int count)
{
int		i,j,idx=-1,in_count,seen_already,max_count=-1,seen_min = 4;

	for (i=0;i<count;i++)
	{
		if (tris_used[i])
			continue;
		for (seen_already = in_count = j = 0; j < 3; j++)
		{
			if (vertCacheIdx(mesh->tris[i].idx[j]) >= 0)
				in_count++;
			else if (allCacheIdx(mesh->tris[i].idx[j]) >= 0)
				seen_already++;
		}
		if (in_count > max_count || (in_count == max_count && seen_already < seen_min))
		{
			seen_min = seen_already;
			max_count = in_count;
			idx = i;
		}
	}
	tris_used[idx] = 1;
	return idx;
}

static void vertCacheTest(GMesh *mesh)
{
	int i,j,cache_miss=0;

	last_vidx = -2;
	nonseq_count = 0;
	memset(vcache,255,sizeof(vcache));
	for (i = 0; i < mesh->tri_count; i++)
	{
		for (j = 0; j < 3; j++)
			cache_miss += vertCacheAdd(mesh->tris[i].idx[j]);
	}
	printf("verts %d  vload %d  nonseq %d  tri*3 %d\n",mesh->vert_count,cache_miss,nonseq_count,mesh->tri_count*3);
}

static void vertCache(GMesh *mesh)
{
	int		i,j,idx;
	int		*tris,*tris_used;
	GTriIdx	*tri_idxs;

#if 0
	printf("\n");
	print_verts = 0;
	vertCacheTest(mesh);
	print_verts = 0;
#endif

	memset(vcache,255,sizeof(vcache));
	tris = malloc(mesh->tri_count * sizeof(int));
	tris_used = calloc(mesh->tri_count, sizeof(int));
	tri_idxs = calloc(mesh->tri_count, sizeof(GTriIdx));

	all_cache_count = 0;
	//This for loop takes a lot of time...
	for (i = 0; i < mesh->tri_count; i++)
	{
		idx = vertCacheGetTri(mesh,tris,tris_used,mesh->tri_count);
		tris[i] = idx;
		for (j = 0; j < 3; j++)
		{
			vertCacheAdd(mesh->tris[idx].idx[j]);
			allCacheAdd(mesh->tris[idx].idx[j]);
		}
	}
	for(i=0;i<mesh->tri_count;i++)
		tri_idxs[i] = mesh->tris[tris[i]];
	memcpy(mesh->tris,tri_idxs,mesh->tri_count * sizeof(GTriIdx));
	free(tris);
	free(tri_idxs);
	free(tris_used);

#if 0
	print_verts = 0;
	vertCacheTest(shape);
	print_verts = 0;
#endif
}

static void addgrid(Node *node)
{
	for(; node; node = node->next)
	{
		int usageBits = USE_NORMALS;
		if( strstri(node->name, "__prquad") != NULL )
			usageBits |= USE_NOMERGE; // dont merge verts

		gmeshSetUsageBits(&node->mesh, node->mesh.usagebits | usageBits);
		gmeshUpdateGrid(&node->mesh, 1);

		if (node->child)
			addgrid(node->child);
	}
}

static Node * ditchUnneededStuff(Node * root, int whattoditch )
{
	Node * node;
	Node * next;

	for(node = root;node;node = next)
	{
		next = node->next;
		if (node->child)
			ditchUnneededStuff(node->child, whattoditch);

		if (whattoditch == PROCESS_GEO_ONLY)
		{
			if(strnicmp(node->name, "GEO_", 4))
			{
				treeDelete(node);
			}
			else
			{
				if(node->rotkeys.count || node->poskeys.count)
					printf("");
				node->rotkeys.count = 0;
				node->poskeys.count = 0;
				node->mesh.grid.cell = 0;
			}
		}
		else if (whattoditch == PROCESS_ANIM_ONLY)
		{
			if(!strnicmp(node->name, "GEO_", 4))
				treeDelete(node);
			else
			{
				gmeshSetUsageBits(&node->mesh, node->mesh.usagebits & (~USE_POSITIONS));
				node->mesh.tri_count  = 0;
				//node->shape.grid.cell  = 0; //this causes problem for addgrid, so I remove it later
				
				node->altMatCount      = 0;
				node->bones.numbones   = 0;
			}
		}
		else if (whattoditch == PROCESS_SCALEBONES_ONLY)
		{
			if(!bone_IdIsValid(node->anim_id))
			{
				treeDelete(node);
			}
			else
			{
				gmeshSetUsageBits(&node->mesh, node->mesh.usagebits & (~USE_POSITIONS));
				node->mesh.tri_count  = 0;
				node->altMatCount      = 0;
				node->bones.numbones   = 0;
				node->rotkeys.count    = 0;
				node->poskeys.count    = 0;
			}
		}
		else 
		{
			// Always ditch _HIDDEN nodes (to prevent runtime errors of duplicate node names for
			//	character parts exported to object_library, eg FX subdir)
			#define HIDDEN_STRLEN  7
			int len = strlen(node->name);
			char* pEndStr = node->name + len - HIDDEN_STRLEN;
			if( len > HIDDEN_STRLEN && !strncmp(pEndStr, "_HIDDEN", HIDDEN_STRLEN) )
			{
				printf("  ditching hidden node \"%s\"\n", node->name);			
				treeDelete(node);
			}
		}
	}
	return getTreeRoot(); //(kindof a cheat)
}

static void reorderTrisByTex(Node * root)
{
	Node * node;
	for( node = root ; node ; node = node->next )
	{	
		reorderTriIdxsByTex(&node->mesh);
		if (node->child)
			reorderTrisByTex(node->child);
	}
}

/*unused: supposed to fix problem of grouped objects getting wrong, consolidated pivot...fix this
*/
static void assignPivots(Node * root)
{
	Node * node;
	for (node = root; node; node = node->next)
	{
		///copyVec3(node->pivot,node->translate);
		//copyVec3(node->translate,node->pivot);
		//copyVec3(node->pivot, node->translate); //mm wa
		//subVec3(node->translate, node->pivot, node->temp); //mm wa
		if (node->child)
			assignPivots(node->child);
	}
}


static void ditchGrid(Node * node)
{
	polyGridFree(&node->mesh.grid);
}

static void checkForBadBones(Node * root)
{
	Node * node;

	for (node = root; node; node = node->next)
	{
		if (node->anim_id < 0)
			printf("      %s isn't a known bonename\n", node->name);
		checkForBadBones(node->child);
	}		
}

static void meshMender(Node *node, char *name)
{
	MeshMenderMendGMesh(&node->mesh, name);
}

static void meshMenderRecurse(Node * node)
{
	// We are not using MeshMender because we determined it still creates
	// seems, and per-pixel calculations create much better results, and
	// our old, hacky method produces possibly better results than MeshMender
	// (assuming we use the hacky method of normalizing), *and* nvdxt produces
	// not quite right normals, so that on a texture mirror it would create
	// a seam *even if* our tangent/binormal generation was perfect.
	for(; node; node = node->next)
	{
		//if(isAnimNode(node))
		if( !strstri(node->name, "GEO_Cape") )
			meshMender(node, node->name);
		if(node->child)
			meshMenderRecurse(node->child);
	}
}



/*cut out path and .wrl to get name of this anim
//cut header name down to just the name without folder path or '.wrl'
should really be in some util-type file
*/
void parseAnimNameFromFileName(char * fname, char * animname)
{
	char basename[MAX_PATH];
	char * bs, * fs, * s;

	fs = strrchr(fname, '/'); 
	bs = strrchr(fname, '\\');
	s = fs > bs ? fs : bs;
	if(s)
		strcpy(basename, s + 1);
	else
		strcpy(basename, fname);
	s = strrchr(basename,'.'); 
	if (s)
		*s = 0;
	strcpy(animname, basename);
}

void nodeUvunwrap(Node *node)
{
	int i;
	Prim **primitives;
	GMesh temp_mesh={0};
	float size;
	Vec2 zerovec2={0};

	if (!node->mesh.tri_count)
		return;

	eaCreate(&primitives);

	for (i = 0; i < node->mesh.tri_count; i++)
	{
		Prim *prim;
		int i0, i1, i2;

		i0 = node->mesh.tris[i].idx[0];
		i1 = node->mesh.tris[i].idx[1];
		i2 = node->mesh.tris[i].idx[2];

		if (node->mesh.tex1s)
			prim = primCreate(node->mesh.positions[i0], node->mesh.positions[i1], node->mesh.positions[i2], node->mesh.tex1s[i0], node->mesh.tex1s[i1], node->mesh.tex1s[i2]);
		else
			prim = primCreate(node->mesh.positions[i0], node->mesh.positions[i1], node->mesh.positions[i2], zerovec2, zerovec2, zerovec2);

		eaPush(&primitives, prim);
	}

	size = uvunwrap(&primitives);

	gmeshCopy(&temp_mesh, &node->mesh, 0);
	gmeshFreeData(&node->mesh);

	gmeshSetUsageBits(&node->mesh, temp_mesh.usagebits | USE_TEX2S);
	for (i = 0; i < temp_mesh.tri_count; i++)
	{
		int idx0, idx1, idx2;
		Vec2 st3s[3];
		primGetTexCoords(primitives[i], st3s[0], st3s[1], st3s[2]);

		idx0 = temp_mesh.tris[i].idx[0];
		idx1 = temp_mesh.tris[i].idx[1];
		idx2 = temp_mesh.tris[i].idx[2];

		idx0 = gmeshAddVert(&node->mesh,
			temp_mesh.positions?temp_mesh.positions[idx0]:0,
			temp_mesh.normals?temp_mesh.normals[idx0]:0,
			temp_mesh.tex1s?temp_mesh.tex1s[idx0]:0,
			st3s[0],
			temp_mesh.boneweights?temp_mesh.boneweights[idx0]:0,
			temp_mesh.bonemats?temp_mesh.bonemats[idx0]:0,
			0);
		idx1 = gmeshAddVert(&node->mesh,
			temp_mesh.positions?temp_mesh.positions[idx1]:0,
			temp_mesh.normals?temp_mesh.normals[idx1]:0,
			temp_mesh.tex1s?temp_mesh.tex1s[idx1]:0,
			st3s[1],
			temp_mesh.boneweights?temp_mesh.boneweights[idx1]:0,
			temp_mesh.bonemats?temp_mesh.bonemats[idx1]:0,
			0);
		idx2 = gmeshAddVert(&node->mesh,
			temp_mesh.positions?temp_mesh.positions[idx2]:0,
			temp_mesh.normals?temp_mesh.normals[idx2]:0,
			temp_mesh.tex1s?temp_mesh.tex1s[idx2]:0,
			st3s[2],
			temp_mesh.boneweights?temp_mesh.boneweights[idx2]:0,
			temp_mesh.bonemats?temp_mesh.bonemats[idx2]:0,
			0);
		gmeshAddTri(&node->mesh, idx0, idx1, idx2, temp_mesh.tris[i].tex_id, 0);
	}

	gmeshUpdateGrid(&node->mesh, 0);

	eaDestroyEx(&primitives, primDestroy);

	assertmsg(!node->reductions, "uvunwrapping of meshes with LOD instructions not yet implemented!");

	gmeshFreeData(&temp_mesh);

	//node->lightmap_size = size;
}

static void makeLodName(char *src, char *dest, char *trick_name, int lodnum, int is_object_library)
{
	char *s;
	char buf[1024];
	s = strstr(src,"__");
	if (s)
		*s = 0;

	if (is_object_library && src[0] != '_')
		sprintf(buf, "_%s_LOD%d%s%s", src, lodnum, trick_name?"__":"", trick_name?trick_name:"");
	else
		sprintf(buf, "%s_LOD%d%s%s", src, lodnum, trick_name?"__":"", trick_name?trick_name:"");

	if (s)
		*s = '_';

	// use a temp buffer in case src == dest
	strcpy(dest, buf);
}

static Node *makeNodeCopy(Node *srcnode, char *trick_name, int lodnum)
{
	Node *node = newNode();
	memcpy(node, srcnode, sizeof(*node));
	ZeroStruct(&node->mesh);
	node->child = 0;
	node->parent = 0;
	node->next = 0;
	node->prev = 0;
	node->nodeptr = 0;
	ZeroStruct(&node->poskeys);
	ZeroStruct(&node->rotkeys);
	node->reductions = 0;

	makeLodName(node->name, node->name, trick_name, lodnum, 0);

	return node;
}

static void reduceNonDefaultLODs(ModelLODInfo *lod_info, Node *srcnode, Node ***LODlist, char *trick_name)
{
	int j, numlods;
	GMesh srcmesh={0};

	gmeshCopy(&srcmesh, &srcnode->mesh, 0);

	numlods = eaSize(&lod_info->lods);
	for (j = 0; j < numlods; j++)
	{
		AutoLOD *lod = lod_info->lods[j];
		int method = ERROR_RMETHOD;

		if (lod->flags & LOD_ERROR_TRICOUNT)
			method = TRICOUNT_RMETHOD;

		if (lod->max_error == 0 && j == 0)
			continue;

		if (lod->lod_modelname)
			continue;

		if (j == 0)
		{
			// for the high LOD, replace the given model
			gmeshFreeData(&srcnode->mesh);
			gmeshReduce(&srcnode->mesh, &srcmesh, srcnode->reductions, lod->max_error / 100.f, method, 0);
		}
		else
		{
			// for the low LODs, make a separate file
			Node *node = makeNodeCopy(srcnode, trick_name, j);
			gmeshReduce(&node->mesh, &srcmesh, srcnode->reductions, lod->max_error / 100.f, method, 0);
			if (node->mesh.tri_count)
				eaPush(LODlist, node);
			else
				freeNode(node);
		}
	}

	gmeshFreeData(&srcmesh);
}

extern int no_lods;
static void reduceNode(ModelLODInfo *lod_info, Node *srcnode, Node ***LODlist, int default_lods_ok)
{
	char *trick_name = 0;
	TrickInfo *trick = 0;

	if (no_lods)
		return;

	trick = trickFromObjectName(srcnode->name, "");
	if (trick)
		trick_name = trick->name;

	reduceNonDefaultLODs(lod_info, srcnode, LODlist, trick_name);
}

static int nodeNamesEq(char *node1, char *node2)
{
	int ret = 0;
	char *s1, *s2;

	s1 = strstr(node1,"__");
	s2 = strstr(node2,"__");

	if (s1)
		*s1 = 0;
	if (s2)
		*s2 = 0;

	ret = stricmp(node1, node2)==0;

	if (s1)
		*s1 = '_';
	if (s2)
		*s2 = '_';

	return ret;
}

/*read one VRML file into a node tree and massage it into shape for the game.
addFile is the only entry point into geo.c (except a few processing functions used by vrml.c 
(which is in turn only called by this function and parseAnimName.. which really should be in 
some util type file) because it's a bit easier to do those bits of node 
processing as nodes are read in.) (Note that some processing of bone info is being done in 
packSkin in output.c which should really be done here.)

Returns a node list of LODs, if generated.
*/
void geoAddFile(char *fname, char *geo_fname, int merge_nodes, int whattoditch, int do_unwrapping, int is_legacy)
{
	Node	*root;
	char animname[MAX_PATH];
	int is_player_library = whattoditch == PROCESS_GEO_ONLY;
	int is_object_library = whattoditch == PROCESS_ALL_NODES;
	int i, count;
	Node **list;

	loadstart_printf("processing %s",fname);

	loadstart_printf("loading VRML.. ");
	root = readVrmlFile(fname);
	if(!root)
		FatalErrorf("Can't open %s for reading!\n",fname);
	loadend_printf("done");
	loadstart_printf("Processing.. ");
	
	root = doAltPivots(root);	//must come before merge nodes
	if (!root)
	{
		Errorf("The file %s contains no geometry (only Alt-pivots)", fname);
		treeFree();
		loadend_printf("done");
		loadend_printf("done");
		return;
	}

	if (merge_nodes)	//needs to happen before some of the functions below or they get confused
		root = mergeNode(root,0);

	//if whattoditch == PROCESS_ANIM_ONLY || PROCESS_SCALEBONES_ONLY, check to make sure bones are all legit
	assignBoneNums(root);

	crazyNormalTrick(root);  //does both "_N" trick and "_SN" trick

	reorderTrisByTex(root); // JE: reoders triangles
	centerNode(root,unitmat); //doesn't seem to center node anymore, but does get vis sphere

	root = ditchUnneededStuff(root, whattoditch);

	if (PROCESS_ANIM_ONLY == whattoditch || whattoditch == PROCESS_SCALEBONES_ONLY)
		checkForBadBones(root);

	addgrid(root); // JE: pools verts

	// fpe 3/5/2009, enabled meshmender to fix bumpmapping 
	//	anomalies caused by degenerate tangent space due to mirrored uv's
	//	fpe 9/29/09, enabled for object_library (used to be only player_library)
	if(do_meshMend)
	{
		// remove degenerate triangles before meshmend
		if(is_object_library)
		{
			list = treeArray(root,&count);
			for (i = 0; i < count; i++)
			{
				if (gmeshMarkDegenerateTris(&list[i]->mesh))
					gmeshUpdateGrid(&list[i]->mesh, 0);
			}
		}
		meshMenderRecurse(root);
	}

	optimizeBones(root); // JE: reorders triangles, how does this not mess up sorting by texture??

	parseAnimNameFromFileName(fname, animname); //cut out path and .wrl to get name of this anim

	{
		int		j, k;
		Node	**LODlist = 0;

		// remove degenerate tris so the AutoLODer doesn't freak out
		list = treeArray(root,&count);
		if (is_object_library)
		{
			for (i = 0; i < count; i++)
			{
				if (gmeshMarkDegenerateTris(&list[i]->mesh))
					gmeshUpdateGrid(&list[i]->mesh, 0);
			}
		}

		loadend_printf("done");
		eaCreate(&LODlist);

		if (whattoditch == PROCESS_ANIM_ONLY || whattoditch == PROCESS_SCALEBONES_ONLY )
		{
			for (i = 0; i < count; i++)
				ditchGrid(list[i]);
		}
		else
		{

			// remove nodes with LOD names
			if (is_player_library && !no_lods)
			{
				char buffer[1024];
				TrickInfo *trick;
				char *trick_name;

				for (i = 0; i < count; i++)
				{
					if (!list[i])
						continue;

					trick_name = 0;
					trick = trickFromObjectName(list[i]->name, "");
					if (trick)
						trick_name = trick->name;

					for (j = 1; j < 4; j++)
					{
						makeLodName(list[i]->name, buffer, trick_name, j, 0);

						for (k = 0; k < count; k++)
						{
							if (i == k || !list[k])
								continue;
							if (nodeNamesEq(buffer, list[k]->name))
								list[k] = 0;
						}
					}
				}

					// cleanup the list
				for (i = 0; i < count; i++)
				{
					if (!list[i])
					{
						count--;
						if (i < count)
						{
							list[i] = list[count];
							i--;
						}
					}
				}
			}

			loadstart_printf("creating reduce instructions.. ");
			for (i = 0; i < count; i++)
                list[i]->reductions = makeGMeshReductions(&list[i]->mesh, list[i]->lod_distances, list[i]->min, list[i]->max, 1, 1, !is_object_library);
			loadend_printf("done");

			if (is_player_library)
			{
				// make LODs
				loadstart_printf("creating LODs.. ");
				for (i = 0; i < count; i++)
				{
					Vec3 dv, min, max;
					ModelLODInfo *lod_info;
					subVec3(list[i]->min,list[i]->mat[3],min);
					subVec3(list[i]->max,list[i]->mat[3],max);
					subVec3(max, min, dv);
					// only use defaults for player geometry
					lod_info = lodinfoFromObjectName(getPlayerLibraryDefaultLODs(), list[i]->name, geo_fname, lengthVec3(dv), is_legacy, list[i]->mesh.tri_count, 0);
					reduceNode(lod_info, list[i], &LODlist, 0);
					freeGMeshReductions(list[i]->reductions);
					list[i]->reductions = 0;
				}
				loadend_printf("done");
			}

			for (i = 0; i < eaSize(&LODlist); i++)
				list[count++] = LODlist[i];
		}

		// do this again to make sure the LODs don't have degenerate tris before being uvunwrapped
		if (is_object_library)
		{
			loadstart_printf("Removing degenerate triangles.. ");
			for (i = 0; i < count; i++)
			{
				if (gmeshMarkDegenerateTris(&list[i]->mesh))
					gmeshUpdateGrid(&list[i]->mesh, 0);
			}
			loadend_printf("done.");
		}

		if (is_object_library && do_unwrapping)
		{
			loadstart_printf("uvunwrapping.. ");
			for (i = 0; i < count; i++)
				nodeUvunwrap(list[i]);
			loadend_printf("done");
		}

		outputPackAllNodes(animname, list, count); //turn list of nodes into a ModelHeader with models
		treeFree();
		eaDestroyEx(&LODlist, freeNode);
	}


	loadend_printf("");
	return;
}



