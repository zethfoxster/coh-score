#include "sprite.h"
#include "utils.h"
#include "assert.h"

typedef struct SpriteIndexElement {
	int next; // index into dynarray
	int prev; // index into dynarray
	int index;
} SpriteIndexElement;

typedef struct SpriteZElements {
	F32 zp;
	int elements_head; // index into dynarray
	int elements_tail; // index into dynarray
	int count; // debug
} SpriteZElements;

// Dyn arrays
static SpriteIndexElement *index_list;
static int index_list_count,index_list_max;

static SpriteZElements *sprite_zidxs;
static int sprite_zidxs_count,sprite_zidxs_max;

static int last_insert_point[2]={-1,-1};
static F32 last_insert_zp[2];

DisplaySprite *sprites=NULL;
int sprites_count=0;
int sprites_max=0;


void spriteListReset()
{
	int i;
	sprite_zidxs_count = 0;
	index_list_count = 0;
	for (i=0; i<ARRAY_SIZE(last_insert_point); i++)  {
		last_insert_point[i] = -1;
	}
}

static int findSpriteIndex(F32 zp)
{
	int size, i;
	int min, max, middle;

	// Check cache
	for (i=0; i<ARRAY_SIZE(last_insert_point); i++) {
		if (zp == last_insert_zp[i] && last_insert_point[i] != -1) {
			return last_insert_point[i];
		}
	}

	size = sprite_zidxs_count;

	// Binary search!
	min = 0;
	max = size;
	middle = 0;

	while (1)
	{
		if (min==max) {
			middle=min;
			break;
		}
		middle = (min + max) / 2;
		if (sprite_zidxs[middle].zp == zp) {
			break;
		} else if (zp < sprite_zidxs[middle].zp) {
			if (middle == 0 || zp > sprite_zidxs[middle-1].zp) {
				// It goes before this guy!
				break;
			}
			//assert(max!=middle); infinite loop error detection
			max = middle;
		} else {
			min = middle + 1;
		}
	}
	// Insert into cache
	last_insert_point[0] = last_insert_point[1];
	last_insert_point[1] = middle;
	last_insert_zp[0] = last_insert_zp[1];
	last_insert_zp[1] = zp;
	return middle;
}


void *dynArrayInsert(void **basep,int struct_size,int *count,int *max_count,int index)
{
	char *base = (char*)*basep;
	dynArrayAdd(basep, struct_size, count, max_count, 1);
	base = (char*)*basep; // re-evaluate, as dynArrayAdd may have just changed *basep
	if (index >= (*count - 1))
		return base + *count-1; // Return the recently added
	memmove(base + struct_size*(index+1), base + struct_size*(index), struct_size * (*count - index - 1));
	memset(base + struct_size*(index), 0, struct_size);
	return base + struct_size*index;
}

DisplaySprite *createDisplaySprite(F32 zp)
{
	SpriteIndexElement *elem;
	int i;

	// init
	if (!sprite_zidxs) {
		dynArrayAdd(&sprite_zidxs, sizeof(SpriteZElements), &sprite_zidxs_count, &sprite_zidxs_max, 30);
		sprite_zidxs_count = 0;
		dynArrayAdd(&index_list, sizeof(SpriteIndexElement), &index_list_count, &index_list_max, 128);
		index_list_count = 0;
	}

	elem = dynArrayAdd(&index_list, sizeof(SpriteIndexElement), &index_list_count, &index_list_max, 1);
	elem->next = elem->prev = -1;
	elem->index = sprites_count;
	assert(elem->index == index_list_count-1);

	i = findSpriteIndex(zp);
	if (i==sprite_zidxs_count || sprite_zidxs[i].zp > zp) {
		dynArrayInsert(&sprite_zidxs, sizeof(SpriteZElements), &sprite_zidxs_count, &sprite_zidxs_max, i);
		sprite_zidxs[i].count = 0;
		sprite_zidxs[i].elements_head = -1;
		sprite_zidxs[i].zp = zp;
		last_insert_point[0]=last_insert_point[1]=-1;
	}
	sprite_zidxs[i].count++;
	if (sprite_zidxs[i].elements_head!=-1) {
		SpriteIndexElement *oldhead = &index_list[sprite_zidxs[i].elements_head];
		oldhead->prev = index_list_count-1;
		elem->next = oldhead->index;
		sprite_zidxs[i].elements_head = index_list_count-1;
	} else {
		sprite_zidxs[i].elements_head = index_list_count-1;
		sprite_zidxs[i].elements_tail = index_list_count-1;
	}
	//printf("%d : %f\n", index_list_count - 1, zp);

	{
	DisplaySprite *sprite;
	sprite = dynArrayAdd(&sprites, sizeof(DisplaySprite), &sprites_count, &sprites_max, 1);
	assert(sprite == &sprites[sprites_count-1]);

	return sprite;
	}
}

int getSpriteIndex(int orderedIndex)
{
	static int last_orderedIndex=0;
	static int zindex=-1;
	static int walk;
	int ret;

	if (orderedIndex == 0) {
		// First one!
		zindex=0;
		walk=sprite_zidxs[0].elements_tail;
	} else {
		// Must be called sequentially, yeah, this is a hack
		assert(orderedIndex == last_orderedIndex+1);
	}
	last_orderedIndex=orderedIndex;
	if (walk==-1) {
		zindex++;
		walk = sprite_zidxs[zindex].elements_tail;
	}
	ret = walk;
	walk = index_list[walk].prev;
	return ret;
}
