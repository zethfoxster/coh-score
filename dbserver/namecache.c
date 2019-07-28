#include "StashTable.h"
#include "namecache.h"
#include "utils.h"
#include "error.h"
#include <string.h>

#include "container.h"
#include "clientcomm.h"
#include "dbinit.h"
#include "comm_backend.h"
#include "container_sql.h"
#include "sql_fifo.h"
#include "gametypes.h"
#include "log.h"

static StashTable db_id_hashes;
static StashTable name_hashes;
static StashTable entCatalog;

static StashTable locksByName;
static StashTable locksById;

void pnameAdd(const char *name, int db_id)
{
	int ret,dummy;
	char *oldstring;
	
	ret = stashFindInt(name_hashes,name,&dummy);
	if (ret && dummy <= 0)
		stashRemoveInt(name_hashes,name, NULL);
	if (!ret || dummy <= 0)
		stashAddInt(name_hashes, name, db_id, false);
	if ( !db_id || !stashIntFindPointer(db_id_hashes, db_id, &oldstring) )
		oldstring = NULL;
		
	{
		char	*s;
		int		len = strlen(name);
	
		s = malloc(len+1);
		strcpy(s,name);

		if (!oldstring || strcmp(oldstring,s) != 0)
		{
			if (oldstring)
			{
				SAFE_FREE(oldstring);
				if ( db_id ) 
					stashIntRemovePointer(db_id_hashes, db_id, NULL);
			}
			if ( db_id ) 
				stashIntAddPointer(db_id_hashes, db_id, s, false);
		}
		else
			SAFE_FREE(s);
	}
}

void pnameDelete(const char *name)
{
	int			db_id;
	StashElement element;

	if (!name)
		return;
	if (stashFindInt(name_hashes,name,&db_id) && db_id )
	{
		char *oldstring;
		if(stashIntRemovePointer(db_id_hashes, db_id, &oldstring))
			SAFE_FREE(oldstring);
	}

	stashFindElement(name_hashes, name, &element);
	if (element)
		stashElementSetPointer(element,0);
	//stashRemovePointer(name_hashes, name, NULL);
}

void pnameAddAll()
{
	int			i,db_id,idx,row_count;
	char		*cols,*name;
	U8			gender,name_gender,playerType,playerSubType,praetorianProgress,playerTypeByLocation;
	U32			authId;
	char		*primarySet,*secondarySet;
	ColumnInfo	*field_ptrs[9];

	loadstart_printf("Reading player names...");
	cols = sqlReadColumnsSlow(ent_list->tplt->tables,"","ContainerId, Name, Gender, NameGender, PlayerType, AuthId","",&row_count,field_ptrs);
	for(idx=0,i=0;i<row_count;i++)
	{
		db_id = *(int *)(&cols[idx]);
		idx += field_ptrs[0]->num_bytes;
		name = &cols[idx];
		idx += field_ptrs[1]->num_bytes;
		gender = *(U8 *)(&cols[idx]);
		idx += field_ptrs[2]->num_bytes;
		name_gender = *(U8 *)(&cols[idx]);
		idx += field_ptrs[3]->num_bytes;
		playerType = *(U8 *)(&cols[idx]);
		idx += field_ptrs[4]->num_bytes;
		authId = *(U32 *)(&cols[idx]);
		idx += field_ptrs[5]->num_bytes;
		if (name[0])
		{
			EntCatalogInfo* info = entCatalogGet( db_id, true );
			info->gender		= gender;
			info->nameGender	= name_gender;
			info->playerType	= playerType;
			info->authId		= authId;

			// These will get filled out below but in case they don't have
			// entries we should set them to something sane.
			info->playerSubType = kPlayerSubType_Normal;
			info->playerTypeByLocation = kPlayerType_Hero;
			info->praetorianProgress = kPraetorianProgress_PrimalBorn;
			info->primarySet[0] = '\0';
			info->secondarySet[0] = '\0';

			pnameAdd( name, db_id );
		}
	}
	free(cols);

	// XXX: Is this a safe way to access the Ents2 table?
	cols = sqlReadColumnsSlow(&(ent_list->tplt->tables[1]),"","ContainerId, PlayerSubType, PraetorianProgress, InfluenceType, OriginalPrimary, OriginalSecondary","",&row_count,field_ptrs);
	for(idx=0,i=0;i<row_count;i++)
	{
		EntCatalogInfo* info = NULL;

		db_id = *(int *)(&cols[idx]);
		idx += field_ptrs[0]->num_bytes;
		playerSubType = *(U8 *)(&cols[idx]);
		idx += field_ptrs[1]->num_bytes;
		praetorianProgress = *(U8 *)(&cols[idx]);
		idx += field_ptrs[2]->num_bytes;
		playerTypeByLocation = *(U8 *)(&cols[idx]);
		idx += field_ptrs[3]->num_bytes;
		primarySet = &cols[idx];
		idx += field_ptrs[4]->num_bytes;
		secondarySet = &cols[idx];
		idx += field_ptrs[5]->num_bytes;

		// If we didn't get identifying data for them on the first pass we
		// don't have any useful way for others to find them so there's no
		// point trying to keep just this data.
		info = entCatalogGet( db_id, false );
		if( info )
		{
			info->playerSubType = playerSubType;
			info->praetorianProgress = praetorianProgress;
			info->playerTypeByLocation = playerTypeByLocation;
			strcpy(info->primarySet, primarySet);
			strcpy(info->secondarySet, secondarySet);
		}
	}
	free(cols);
	loadend_printf("");
}

void pnameInit()
{
	db_id_hashes = stashTableCreateInt(4);
	name_hashes = stashTableCreateWithStringKeys(1000,StashDeepCopyKeys);
	entCatalog = stashTableCreateInt(32);
	locksByName = stashTableCreateWithStringKeys(512, StashDefault);
	locksById = stashTableCreateInt(512);
}

char *pnameFindById(int db_id)
{
	char	*name;

	if (db_id && stashIntFindPointer(db_id_hashes, db_id, &name))
	{
		return name;
	}
	return 0;
}

int pnameFindByName(const char *name)
{
	int		db_id;

	if (!stashFindInt(name_hashes,name,&db_id))
		db_id = -1;
	return db_id;
}

// When a player reuses or changes name, you need to make sure the mapservers all clear the name<->db_id mapping
// from their caches.
void playerNameClearCacheEntry(char *name,int db_id)
{
	int		i;
	Packet	*pak;
	MapCon	*map;

	LOG_OLD( "broadcasting name cache clear for %s, old db_id %d\n",unescapeString(name),db_id);
	for(i=0;i<map_list->num_alloced;i++)
	{
		map = (MapCon*) map_list->containers[i];
		if (!map->active || !map->link)
			continue;
		pak = pktCreateEx(map->link,DBSERVER_CLEAR_PNAME_CACHE_ENTRY);
		pktSendString(pak,unescapeString(name));
		pktSendBitsPack(pak,1,db_id);
		pktSend(&pak,map->link);
	}
}

void playerNameLogin(char* name, int db_id, int just_created, U32 authId)
{
	if (just_created)
		pnameDelete(name);

	entCatalogGet( db_id, true )->authId = authId;
	pnameAdd( name, db_id );
}

void playerNameDelete(char* name, int db_id)
{
	pnameDelete(name);
	entCatalogDelete(db_id);
}

static int tempLockCheckAccountValid( const char * name, U32 authId )
{
	TempLockedName * data;
	if (stashFindPointer(locksByName, name, &data))
	{
		if (data->authId != authId)
		{
			return 0; //name was found and does NOT belong to this account
		}
	}
	return 1;  //name was found and belongs this account or name was not found, thus name is valid
}

int playerNameValid(char* name, int* pIsEmpty, U32 authId)
{
	int empty = stricmp(name,"EMPTY")==0;
	if (pIsEmpty)
	{
		*pIsEmpty = empty;
	}

	return !empty &&
		pnameFindByName(name) <= 0 &&
		containerIdFindByElement(dbListPtr(CONTAINER_ENTS),"Name",name) <= 0 &&
		tempLockCheckAccountValid(name, authId);
}

void playerNameCreate(char* name,int db_id, U32 authId)
{
	int old_id;
	if (stashFindInt(name_hashes,name,&old_id))
		playerNameClearCacheEntry(name,old_id);
	playerNameLogin(name, db_id, 1, authId);
}

EntCatalogInfo	*entCatalogGet( int db_id, bool bAddIfNotFound )
{
	EntCatalogInfo* catalogInfo = NULL;
	if(db_id)
		stashIntFindPointer(entCatalog,db_id,&catalogInfo);
	if (( catalogInfo == NULL ) && ( bAddIfNotFound ))
	{
		assert(db_id != 0);
		catalogInfo = (EntCatalogInfo*)calloc(sizeof(EntCatalogInfo),1);	assert(catalogInfo);
		catalogInfo->id = db_id;
		stashIntAddPointer(entCatalog,db_id,catalogInfo,false);
	}
	return catalogInfo;		
}

void entCatalogDelete( int db_id )
{
	stashIntRemovePointer(entCatalog,db_id,NULL);	// OK to call stashIntRemovePointer() if db_id isn't in the table
}

void entCatalogPopulate( int db_id, EntCon *ent_container )
{
	EntCatalogInfo *entInfo = entCatalogGet(db_id, false);
	if ( entInfo )
	{
		entInfo->gender = ent_container->gender;
		entInfo->nameGender	= ent_container->nameGender;
		entInfo->playerType	= ent_container->playerType;
		entInfo->playerSubType = ent_container->playerSubType;
		entInfo->praetorianProgress = ent_container->praetorianProgress;
		entInfo->playerTypeByLocation = ent_container->playerTypeByLocation;
		strcpy(entInfo->primarySet, ent_container->primarySet);
		strcpy(entInfo->secondarySet, ent_container->secondarySet);
	}
}

// add a new name to lock list
// update time if it's the same user
// return 0: error adding
// return 1: successful add
int tempLockAdd( char * name, U32 authId, U32 time )
{
	TempLockedName * oldIdData;

	devassert(authId); //shouldn't be adding a zero authId
	// check in case this name has been taken
	if (pnameFindByName(name) > 0)
	{
		return 0;
	}

	//check if this account has something already
	if (stashIntFindPointer(locksById, authId, &oldIdData))
	{
		//refresh time for this account
		oldIdData->time = time;
		if (strcmp(oldIdData->name, name) != 0)
		{
			//delete old name and make a new one
			stashRemovePointer(locksByName, oldIdData->name, NULL);
			Strcpy(oldIdData->name, name);
			stashAddPointer(locksByName, oldIdData->name, oldIdData, false);
		}
	}
	else
	{
		//create new account and name entry
		TempLockedName * newData = malloc(sizeof(TempLockedName));
		//copy
		newData->authId = authId;
		Strcpy(newData->name, name);
		newData->time = time;

		stashAddPointer(locksByName, newData->name, newData, false);
		stashIntAddPointer(locksById, newData->authId, newData, false);
	}

	return 1;
}

int tempLockFindbyName( char * name )
{
	return stashFindPointer(locksByName, name, NULL);
}

//check if both the name and user exist - to allow for easy out
int tempLockCheckAccountandRefresh( char * name, U32 authId, U32 time )
{
	TempLockedName * data;
	if (stashFindPointer(locksByName, name, &data))
	{
		if (data->authId == authId)
		{
			//refresh time for this account
			data->time = time;
			return 1;
		}
	}
	
	return 0;
}

void tempLockCheckElement(StashElement elem, U32 timeout, U32 time)
{
	TempLockedName * data;

	data = stashElementGetPointer(elem);
	if (data->time + timeout < time)
	{
		//timeout passed, delete lock
		stashIntRemovePointer(locksById, data->authId, NULL);
		stashRemovePointer(locksByName, data->name, NULL);
		SAFE_FREE(data);
	}
}

//runs through all locked names and deletes those that have timed out
void tempLockTick(U32 timeout, U32 time)
{
	static StashTableIterator iter;
	StashElement elem;
	const U32 checksPerTick = 10;  // how many usernames to verify every tick
	U32 count = 0;
	U32 totalLocks = stashGetValidElementCount(locksById);

	//check names incrementally every frame;
	while (count < checksPerTick && count < totalLocks)
	{
		int isNext = iter.pTable ? stashGetNextElement(&iter, &elem) : 0;
		if (!isNext)
		{
			//reached the end, start over
			stashGetIterator(locksById, &iter);
			isNext = stashGetNextElement(&iter, &elem);
			devassert(isNext);
		}

		tempLockCheckElement(elem, timeout, time);
		++count;
	}
}

void tempLockRemovebyName( const char * name )
{
	TempLockedName * data;
	
	if (stashFindPointer(locksByName, name, &data))
	{
		stashIntRemovePointer(locksById, data->authId, NULL);
		stashRemovePointer(locksByName, data->name, NULL);
		SAFE_FREE(data);
	}
}