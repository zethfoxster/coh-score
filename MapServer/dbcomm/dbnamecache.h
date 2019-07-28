#ifndef _DBNAMECACHE_H
#define _DBNAMECACHE_H

#include "stdtypes.h"
#include "gametypes.h"
#include "netio.h"
#include "dbcontainer.h"

typedef struct
{
	char	name[64];
	Gender	gender;
	Gender	name_gender;
	int		id;
	int		playerType;
	int		playerSubType;
	int		playerTypeByLocation;
	int		praetorianProgress;
	int		authId;
} ContainerName;

extern ContainerName	*container_names;

void handleClearPnameCacheEntry(Packet *pak);
void handleSendEntNames(Packet *pak);
void handleSendGroupNames(Packet *pak);
ContainerName *dbPlayerNamesFromIds(int *db_ids,int count);
char *dbPlayerNameFromId(int db_id);
char *dbPlayerNameFromIdLocalHash(int db_id);
char *dbPlayerNameAndGenderFromId(int db_id,Gender *gender,Gender *name_gender);
char *dbPlayerMessageVarsFromId(int db_id,Gender *gender,Gender *name_gender,int *playerType, int *playerSubType);
int dbPlayerIdFromName(char *name);
int dbPlayerOnWhichMap(char *name);
void dbNameTableInit();
void dbAddToNameTables(const char *name, int db_id, Gender gender, Gender name_gender, int playerType, int playerSubType, int playerTypeByLocation, int praetorianProgress, int authId);
int dbSupergroupIdFromName(char *name);
char *dbSupergroupNameFromId(int sgid);
int dbPlayerTypeFromId(int db_id);
int dbPlayerSubTypeFromId(int db_id);
int dbPlayerTypeByLocationFromId(int db_id);
int dbPraetorianProgressFromId(int db_id);
int authIdFromName(char *name);
int authIdFromDbId(int db_id);
#endif
