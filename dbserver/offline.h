#ifndef _OFFLINE_H
#define _OFFLINE_H

#include "stdtypes.h"

//
// LSTL: 09/2009
// Enables code with supports the "-process_deletion_log" command line option
// Can be un-defined when the initial offline database has been fully populated from
// deletion log data.
//
#define OFFLINE_ENABLE_DELETION_LOG_IMPORT_CODE


// These enum values are also written to the players.idx file.
typedef enum
{
	kOfflineObjStatus_OFFLINE	= 0,	// Player is in the list because he has been put offline
	kOfflineObjStatus_RESTORED	= 1,	// Player is not offline. The offline storage is obsolete and could be deleted.
	kOfflineObjStatus_DELETED	= 2		// Player is in the list because he has been deleted
} tOfflineObjStatus;

typedef enum
{
	kOfflineRestore_SUCCESS = 0,
	kOfflineRestore_PLAYER_NOT_FOUND,			// can't find an entry for dude
	kOfflineRestore_DATA_FILE_UNKNOWN,			// we found an index entry, but don't know what file the data is in
	kOfflineRestore_NO_MATCHING_DATA			// tOfflineObjStatus of the offline data is wrong for the request
} tOfflineRestoreResult;

typedef struct
{
	int					auth_id;
	char				auth_name[32];	// may be empty string on old records
	int					db_id;
	tOfflineObjStatus	storage_status;
	U32					file_pos;
	char				name[32];
	U32					origin;
	U32					archetype;
	U32					fileKey;
	U16					level;
	U32					idxfile_pos;
	U32					dateStored;
} OfflinePlayer;

void offlineInitOnce( void );
void offlineUnusedPlayers( void );
int offlinePlayerList(int auth_id,char *auth_name,OfflinePlayer *list,int max_list,U32 *online_ids,int online_count);
int offlinePlayerFindByName(char *name);
int offlinePlayerMoveOffline(int id);
int offlinePlayerDelete(int auth_id,char *auth_name,int db_id);
tOfflineRestoreResult offlinePlayerRestoreOfflined(int auth_id,char *auth_name,int db_id);
tOfflineRestoreResult offlinePlayerRestoreDeleted(int auth_id,char* auth_name,char* char_name, int seqNbr, char* deletionDateMMYYYY /* MM/YYYY */ );
void offlinePlayerListDeleted(int auth_id,char *auth_name, char* deletionDateMMYYYY /* MM/YYYY */, 
								OfflinePlayer *list,int max_list, int* p_num_found, int* p_auth_id);
void *offlinePlayerTempLoad(int db_id);

#ifdef OFFLINE_ENABLE_DELETION_LOG_IMPORT_CODE
	void offlineProcessDeletionLogFiles(const char* szLogFileSpec /* can include wildcards */);
#endif

#endif // _OFFLINE_H

