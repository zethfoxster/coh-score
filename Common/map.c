#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "group.h"
#include "grouputil.h"
#include "syslog.h"
#include "entity.h"
#include "dbcomm.h"
#include "dobbs_hash.h"
#include "entVarUpdate.h"
#include "entity.h"
#include "utils.h"
#include "error.h"
#include "comm_game.h"

#if SERVER
#include "sendToClient.h"
#include "parseClientInput.h"
#endif

#if CLIENT


#endif

int		map_cnt;
Map		maps[ MAX_MAPS ];
int		location_types_cnt;
char	location_types[ MAX_LOCATION_TYPES ][ MAX_NAME_LEN ];	//Slum, Office, Park.

// Server needs to send doorway list to client, internally 
// mark what maps go with what doorways.

//Doorway	dw;

Doorway	doorways[ MAX_DOORWAYS ];
int		dw_cnt;



char	somn_tmp[256];

char *suckOutMapName( char *s )
{
	int	i, len = strlen(s) - 4;	//strip off .txt

	if (len < 0)
		return 0;
	while( len && s[len-1] != '\\' && s[len-1] !='/' )
		len--;

	i = 0;
	while( s[len] != '.' && s[len] )
		somn_tmp[i++]=s[len++];
	somn_tmp[i] = 0;
	strupr( somn_tmp );			//Apr. 29/02.  Make a hash of this name be the same regardless of if
								//somebody makes name be upper or lower case.
	return ( char *)&somn_tmp;
}



char curr_map_name[256];
int			curr_map_hid;


char *shortVersionCurrMapname()
{
	return suckOutMapName( curr_map_name );
}


static int is_mission_map;		//this is calculated every time setMapName is called (should be once per
								//mapserver) rather than every time missionMap is calc. for performance.
static int map_idx;

void setMapName( char * s )
{
	int	i;
	char * map_short;

	strcpy( curr_map_name, s );
	map_short = shortVersionCurrMapname();

	if(map_short)
		curr_map_hid = ( int )STR_HASH( map_short );
	else 
		curr_map_hid = 0;

	is_mission_map = 0;
	map_idx = -1;
	for(i=0;i<map_cnt;i++)
	{
		if( stricmp( curr_map_name, maps[i].name )==0 )
			map_idx = i;

		if( !maps[i].static_map && stricmp( curr_map_name, maps[i].name )==0 )	//( char *)&maps[i].name[5] ) )	//skip /map first part.
			is_mission_map = i + 1;
	}
}

int currMap( char *s )
{
	return stricmp( s, curr_map_name )==0;
}


int getMapIdx()
{
	return map_idx;
}


int	missionMap()
{
	return is_mission_map;
}

CombatVsType combatVsType()
{
	int	i;

	i = getMapIdx();

	if( i < 0 )
		return PLAYER_VS_CRITTER;
	else return maps[ i ].combat_vs_type;
}

#if SERVER


char* getLocation TypeString(int* locIndices, int locCount)
{
	static char locationBuffer[1024];
	char* printCursor;
	int i;

	printCursor = locationBuffer;
	*printCursor = '\0';

	for(i = 0; i < locCount; i++){
		printCursor += sprintf(printCursor, location_types[locIndices[i]]);
		if(i != (locCount - 1))
			printCursor += sprintf(printCursor, ", ");
	}

	return locationBuffer;
}
#endif

int locationTypeFromString(int add_if_unique, char* loc_type_name)
{
	int	i;

	for( i = 0; i < location_types_cnt; i++ )
		if( stricmp( loc_type_name, location_types[ i ] )==0 )
			return i;

	if( !add_if_unique )
		return -1;

	if( location_types_cnt >= MAX_LOCATION_TYPES )
	{
		logNAddEntry( LF_ERRORS, "Too many locations, dropping last added." );
		return ( MAX_LOCATION_TYPES - 1 );
	}

	if( strlen( loc_type_name ) >= MAX_NAME_LEN )
	{
		logNAddEntry( LF_ERRORS, "Location type name, %s is to



