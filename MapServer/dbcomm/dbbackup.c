
#include "dbbackup.h"
#include "net_structdefs.h"
#include "entity.h"
#include "sendToClient.h"
#include "language/langServerUtil.h"
#include "dbnamecache.h"
#include "earray.h"
#include "dbcomm.h"
#include "comm_backend.h"
#include "svr_base.h"
#include "timing.h"

//------------------------------------------------------------------------------------
// Cached Search Data ////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------

typedef struct DbBackupHeader
{
	U32 date;
	char info[256];	
} DbBackupHeader;

DbBackupHeader ** s_headers;

static int s_backup_id;

static void dbbackup_displaySearch( int db_id, int type )
{
	Entity *e = entFromDbId(db_id);
	if(e)
	{
		int i;
		char datestr[256];
		
		if( type == CONTAINER_ENTS )
		{
			conPrintf( e->client, localizedPrintf(e,"BackSearchResultHeader1", dbPlayerNameFromId(s_backup_id) ) );
			conPrintf( e->client, localizedPrintf(e,"BackSearchResultHeader2", dbPlayerNameFromId(s_backup_id) ) );
			conPrintf( e->client, localizedPrintf(e,"BackSearchResultHeader3", dbPlayerNameFromId(s_backup_id) ) );
			conPrintf( e->client, localizedPrintf(e,"BackSearchResultHeader4") );
		}
		else if( type == CONTAINER_SUPERGROUPS )
		{
			conPrintf( e->client, localizedPrintf(e,"BackSearchResultHeaderSG1", dbSupergroupNameFromId(s_backup_id) ) );
			conPrintf( e->client, localizedPrintf(e,"BackSearchResultHeaderSG2", dbSupergroupNameFromId(s_backup_id) ) );
			conPrintf( e->client, localizedPrintf(e,"BackSearchResultHeaderSG3", dbSupergroupNameFromId(s_backup_id) ) );
			conPrintf( e->client, localizedPrintf(e,"BackSearchResultHeaderSG4") );
		}

		conPrintf( e->client, "----------------------" );
		conPrintf( e->client, "----------------------" );

		for( i = 0; i < eaSize(&s_headers); i++ )
		{
			timerMakeLogDateStringFromSecondsSince2000(datestr, s_headers[i]->date);

			if( type == CONTAINER_ENTS )
				conPrintf( e->client, localizedPrintf(e,"BackSearchPlayerResult", i, datestr, s_headers[i]->info ) );
			else if( type == CONTAINER_SUPERGROUPS )
				conPrintf( e->client, localizedPrintf(e,"BackSearchPlayerResultSG", i, datestr, s_headers[i]->info ) );
			
			if( i && (i % 5) == 0 )
				conPrintf( e->client, "----------------------" );
		}
	}
}

//------------------------------------------------------------------------------------
// Deal with player commands /////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------
void dbbackup( ClientLink * client, char * name, int type )
{
	int id;
	if( type == CONTAINER_ENTS )
		id = dbPlayerIdFromName(name);
	if( type == CONTAINER_SUPERGROUPS )
		id = dbSupergroupIdFromName(name);

	if( id > 0 )
	{
		Packet * pak = pktCreateEx(&db_comm_link,DBCLIENT_BACKUP);
		pktSendBitsPack(pak,1,id);
		pktSendBitsPack(pak,1,type);
		pktSend(&pak,&db_comm_link);
		conPrintf( client, localizedPrintf(client->entity,"BackupPerformed") );
	}
	else
		conPrintf( client, localizedPrintf(client->entity,"BackupFailed") );
}

void dbbackup_search( ClientLink * client, char * name, int type )
{
	int id;

	if( type == CONTAINER_ENTS )
		id = dbPlayerIdFromName(name);
	if( type == CONTAINER_SUPERGROUPS )
		id = dbSupergroupIdFromName(name);
    
	 if( id > 0 )
	{
		Packet * pak = pktCreateEx(&db_comm_link,DBCLIENT_BACKUP_SEARCH);
		pktSendBitsPack(pak,1,id);
		pktSendBitsPack(pak,1,type);
		pktSendBitsPack(pak,1,client->entity->db_id);
		pktSend(&pak,&db_comm_link);
	}
}

void dbbackup_view( ClientLink * client, char * name, int idx, int type )
{
	int id;

	if( type == CONTAINER_ENTS )
		id = dbPlayerIdFromName(name);
	if( type == CONTAINER_SUPERGROUPS )
		id = dbSupergroupIdFromName(name);

	if( id == s_backup_id )
	{
		if( idx < 0 || idx >= eaSize(&s_headers) )
			conPrintf( client, clientPrintf(client, "BackSearchIdBound") );
		else
		{
			Packet * pak = pktCreateEx(&db_comm_link,DBCLIENT_BACKUP_VIEW);
			pktSendBitsPack(pak,1,s_backup_id);
			pktSendBitsPack(pak,1,type);
			pktSendBitsPack(pak,1,s_headers[idx]->date);
			pktSendBitsPack(pak,1,client->entity->db_id);
			pktSend(&pak,&db_comm_link);
		}
	}
	else
		conPrintf( client, clientPrintf(client, "BackupSearchNotCurrent") );

}

void dbbackup_apply( ClientLink * client, char * name, int idx, int type )
{
	int id;

	if( type == CONTAINER_ENTS )
		id = dbPlayerIdFromName(name);
	if( type == CONTAINER_SUPERGROUPS )
		id = dbSupergroupIdFromName(name);

	if( id == s_backup_id )
	{
		if( idx < 0 || idx >= eaSize(&s_headers) )
			conPrintf( client, clientPrintf(client, "BackSearchIdBound") );
		else
		{
			Packet * pak = pktCreateEx(&db_comm_link,DBCLIENT_BACKUP_APPLY);
			pktSendBitsPack(pak,1,s_backup_id);
			pktSendBitsPack(pak,1,type);
			pktSendBitsPack(pak,1,s_headers[idx]->date);
			pktSendBitsPack(pak,1,client->entity->db_id);
			pktSend(&pak,&db_comm_link);
		}
	}
	else
		conPrintf( client, clientPrintf(client, "BackupSearchNotCurrent") );

}

//------------------------------------------------------------------------------------
// Deal with dbserver responses //////////////////////////////////////////////////////
//------------------------------------------------------------------------------------

void handleDbBackupSearch(Packet *pak)
{
	int db_id = pktGetBitsPack( pak, 1 );
	int type = pktGetBitsPack( pak, 1 );
	s_backup_id = pktGetBitsPack( pak, 1 );

	if( !s_headers )
		eaCreate(&s_headers);
	eaClearEx(&s_headers, NULL);

	while( pktGetBits(pak,1) )
	{
		DbBackupHeader * dbbh = malloc(sizeof(DbBackupHeader));
		dbbh->date = pktGetBitsPack( pak, 1 );
		strcpy( dbbh->info, pktGetString(pak) );
		eaPush(&s_headers, dbbh);
	}
	dbbackup_displaySearch(db_id, type);
}

void handleDbBackupApply(Packet *pak)
{
	int db_id = pktGetBitsPack(pak, 1);
	int type = pktGetBitsPack( pak, 1 );
	int success = pktGetBits(pak, 1);
	Entity *e = entFromDbId(db_id);

	if( e )
	{
		if( success )
			conPrintf( e->client, localizedPrintf(e,"BackupAppySuccess") );
		else
			conPrintf( e->client, localizedPrintf(e,"BackupAppyFail") );
	}
	
}

void handleDbBackupView(Packet *pak)
{
	int db_id = pktGetBitsPack(pak,1);
	int type = pktGetBitsPack( pak, 1 );
	char *result = pktGetString(pak);
	Entity *e = entFromDbId(db_id);

	if(e)
		conPrintf( e->client, result );
}
