#ifndef _NAMECACHE_H
#define _NAMECACHE_H

#include "stdtypes.h"

void pnameInit();
char *pnameFindById(int db_id);
int pnameFindByName(const char *name);

// use escaped strings for all these
void playerNameLogin(char* name, int db_id, int just_created, U32 authId);
void playerNameDelete(char* name, int db_id);
int playerNameValid(char* name, int* isempty, U32 authId);
void playerNameCreate(char* name,int db_id, U32 auth_id);
void pnameAddAll();
void pnameAdd(const char *name, int db_id);
void playerNameClearCacheEntry(char *name,int db_id);

// The entity catalog contains supplemental info for all known entities,
// where active or not.
typedef struct EntCatalogInfo
{
	int			id;
	U32			authId;
	U8			gender;
	U8			nameGender;
	U8			playerType;
	U8			playerSubType;
	U8			playerTypeByLocation;
	U8			praetorianProgress;
	char		primarySet[64];
	char		secondarySet[64];
} EntCatalogInfo;

typedef struct EntCon EntCon;

EntCatalogInfo	*entCatalogGet( int db_id, bool bAddIfNotFound );	// may return NULL if !bAddIfNotFound
void			entCatalogDelete( int db_id );
void			entCatalogPopulate( int db_id, EntCon *ent_container );

typedef struct TempLockedName
{
	char name[32];
	U32 authId;
	U32 time;
} TempLockedName;

int tempLockAdd( char * name, U32 authId, U32 time );
int tempLockFindbyName( char * name );
int tempLockCheckAccountandRefresh( char * name, U32 authId, U32 time );
void tempLockTick(U32 timeout, U32 time);
void tempLockRemovebyName( const char * name );

#endif
