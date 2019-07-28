#include "bones.h"
#include "structDefines.h"
#include "utils.h"
#include "assert.h"

#include "AutoGen/bones_h_ast.c"

BoneId bone_IdFromName(const char *name)
{
	STATIC_ASSERT(BONEID_INVALID == -1);
	return StaticDefineIntGetInt(BoneIdEnum, name); // returns -1 if not found
}

BoneId bone_IdFromText(const char *text)
{
	// abuse what we know about AUTO_ENUM to keep things fast, and allow traversing the list in reverse
	int i;

	assert(BoneIdEnum[1].value == 0);
	assert(BoneIdEnum[BONEID_COUNT+1].value == BONEID_COUNT);

	if(!text || !text[0])
		return BONEID_INVALID;

	// the list *must* be traversed in reverse, because NECK and NECKLINE collide
	for(i = BONEID_COUNT; i >= 1; --i)
		if(strStartsWith(text, BoneIdEnum[i].key))
			return BoneIdEnum[i].value;

	return BONEID_INVALID;
}

const char* bone_NameFromId(BoneId boneid)
{
	if(INRANGE0(boneid, BONEID_COUNT))
	{
		// abuse what we know about AUTO_ENUM to get the speed of an array lookup
		StaticDefineInt *guess = &BoneIdEnum[boneid+1];
		if(devassert(guess->value == boneid))
			return guess->key;
		return StaticDefineIntRevLookup(BoneIdEnum, boneid);
	}
	return NULL;
}
