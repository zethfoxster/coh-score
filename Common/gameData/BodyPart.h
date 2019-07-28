/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#ifndef BODYPART_H
#define BODYPART_H

#include "bones.h"
#include "costume.h"

#define MAX_BODYPARTS MAX_COSTUME_PARTS

typedef struct BodyPart
{
	char*	name;			// Reference name
	BoneId	bone_num[2];	// Bone ID in animation system, only 2 values if the Body part is really representing a left AND right value
	int		bodyPartIndex;	// Bone index in the BodyParts array
	int		num_parts;		// How many animation bones should be treated as one unit?
	int		influenceCost;	// How much does this cost in the tailor screen
	char*	name_geo;		// 
	char*	name_tex;		// 
	char*	base_var;		//
	char*   sourceFile;     // source file
	U8		dont_clear;		// when applying a new costume, don't clear this part if it isn't in the new costume.  could be dont_override instead.
} BodyPart;

typedef struct{
	const BodyPart** bodyParts;
} BodyPartList;

extern SHARED_MEMORY BodyPartList bodyPartList;

void bpReadBodyPartFiles(void);
const BodyPart* bpGetBodyPartFromName(const char* name);
int bpGetIndexFromName(const char *name);
int GetBodyPartCount(void);

#endif
