#include "villain_stats.h"
#include "utils.h"
#include "common.h"





// villain names are assumed to be of format
//
//		<VILLAIN TYPE>:###
//
BOOL GetVillainTypeFromName(const char * entityName, char * villainTypeOut)
{
	const char * p = entityName;
    strcpy_unsafe(villainTypeOut, ""); 

	while(   *p
		  && *p != ':')
	{
		*(villainTypeOut++) = *(p++);

		assert(p - entityName < 200);	// if this trips, name most likely does not have a ':' BAD NEWS!!!
	}

	*villainTypeOut = '\0';

	return (*p == ':');
}
