#include "mapgroup.h"
#include "container/dbcontainerpack.h"
#include "comm_backend.h"
#include "dbcomm.h"
#include "scriptengine.h"
#include "StringCache.h"

MapGroup	g_mapgroup;
int			g_mapgroup_id;
int			g_mapgroup_id_initialized = false;

//---------------------------------------------------------------------------------
// Map Data Tokens
//---------------------------------------------------------------------------------
MapDataToken* createMapDataToken(void)
{
	return calloc(1, sizeof(MapDataToken));
}


LineDesc mapdata_token_line_desc[] =
{
	{{ PACKTYPE_STR_UTF8,		SIZEOF2(MapDataToken, name),	"Name",		OFFSET(MapDataToken, name) },
		"TODO"},
	{{ PACKTYPE_INT,		SIZE_INT32,						"Value",	OFFSET(MapDataToken, val) },
		"TODO"},
	{ 0 },
};

StructDesc mapdata_token_desc[] =
{
	sizeof(MapDataToken),
	{AT_EARRAY, OFFSET(MapGroup, data )},
	mapdata_token_line_desc,
	"TODO"
};

LineDesc mapgroup_line_desc[] =
{
	{{ PACKTYPE_STR_UTF8,			SIZEOF2(MapGroup, name),			"Name",				OFFSET(MapGroup, name)							},
		"TODO"},
	{{ PACKTYPE_STR_UTF8,			SIZEOF2(MapGroup, activeEvent),		"ActiveEvent",		OFFSET(MapGroup, activeEvent)					},
		"TODO"},
	{{ PACKTYPE_STR_UTF8,			SIZEOF2(MapGroup, eventSignal),		"EventSignal",		OFFSET(MapGroup, eventSignal)					},
		"TODO"},
	{{ PACKTYPE_INT,			SIZE_INT32,							"EventSignalTS",	OFFSET(MapGroup, eventSignalTS)					},
		"TODO"},
	{ PACKTYPE_EARRAY,			(int)createMapDataToken,			"MapDataTokens",	(int) mapdata_token_desc						},
	{ 0 },
};

StructDesc mapgroup_desc[] =
{
	sizeof(MapGroup),
	{AT_NOT_ARRAY,{0}},
	mapgroup_line_desc,
	"TODO"
};

static char *packageMapGroup(MapGroup *map_group)
{
	char* result = dbContainerPackage(mapgroup_desc,(char*)map_group);
	return result;
}

void containerHandleMapGroup(ContainerInfo *ci)
{
	eaDestroyEx(&g_mapgroup.data, NULL);
	memset(&g_mapgroup, 0, sizeof(g_mapgroup)); // stupid unpack code
	dbContainerUnpack(mapgroup_desc,ci->data,(char*)&g_mapgroup);
	ScriptHandleMapGroup();
}

char *mapGroupTemplate()
{
	return dbContainerTemplate(mapgroup_desc);
}

char *mapGroupSchema()
{
	return dbContainerSchema(mapgroup_desc, "MapGroups");
}

void mapGroupLock()
{
	int		ret;

	ret = dbSyncContainerRequest(CONTAINER_MAPGROUPS,g_mapgroup_id,CONTAINER_CMD_LOCK_AND_LOAD,0);
	assert(ret);
}

void mapGroupUpdateUnlock()
{
	char	*str = packageMapGroup(&g_mapgroup);
	dbSyncContainerUpdate(CONTAINER_MAPGROUPS,g_mapgroup_id,CONTAINER_CMD_UNLOCK,str);
	free(str);
}

void mapGroupInit(char *unique_name)
{
	static PerformanceInfo* perfInfo;
	int		ret,type=CONTAINER_MAPGROUPS;
	char	*data;

	// Don't try if we aren't connected to a dbserver...
	if (db_comm_link.type == NLT_NONE)
		return;

	g_mapgroup_id_initialized = true;

	g_mapgroup_id = dbSyncContainerFindByElement(CONTAINER_MAPGROUPS,"Name",unique_name,0,1);
	dbLog("MapGroupInit",NULL,"dbSyncContainerFindByElement() #1 returned %d, map id %d", g_mapgroup_id, db_state.map_id);	// DGNOTE - debugging
	if (g_mapgroup_id <= 0)
	{
		g_mapgroup_id = dbSyncContainerFindByElement(CONTAINER_MAPGROUPS,"Name",unique_name,0,0);
		dbLog("MapGroupInit",NULL,"dbSyncContainerFindByElement() #2 returned %d, map id %d", g_mapgroup_id, db_state.map_id);	// DGNOTE - debugging
		if (g_mapgroup_id > 0)
		{
			mapGroupLock();
			mapGroupUpdateUnlock();
		}
		else
		{
			strcpy(g_mapgroup.name,unique_name);
			data = packageMapGroup(&g_mapgroup);
			dbAsyncContainerUpdate(type,-1,CONTAINER_CMD_CREATE,data,g_mapgroup_id);
			if (!dbMessageScanUntil("CONTAINER_CMD_CREATE", &perfInfo))
				assert(0);
			g_mapgroup_id = db_state.last_id_acked;
			dbLog("MapGroupInit",NULL,"CONTAINER_CMD_CREATE OK, g_mapgroup_id is %d, map id %d", g_mapgroup_id, db_state.map_id);	// DGNOTE - debugging
		}
	}
	ret = dbContainerAddDelMembers(type,1,1,g_mapgroup_id,1,&db_state.map_id,0);
	if (!ret)
	{
		dbLog("MapGroupInit",NULL,"dbContainerAddDelMembers() failed, g_mapgroup_id is %d, map id %d", g_mapgroup_id, db_state.map_id);	// DGNOTE - debugging
		assert(0 == "mapGroupInit()");
	}
	mapGroupUpdateUnlock();
}

void testmapgroups()
{
	mapGroupInit("AllStaticMaps");
	mapGroupLock();
	mapGroupUpdateUnlock();
	dbBroadcastMsg(CONTAINER_MAPGROUPS,&g_mapgroup_id, 101, 99, 1,"heya");
}


//Do I already have this MapData?
// do not persist the token!
MapDataToken * getMapDataToken(const char * pName)
{
	MapDataToken *retval = NULL;
	int count, i;

	count = eaSize(&g_mapgroup.data);
	for( i = 0 ; i < count ; i++ )
	{
		if( g_mapgroup.data[i]->name && 0 == stricmp( g_mapgroup.data[i]->name, pName ) )
		{
			retval = g_mapgroup.data[i];
			break;
		}
	}
	return retval;
}

// do not persist the token!
MapDataToken * giveMapDataToken(const char *pName)
{
	MapDataToken *retval = NULL;

	if( pName )
	{
		if ((retval = getMapDataToken(pName)) == NULL) 
		{
			retval = calloc(1,sizeof(MapDataToken)); //freed on eaDestroy
			strcpy(retval->name, pName );
			eaPush(&g_mapgroup.data, retval);
		}
	} 

	return retval;
}

void removeMapDataToken(const char *pName)
{
	int count, i;
	if( pName )
	{
		count = eaSize(&g_mapgroup.data);
		for( i = 0 ; i < count ; i++ )
		{
			if( 0 == stricmp( g_mapgroup.data[i]->name, pName ) )
			{
				void *token = NULL;

				token = eaRemove(&g_mapgroup.data, i );
				if (token)
					free(token);
				break;
			}
		}
	}
}

int mapGroupIsInitalized()
{
	return g_mapgroup_id_initialized;
}
