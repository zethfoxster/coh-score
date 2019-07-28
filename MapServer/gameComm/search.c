

#include "entity.h"
#include "entPlayer.h"

#include "dbquery.h"
#include "dbnamecache.h"

#include "svr_base.h"
#include "staticMapInfo.h"
#include "earray.h"
#include "comm_game.h"
#include "langServerUtil.h"
#include "SimpleParser.h"
#include "utils.h"
#include "entVarUpdate.h"
#include "mathutil.h"
#include "classes.h"
#include "origins.h"
#include "teamCommon.h"
#include "auth/authUserData.h"
#include "cmdserver.h"
#include "character_base.h"

typedef struct Search
{
	// bitfields
	int origins;			
	int archetypes;

	// not bitfields
	int minLevel;
	int maxLevel;

	int lfg;
	bool all_maps;

	char name[32];
	int *mapIds;

}Search;

#define MAX_SEARCH_RESULTS	50

int search_match( Entity *e, Search * search, OnlinePlayerInfo *opi )
{
	int map_count = eaiSize(&search->mapIds);

	if( opi->origin < 0 || opi->archetype < 0 )
		return 0;

	if( opi->hidefield&(1<<HIDE_SEARCH) && !e->access_level )
		return 0;

	if( (search->lfg==LFG_NONE && opi->lfg) ) // looking for people not seeking
		return 0;

	if( search->lfg && !(search->lfg&LFG_NONE) && !(search->lfg&opi->lfg))  // looking for anyone or specific seeking
		return 0;

	if( search->origins && !(search->origins&(1<<opi->origin)) )
		return 0;

	if( search->archetypes && !(search->archetypes&(1<<opi->archetype)) )
		return 0;

	if( search->minLevel && (opi->level+1) < search->minLevel )
		return 0;

	if( search->maxLevel && (opi->level+1) > search->maxLevel )
		return 0;

	if( search->name && strnicmp( dbPlayerNameFromId(opi->db_id), search->name, strlen(search->name) ) != 0 )
		return 0;

	// Can't make assumptions about where someone can be given their player
	// type. We have to check their logical area against the other person to
	// see if one could reach the other's part of the world.
	if( e )
	{
		const MapSpec *spec = MapSpecFromMapId( e->static_map_id );

		if( ENT_IS_IN_PRAETORIA(e) )
		{
			if( opi->team_area != MAP_TEAM_PRAETORIANS && opi->team_area != MAP_TEAM_EVERYBODY )
				return 0;

			if( (spec->teamArea != MAP_TEAM_EVERYBODY || opi->team_area != MAP_TEAM_EVERYBODY) && !OPI_IS_IN_PRAETORIA(opi) )
				return 0;
		}
		else
		{
			if( opi->team_area == MAP_TEAM_PRAETORIANS )
				return 0;

			if( (spec->teamArea != MAP_TEAM_EVERYBODY || opi->team_area != MAP_TEAM_EVERYBODY) && OPI_IS_IN_PRAETORIA(opi) )
				return 0;

			if( ENT_IS_ON_BLUE_SIDE(e) && opi->team_area == MAP_TEAM_VILLAINS_ONLY )
				return 0;

			if( ENT_IS_ON_RED_SIDE(e) && opi->team_area == MAP_TEAM_HEROES_ONLY )
				return 0;

			if( ENT_IS_ON_BLUE_SIDE(e) && opi->influenceType != kPlayerType_Hero && !opi->can_co_faction )
				return 0;

			if( ENT_IS_ON_RED_SIDE(e) && opi->influenceType != kPlayerType_Villain && !opi->can_co_faction )
				return 0;
		}
	}

	if( !search->all_maps && map_count )
	{
		int i;
		for(i = 0; i < map_count; i++ )
		{
			StaticMapInfo *smi = staticMapGetBaseInfo(opi->map_id);
			if( smi && ( eaiGet(&search->mapIds, i) == smi->baseMapID) )
				return 1;
		}

		return 0;
	}

	return 1;
}

void search_find( Entity *e, Search *search )
{
	OnlinePlayerInfo * opi = onlinePlayerInfoIncremental();
	OnlinePlayerInfo **results = 0;
	int i, count, result_count = 0;
	int id_count = 0, db_ids[MAX_SEARCH_RESULTS];

	eaCreate(&results);


	// find all the players matching search
	while( opi && result_count < MAX_SEARCH_RESULTS )
	{
		if( search_match( e, search, opi ) )
		{
			result_count++;
			eaPush(&results, opi);
		}

		opi = onlinePlayerInfoIncremental();
	}

	count = MIN( eaSize(&results), MAX_SEARCH_RESULTS );

	for( i = 0; i < count; i++ )
	{
		if( !dbPlayerNameFromIdLocalHash(results[i]->db_id) )
		{
			db_ids[id_count] = results[i]->db_id;	
			id_count++;
		}
	}

	if( id_count )
		dbPlayerNamesFromIds(&db_ids[0],id_count);

	START_PACKET(pak, e, SERVER_SEARCH_RESULTS)
	pktSendBits( pak, 32, result_count );
	pktSendBits( pak, 32, count );
	// now send down data to client
	for( i = 0; i < count; i++ )
	{
		StaticMapInfo *smi = staticMapInfoFind(results[i]->map_id);
		Entity *ent = entFromDbId( results[i]->db_id );

		pktSendString( pak, dbPlayerNameFromId(results[i]->db_id) );
		pktSendBitsPack( pak, 10, ent?ent->pl->lfg:results[i]->lfg );
		pktSendBitsPack( pak, 1, (ent?ent->pl->hidden:results[i]->hidefield)&(1<<HIDE_SEARCH) );

		if( ent && ent->teamup )
		{
			pktSendBitsPack( pak, 4, ent->teamup->members.count );
			pktSendBitsPack( pak, 1, ent->teamup->members.leader == e->db_id );
		}
		else
		{
			pktSendBitsPack( pak, 4, results[i]->teamsize );
			pktSendBitsPack( pak, 1, results[i]->leader );
		}

		if( ent && e->teamup_id == ent->teamup_id )
			pktSendBitsPack( pak, 1, 1 );
		else
			pktSendBitsPack( pak, 1, 0 );

		if( ent && ent->league )
		{
			pktSendBitsPack( pak, 8, ent->league->members.count );
			pktSendBitsPack( pak, 1, ent->league->members.leader == e->db_id );
		}
		else
		{
			pktSendBitsPack( pak, 8, results[i]->leaguesize );
			pktSendBitsPack( pak, 1, results[i]->league_leader );
		}

		if( ent && e->league_id == ent->league_id )
			pktSendBitsPack( pak, 1, 1 );
		else
			pktSendBitsPack( pak, 1, 0 );

		pktSendBitsPack( pak, 4, results[i]->archetype);
		pktSendBitsPack( pak, 4, results[i]->origin );
		pktSendBitsPack( pak, 4, results[i]->level );
		pktSendBitsPack( pak, 32, results[i]->db_id );
		pktSendBitsPack( pak, 1, results[i]->arena_map );
		pktSendBitsPack( pak, 1, results[i]->mission_map );

		// Opposite faction? This doesn't directly preclude teaming but it
		// does have some consequences.
		if( ( ENT_IS_ON_BLUE_SIDE(e) && results[i]->influenceType != kPlayerType_Hero ) ||
			( ENT_IS_ON_RED_SIDE(e) && results[i]->influenceType != kPlayerType_Villain ) )
		{
			pktSendBitsPack( pak, 1, 1 );
		}
		else
		{
			pktSendBitsPack( pak, 1, 0 );
		}

		pktSendBitsPack( pak, 1, results[i]->faction_same_map );

		// Other universe? Like faction, it doesn't directly preclude teaming
		// but it does change the rules in some ways.
		if( ENT_IS_IN_PRAETORIA(ent) )
		{
			if( !OPI_IS_IN_PRAETORIA(results[i]) )
			{
				pktSendBitsPack( pak, 1, 1 );
			}
			else
			{
				pktSendBitsPack( pak, 1, 0 );
			}
		}
		else
		{
			if( OPI_IS_IN_PRAETORIA(results[i]) )
			{
				pktSendBitsPack( pak, 1, 1 );
			}
			else
			{
				pktSendBitsPack( pak, 1, 0 );
			}
		}

		pktSendBitsPack( pak, 1, results[i]->universe_same_map );

		pktSendString( pak ,results[i]->comment );
		pktSendString( pak, getTranslatedMapName(results[i]->map_id) );
	}

	END_PACKET

	eaDestroy(&results);
}



static int archtypeIdxFromDisplayName( char * name )
{
	int i;

	// first compare to internal names
	for( i = eaSize(&g_CharacterClasses.ppClasses)-1; i >= 0; i-- )
	{
		if( stricmp( name, g_CharacterClasses.ppClasses[i]->pchName) == 0 )
			return i;
	}	

	// next compare to display names
	for( i = eaSize(&g_CharacterClasses.ppClasses)-1; i >= 0; i-- )
	{
		if( strnicmp( name, localizedPrintf(0,g_CharacterClasses.ppClasses[i]->pchDisplayName), strlen(name) ) == 0 )
			return i;
	}
	return -1;	
}

static int originIdxFromDisplayName( char * name )
{
	int i;
	// first compare internal names
	for( i = eaSize(&g_CharacterOrigins.ppOrigins)-1; i >= 0; i-- )
	{
		if( stricmp( name, g_CharacterOrigins.ppOrigins[i]->pchName ) == 0 )
			return i;
	}

	// next try comparing to display names
	for( i = eaSize(&g_CharacterOrigins.ppOrigins)-1; i >= 0; i-- )
	{
		if( strnicmp( name, localizedPrintf(0,g_CharacterOrigins.ppOrigins[i]->pchDisplayName), strlen(name) ) == 0 )
			return i;
	}
	return -1;	
}


void search_parse( Entity *e, char* string )
{
	char	*tmp = strdup(string);
	char	*mem = tmp;
	char	*token;
	Search	search = {0};
	bool nameFirst = 1;

 	tmp = (char*)removeLeadingWhiteSpaces(tmp);
	token = strtok(tmp, "+" );

	dbUpdateAllOnlineInfoComments();
	

	// if the first character is a '+' the first term is not a name
 	if( tmp[0] != '+' && token )
	{
		removeTrailingWhiteSpaces(&token);
		strncpy(search.name, token, 31 );
		token = strtok(NULL, "+" );
	}

	while( token )
	{
		int tmp_int;
		char *maxLevel;

		removeTrailingWhiteSpaces(&token);

		// archetype flag
		tmp_int = archtypeIdxFromDisplayName(token);
		if( tmp_int >= 0 )
			search.archetypes |= (1<<tmp_int);

		// origin flag
		tmp_int = originIdxFromDisplayName(token);
		if( tmp_int >= 0 )
			search.origins |= (1<<tmp_int);

		//lfg
		if( stricmp( token, "lfg" ) == 0 )
			search.lfg |= LFG_ANY;
		if( stricmp( token, "lftf" ) == 0 )
			search.lfg |= LFG_TF;
		if( stricmp( token, "lfstreet" ) == 0 )
			search.lfg |= LFG_HUNT;
		if( stricmp( token, "lfmission" ) == 0 )
			search.lfg |= LFG_DOOR;
		if( stricmp( token, "lfarena" ) == 0 )
			search.lfg |= LFG_ARENA;
		if( stricmp( token, "lftrial" ) == 0 )
			search.lfg |= LFG_TRIAL;
		if( stricmp( token, "lfnone" ) == 0 )
			search.lfg |= LFG_NONE;
		if( stricmp( token, "nogroup" ) == 0 )
			search.lfg |= LFG_NOGROUP;

		if( strnicmp( token, "all", 3 ) == 0 )
			search.all_maps = true;

		// level ranges
		if( atoi(token) )
		{
			search.minLevel = atoi(token);

			// level ranges
			maxLevel = strchr( token, '-' );

			if( maxLevel && atoi(maxLevel+1) )
				search.maxLevel = atoi(maxLevel+1);
		}

		//mapname
		if( !search.all_maps )
		{
			tmp_int = staticMapInfoMatchNames( token );
			if( tmp_int >= 0 )
			{
				if( !search.mapIds )
					eaiCreate(&search.mapIds);
			
				eaiPush( &search.mapIds, tmp_int );
			}
		}
		token = strtok(NULL, "+" );
	}

	// if no all map flag and no specific map mentioned, search current map
	if( !search.all_maps && !search.mapIds )
	{
		StaticMapInfo *smi = staticMapGetBaseInfo(e->map_id );
		eaiCreate(&search.mapIds);
		eaiPush(&search.mapIds, smi?smi->baseMapID:e->map_id );
	}

	search_find( e, &search );

	if( search.mapIds )
		eaiDestroy(&search.mapIds);

	if( mem )
		free(mem);
}

void search_receive( Entity *e, Packet *pak )
{
	Search	search = {0};
	int idx;
	dbUpdateAllOnlineInfoComments();

	// name
	if( pktGetBits(pak,1))
	{
		char * strptr;
		strncpy(search.name, pktGetString(pak), 31);
		strptr = search.name;
		removeTrailingWhiteSpaces(&strptr);
	}

	// class
	idx = pktGetBitsPack(pak,1);
	while( idx >= 0 )
	{
		search.archetypes |= (1<<idx);
		idx = pktGetBitsPack(pak,1);
	}

	// origin
	idx = pktGetBitsPack(pak,1);
	while( idx >= 0 )
	{
		search.origins |= (1<<idx);
		idx = pktGetBitsPack(pak,1);
	}

	// maps
	if( pktGetBits(pak,1) )
	{
		while( pktGetBits(pak,1) )
		{
			idx = staticMapInfoMatchNames( pktGetString(pak) );
			if( idx >= 0 )
			{
				if( !search.mapIds )
					eaiCreate(&search.mapIds);
				eaiPush( &search.mapIds, idx );
			}
		}
	}
	else
	{
		search.all_maps = true;
	}

	// levels
	if( pktGetBits(pak,1) )
	{
		search.minLevel = pktGetBitsPack(pak,1);
		search.maxLevel = pktGetBitsPack(pak,1);
	}

	search.lfg = pktGetBitsPack(pak,1);

	search_find( e, &search );
}