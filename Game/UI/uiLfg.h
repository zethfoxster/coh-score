
#ifndef UILFG_H
#define UILFG_H


int lfgWindow();

void lfg_clearAll();
void lfg_add( char * name, int archetype, int origin, int level, int lfg,
				 int hidden, int teamsize, int leader, int sameteam, int leaguesize, 
				 int leagueleader, int sameleague, char *mapName, int dbid, int arena_map, 
				 int mission_map, int can_co_faction, int faction_same_map, 
				 int can_co_universe, int universe_same_map, char * comment );
void lfg_setResults( int results, int actual );
void lfg_setNameWidth();
void lfg_setSearchMethod(int wdwType);

typedef struct Packet Packet;
void lfg_buildSearch(Packet *pak);

#define LFG_CLASS_COLUMN	"Class"
#define LFG_ORIGIN_COLUMN	"Origin"
#define LFG_LVL_COLUMN		"Level"
#define LFG_NAME_COLUMN		"Name"
#define LFG_MAP_COLUMN		"Map"
#define LFG_LFG_COLUMN		"Lfg"
#define LFG_COMMENT_COLUMN  "Comment"


void hideShowDialog(int clear_choices);
void hideSetValue( int hide_type, bool on );
void hideSetAll( bool on );
#endif