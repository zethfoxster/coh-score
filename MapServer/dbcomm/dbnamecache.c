#include "dbnamecache.h"
#include "StashTable.h"
#include "utils.h"
#include "timing.h"
#include "entity.h"
#include "dbcomm.h"
#include "comm_backend.h"

static StashTable db_id_hashes;
static StashTable sg_id_hashes;
static StashTable auth_id_hashes;

static StashTable name_hashes;
static StashTable sg_name_hashes;

ContainerName	*container_names;
int				container_name_count,container_name_max;

void dbNameTableInit()
{
	db_id_hashes = stashTableCreateInt(4);
	sg_id_hashes = stashTableCreateInt(4);
	auth_id_hashes = stashTableCreateInt(4);
	
	name_hashes = stashTableCreateWithStringKeys(1000, StashDeepCopyKeys);
	sg_name_hashes = stashTableCreateWithStringKeys(1000, StashDeepCopyKeys);
}

void dbAddToNameTables(const char *name, int db_id, Gender gender, Gender name_gender, int playerType, int playerSubType, int playerTypeByLocation, int praetorianProgress, int authId)
{
	int dummy, len;
	char *oldstring, *newstring;

	if (!name_hashes)
		name_hashes = stashTableCreateWithStringKeys(1000, StashDeepCopyKeys);

	if (!stashFindInt(name_hashes,name,&dummy))
		stashAddInt(name_hashes, name, db_id, false);

	// new hash table serves as a converter from auth_id to db_id
	if( authId )
		stashIntAddInt(auth_id_hashes, db_id, authId, 1 );

	if (db_id <= 0)
		name = "";
	if (!stashIntFindPointer(db_id_hashes, db_id, &oldstring))
		oldstring = NULL;
	
	len = strlen(name);

	newstring = calloc(1, len+7);
	strcpy(newstring,name);
	newstring[len+1] = (U8)gender;
	newstring[len+2] = (U8)name_gender;
	newstring[len+3] = (U8)playerType;
	newstring[len+4] = (U8)playerSubType;
	newstring[len+5] = (U8)praetorianProgress;
	newstring[len+6] = (U8)playerTypeByLocation;

	if (oldstring && memcmp(oldstring,newstring,len+7) == 0)
	{
		SAFE_FREE(newstring);
		return;
	}
	else if (oldstring)
	{
		stashIntRemovePointer(db_id_hashes, db_id, NULL);
		SAFE_FREE(oldstring);
	}

	if (!oldstring && db_id)
	{
		stashIntAddPointer(db_id_hashes, db_id, newstring, false);
	}
}

void handleSendEntNames(Packet *pak)
{
	int				list_id,i;

	list_id = pktGetBitsPack(pak,1);
	container_name_count	= pktGetBitsPack(pak,1);
	dynArrayFit(&container_names,sizeof(container_names[0]),&container_name_max,container_name_count);
	for(i=0;i<container_name_count;i++)
	{
		container_names[i].id = pktGetBitsPack(pak,1);
		strcpy(container_names[i].name,unescapeString(pktGetString(pak)));
		container_names[i].gender = pktGetBitsPack(pak,3);
		container_names[i].name_gender = pktGetBitsPack(pak,3);
		container_names[i].playerType = pktGetBitsPack(pak,1);
		container_names[i].playerSubType = pktGetBitsAuto(pak);
		container_names[i].playerTypeByLocation = pktGetBitsAuto(pak);
		container_names[i].praetorianProgress = pktGetBitsAuto(pak);
		container_names[i].authId = pktGetBitsPack(pak,1);
		dbAddToNameTables(container_names[i].name,
							container_names[i].id,
							container_names[i].gender,
							container_names[i].name_gender,
							container_names[i].playerType,
							container_names[i].playerSubType,
							container_names[i].playerTypeByLocation,
							container_names[i].praetorianProgress,
							container_names[i].authId);
	}
}

void handleSendGroupNames(Packet *pak)
{
	int				dummy,list_id,i;

	list_id = pktGetBitsPack(pak,1);
	container_name_count	= pktGetBitsPack(pak,1);
	dynArrayFit(&container_names,sizeof(container_names[0]),&container_name_max,container_name_count);
	for(i=0;i<container_name_count;i++)
	{
		container_names[i].id = pktGetBitsPack(pak,1);
		strcpy(container_names[i].name,pktGetString(pak));
		if (list_id == CONTAINER_SUPERGROUPS)
		{
			if (!stashIntFindPointer(sg_id_hashes, container_names[i].id, NULL))
				stashIntAddPointer(sg_id_hashes, container_names[i].id, strdup(container_names[i].name), false);

			if (!stashFindInt(sg_name_hashes,container_names[i].name,&dummy))
				stashAddInt(sg_name_hashes, strdup(container_names[i].name), container_names[i].id, false);
		}
	}
}

static ContainerName *dbSyncGetNames(int list_id,int *container_ids,int count)
{
	static PerformanceInfo* perfInfo;

	int		i;
	Packet	*pak;

	pak = pktCreateEx(&db_comm_link,list_id==CONTAINER_ENTS?DBCLIENT_REQ_ENT_NAMES:DBCLIENT_REQ_GROUP_NAMES);
	pktSendBitsPack(pak,1,list_id);
	pktSendBitsPack(pak,1,count);
	for(i=0;i<count;i++)
		pktSendBitsPack(pak,1,container_ids[i]);
	pktSend(&pak,&db_comm_link);

	if (dbMessageScanUntil(__FUNCTION__, &perfInfo))
		return container_names;
	return 0;
}

ContainerName *dbPlayerNamesFromIds(int *db_ids,int count)
{
	ContainerName	*cname;
	char			*name=NULL;
	int				i,id;

	dynArrayFit(&container_names,sizeof(container_names[0]),&container_name_max,count);
	for(i=0;i<count;i++)
	{
		id = db_ids[i];
		cname = &container_names[i];
		cname->id = id;
		if (!stashIntFindPointer(db_id_hashes, id, &name))
			break;
		strcpy(cname->name,name);
		cname->gender = name[strlen(name)+1];
		cname->name_gender = name[strlen(name)+2];
		cname->playerType = name[strlen(name)+3];
		cname->playerSubType = name[strlen(name)+4];
		cname->praetorianProgress = name[strlen(name)+5];
		cname->playerTypeByLocation = name[strlen(name)+6];
	}
	if (i < count)
		dbSyncGetNames(CONTAINER_ENTS,db_ids,count);
	return container_names;
}

char *dbSupergroupNameFromId(int sgid)
{
	char *name;
	if (stashIntFindPointer(sg_id_hashes, sgid, &name))
		return name;

	dynArrayFit(&container_names,sizeof(container_names[0]),&container_name_max,1);
	dbSyncGetNames(CONTAINER_SUPERGROUPS,&sgid,1);
	if (!stashIntFindPointer(sg_id_hashes, sgid, &name) || !name[0])
	{
		Errorf("Failed to get supergroup name from id");
		if (!stashIntFindPointer(sg_id_hashes, sgid, NULL))
			return NULL;
	}
	return name;
}

char *dbPlayerNameFromId(int db_id)
{
	ContainerName			*cnames;
	cnames = dbPlayerNamesFromIds(&db_id,1);
	// MAK - a hack I know, but I want a pointer I can hold on to

	return dbPlayerNameFromIdLocalHash(db_id);
}


char *dbPlayerNameFromIdLocalHash(int db_id)
{
	char* pcResult;
	if (stashIntFindPointer(db_id_hashes, db_id, &pcResult))
		return pcResult;
	return NULL;
}


char *dbPlayerNameAndGenderFromId(int db_id,Gender *gender,Gender *name_gender)
{
	ContainerName			*cnames;

	cnames = dbPlayerNamesFromIds(&db_id,1);
	if (gender)
		*gender = cnames->gender;
	if (name_gender)
		*name_gender = cnames->name_gender;
	return dbPlayerNameFromIdLocalHash(db_id);
}

char *dbPlayerMessageVarsFromId(int db_id,Gender *gender,Gender *name_gender,int *playerType, int *playerSubType)
{
	ContainerName			*cnames;

	cnames = dbPlayerNamesFromIds(&db_id,1);
	if (gender)
		*gender = cnames->gender;
	if (name_gender)
		*name_gender = cnames->name_gender;
	if (playerType)
		*playerType = cnames->playerType;
	if (playerSubType)
		*playerSubType = cnames->playerSubType;
	return dbPlayerNameFromIdLocalHash(db_id);
}

int dbPlayerTypeFromId(int db_id)
{
	ContainerName			*cnames;

	cnames = dbPlayerNamesFromIds(&db_id,1);
	return cnames->playerType;
}

int dbPlayerSubTypeFromId(int db_id)
{
	ContainerName			*cnames;

	cnames = dbPlayerNamesFromIds(&db_id,1);
	return cnames->playerSubType;
}

int dbPlayerTypeByLocationFromId(int db_id)
{
	ContainerName			*cnames;

	cnames = dbPlayerNamesFromIds(&db_id,1);
	return cnames->playerTypeByLocation;
}

int dbPraetorianProgressFromId(int db_id)
{
	ContainerName			*cnames;

	cnames = dbPlayerNamesFromIds(&db_id,1);
	return cnames->praetorianProgress;
}

static char *trim(char *pch)
{
	int len = strlen(pch);

	// Get rid of leading spaces, inefficiently
	while(*pch==' ')
	{
		strcpy(pch, pch+1);
		len--;
	}
	// Get rid of trailing spaces, inefficiently
	while(len>0 && pch[len-1]==' ')
	{
		pch[len-1]='\0';
		len--;
	}

	return pch;
}

int dbPlayerIdFromName(char *name)
{
	int db_id,tmp;

	trim(name);
	if (stashFindInt(name_hashes,name,&db_id))
		return db_id;
	db_id = dbSyncContainerFindByElement(CONTAINER_ENTS, "Name", name, &tmp,0);
	if (db_id < 0)
		dbAddToNameTables(name, db_id, 0, 0, 0, 0, 0, 0, 0); // Add dummy name stash table for future fast look up
	else
		dbPlayerNameFromId(db_id);
	return db_id;
}

int dbSupergroupIdFromName(char *name)
{
	int id,tmp;

	trim(name);
	if (stashFindInt(sg_name_hashes,name,&id))
		return id;
	id = dbSyncContainerFindByElement(CONTAINER_SUPERGROUPS, "Name", name, &tmp, 0);
	return id;
}


int dbPlayerOnWhichMap(char *name)
{
	char	id_str[20];
	int		db_id,map_num;
	Entity	*e;

	db_id = dbPlayerIdFromName(name);
	if (db_id < 0)
		return -1;
	e = entFromDbId(db_id);
	if (e)
		return e->map_id;
	sprintf(id_str,"%d",db_id);
	dbSyncContainerFindByElement(CONTAINER_ENTS,"ContainerId",id_str,&map_num,1);
	// Returns -1 if the player is not online
	return map_num;
}

void handleClearPnameCacheEntry(Packet *pak)
{
	char	*name;
	int		db_id;

	name	= pktGetString(pak);
	db_id	= pktGetBitsPack(pak,1);
	if (!db_id)
		stashFindInt(name_hashes,name,&db_id);
	stashRemovePointer(name_hashes,name, NULL);
	if (db_id > 0)
	{
		char *oldstring;
		if(stashIntRemovePointer(db_id_hashes, db_id, &oldstring))
			SAFE_FREE(oldstring);
	}
}


int authIdFromName(char *name)
{
	int db_id,authId=0;

	db_id = dbPlayerIdFromName(name);
	stashIntFindInt(auth_id_hashes,db_id,&authId);
	return authId;
}

int authIdFromDbId(int db_id)
{
	int authId=0;
	char *name = dbPlayerNameFromId(db_id); // to ensure the name has been requested
	stashIntFindInt(auth_id_hashes,db_id,&authId);
	return authId;
}