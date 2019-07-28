
#include "Index2StringMap.h"

void InitIndex2StringMap(Index2StringMap * map, int max)
{
	map->max = max;
	map->table = stashTableCreateWithStringKeys(max, StashDeepCopyKeys);
}


int GetIndexFromString(Index2StringMap * map, const char * name)
{
	int i = 0;
	stashFindInt(map->table, name, &i);

	if(!i)
	{	
		int size = stashGetValidElementCount(map->table);
		i = size + 1;	// skip over '0', as this value indicates "no entry yet"
		stashAddInt(map->table, name, i, false);
	}

	i -= 1;		// restore back to 0-based 

	if(i < map->max)
	{
		map->max = stashGetMaxSize(map->table);
	}
//	assert(i < map->max);	//otherwise time to increase

	return i;  
}

const char * GetStringFromIndex(Index2StringMap * map, int index)
{
	StashElement element;
	StashTableIterator it;

	index += 1; // must convert back to base-1

	assert((U32)index <= stashGetValidElementCount(map->table));

	// do linear search for name. Inefficient, but this is only used at very end when generating HTML
	stashGetIterator(map->table, &it);
	while(stashGetNextElement(&it, &element))
	{
		if(index == stashElementGetInt(element))
		{
			return stashElementGetStringKey(element);
		}
	}

	assert(!"Didn't find index in table!");
	return 0;
}