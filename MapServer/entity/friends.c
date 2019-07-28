
#include "entity.h"
#include "friends.h"
#include "friendCommon.h"
#include "svr_chat.h"
#include "dbquery.h"
#include "dbcontainer.h"
#include "svr_base.h"
#include "comm_game.h"
#include "language/langServerUtil.h"
#include "svr_player.h"
#include "dbcomm.h"
#include "character_base.h"
#include "origins.h"
#include "classes.h"
#include "utils.h"
#include "StringCache.h"
#include "staticMapInfo.h"
#include "entPlayer.h"
#include "dbnamecache.h"
#include "comm_backend.h"

void friendReadDBData(Packet *in_pak,int db_id,int count)
{
	int i, dbid;
	char className[32], originName[32];
	Entity *e = entFromDbIdEvenSleeping( db_id );

	if( !e )
		return;

	for( i = 0; i < count; i++ )
	{
		dbid = pktGetBitsPack(in_pak,1);
		strncpyt( className, pktGetString(in_pak), 32);
		strncpyt( originName, pktGetString(in_pak), 32);
	}

	for( i = 0; i < e->friendCount; i++ )
	{
		if( e->friends[i].dbid == dbid )
		{
			e->friends[i].arch = allocAddString(className);
			e->friends[i].origin = allocAddString(originName);
		}
	}
}

//
//
int updateFriendStatus( Entity *e, Friend* f )
{
	OnlinePlayerInfo *opi = dbGetOnlinePlayerInfo(f->dbid);
	Entity *ent = entFromDbId( f->dbid );
	int changed = 0;
	char *name =  dbPlayerNameFromId(f->dbid);
	
	// make sure they have a valid name filled in
 	if( !f->name )
	{		
		f->name = malloc( sizeof(char)*64 );	

		if( name )
			strcpy( f->name, name );
		else
			strcpy( f->name, "empty" );
	}

	//check for rename
 	if( name && stricmp(f->name, name ) != 0 )
	{
		free(f->name);
		f->name = malloc( sizeof(char)*64 );

		if( name )
			strcpy( f->name, name );
		else
			strcpy( f->name, "empty" );
	}

	if( ent && (!ent->pl->hidden || e->access_level > 0) )
	{
		// make sure map and map name is current
		if( e->map_id != f->map_id || !f->mapname )
		{
			strncpyta( &f->mapname, getTranslatedMapName(e->map_id), 256 );
			f->map_id = e->map_id;
			f->static_info = staticMapInfoFind(f->map_id);
			f->send = changed = TRUE;
		}

		if( f->online == FALSE ) // if they weren't online before, send notification to player
		{
			f->online = TRUE;
			f->send = changed = TRUE;
			chatSendToPlayer( e->db_id, localizedPrintf(e,"FriendIsOnline", f->name) , INFO_SVR_COM, 0 );
		}
	}
    else if( opi && (!(opi->hidefield&(1<<HIDE_FRIENDS)) || e->access_level > 0) ) // friend is online, make sure data is up to date
	{
		// make sure map and map name is current
		if( opi->map_id != f->map_id || !f->mapname )
		{
			strncpyta( &f->mapname, getTranslatedMapName(opi->map_id), 256 );
			f->map_id = opi->map_id;
			f->static_info = staticMapInfoFind(f->map_id);
			f->send = changed = TRUE;
		}

		if( f->online == FALSE ) // if they weren't online before, send notification to player
		{
			f->online = TRUE;
			f->send = changed = TRUE;
			chatSendToPlayer( e->db_id, localizedPrintf(e,"FriendIsOnline", f->name), INFO_SVR_COM, 0 );
		}
	}
	else
	{
		if( f->online == TRUE )
			f->send = changed = TRUE;

		f->online = FALSE;
	}

	if( !f->arch || !f->origin )
	{
		Entity *ent = entFromDbId( f->dbid );

		if( ent )
		{
			f->arch = allocAddString(ent->pchar->pclass->pchName);
			f->origin = allocAddString(ent->pchar->porigin->pchName);
		}
		else
		{
			char match[200];
			sprintf(match,"WHERE ContainerId = %d",f->dbid);
			dbReqCustomData(CONTAINER_ENTS, "Ents",0, match, "ContainerId, Class, Origin", friendReadDBData, e->db_id );
		}
	}

	if( stricmp( f->name, "empty" ) == 0 )
	{
		removeFriend( e, f->name );
	}

	return changed;
}


void updateFriendList( Entity *e)
{
	int i, update = 0;
	for( i = 0; i < e->friendCount; i++ )
 	{
		update |= updateFriendStatus( e, &e->friends[i] );
	}

	if( update && e->ready )
	{
		START_PACKET( pak1, e, SERVER_SEND_FRIENDS );
		sendFriendsList( e, pak1, 0 );
		END_PACKET
	}
}

void updateAllFriendLists(void)
{
	int i;
	PlayerEnts	*players = &player_ents[PLAYER_ENTS_ACTIVE];

	for(i=0;i<players->count;i++)
	{
		Entity      *e = players->ents[i];
		updateFriendList(e);
	}
}


//
//
void sendFriendsList( Entity *e, Packet *pak, int send_all )
{
	int i;

	//JE/JS: Do *not* call updateFriendStatus in here, since it can senda chat message int
	// the middle of this packet! Bad!

	// is this a full update?
	pktSendBitsPack( pak, 1, send_all );

	// send down how many friends we will send
	pktSendBitsPack( pak, 1, e->friendCount );

	for( i = 0; i < e->friendCount; i++ )
	{
		if( send_all || e->friends[i].send )
		{
			e->friends[i].send = 0;
			pktSendBits( pak, 1 ,1 );
			pktSendBitsPack( pak, 1, e->friends[i].dbid );
			pktSendBits( pak, 1, e->friends[i].online );

			pktSendString( pak, e->friends[i].name);

			if( e->friends[i].arch )
				pktSendBitsPack( pak, 1, classes_GetIndexFromName(&g_CharacterClasses, e->friends[i].arch ) );
			else
				pktSendBitsPack( pak, 1, -1);

			if( e->friends[i].origin )
				pktSendBitsPack( pak, 1, origins_GetIndexFromName(&g_CharacterOrigins, e->friends[i].origin) );
			else
				pktSendBitsPack( pak, 1, -1);

			if ( e->friends[i].online )
			{
				pktSendBitsPack( pak, 1, e->friends[i].map_id );

				// This should already be updated.
				pktSendString( pak, e->friends[i].mapname );
			}
		}
		else
			pktSendBits( pak, 1, 0 );
	}
}

int isFriend( Entity * e, int dbid )
{
	int i;
	for( i = 0; i < e->friendCount; i++)
	{
		if( dbid == e->friends[i].dbid )
			return TRUE;
	}

	return FALSE;
}
//
//
void addFriend( Entity *e, char *name )
{
	int dbid = dbPlayerIdFromName(name);
	char *realName = dbPlayerNameFromId(dbid);

	if( strlen(name) >= 32 )
	{
		char tmp[32];
		strncpy( tmp, name, 31 );
		chatSendToPlayer( e->db_id, localizedPrintf(e, "playerNotFound", tmp ), INFO_USER_ERROR, 0 );
		return;
	}
		

	if( e->friendCount >= MAX_FRIENDS )
	{
		chatSendToPlayer( e->db_id, localizedPrintf(e,"MaxFriends" ), INFO_USER_ERROR, 0 );
		return;
	}

	if( dbid > 0 )
	{
		if( dbid != e->db_id )
		{
			if( !isFriend( e, dbid ) )
			{
				e->friends[e->friendCount].dbid = dbid;
				e->friends[e->friendCount].name = malloc( sizeof(char)*64 );
				strcpy( e->friends[e->friendCount].name,  realName);
				e->friends[e->friendCount].online = TRUE;
				chatSendToPlayer( e->db_id, localizedPrintf(e,"FriendAdded", realName), INFO_SVR_COM, 0 );

				updateFriendStatus( e, &e->friends[e->friendCount] );
				e->friendCount++;
			}
			else
				chatSendToPlayer( e->db_id, localizedPrintf(e,"AlreadyAFriend", realName), INFO_USER_ERROR, 0 );
		}
		else
			chatSendToPlayer( e->db_id, localizedPrintf(e,"YouAreAlwaysAFriend"), INFO_USER_ERROR, 0 );
	}
	else
		chatSendToPlayer( e->db_id, localizedPrintf(e,"playerNotFound", name), INFO_USER_ERROR, 0 );


	e->resend_hp = TRUE;
	updateFriendList(e);
	START_PACKET( pak1, e, SERVER_SEND_FRIENDS );
	sendFriendsList( e, pak1, 1 );
	END_PACKET
}

//
//
void removeFriend( Entity *e, char *name )
{
	int i;

	for( i = 0; i < e->friendCount; i++ )
	{
		if ( !e->friends[i].name )
		{
			dbLog("friend", e, "Null friend in friend list for %s(%d)", e->name, e->db_id);
			continue;
		}
		else if( stricmp( e->friends[i].name, name ) == 0 )
		{
			if( stricmp( name, "empty") != 0 )
				chatSendToPlayer( e->db_id, localizedPrintf(e,"FriendRemoved", name), INFO_SVR_COM, 0 );

			friendDestroy(&e->friends[i]);
			memcpy( &e->friends[i], &e->friends[i+1], sizeof(Friend) * (e->friendCount - (i + 1)) );
			e->friendCount--;
			memset(&e->friends[e->friendCount], 0, sizeof(Friend)); // clear old one

			e->resend_hp = TRUE;
			updateFriendList(e);
			START_PACKET( pak1, e, SERVER_SEND_FRIENDS );
			sendFriendsList( e, pak1, 1 );
			END_PACKET

			return;
		}
	}

	chatSendToPlayer( e->db_id, localizedPrintf(e,"NotInFriendsList", name), INFO_USER_ERROR, 0 );
}

void displayFriends( Entity *e )
{
	int i;
	char * msg;

	if( e->friendCount == 0 )
	{
		chatSendToPlayer( e->db_id, localizedPrintf(e,"NoFriends"), INFO_SVR_COM, 0 );
		return;
	}
	else
	{
		chatSendToPlayer( e->db_id, localizedPrintf(e,"YourFriendsAre"), INFO_SVR_COM, 0 );
	}

 	for(i = 0; i < e->friendCount; i++ )
	{
		updateFriendStatus( e, &e->friends[i] );

		if( e->friends[i].online )
			msg = localizedPrintf(e, "FriendOnline", e->friends[i].name, e->friends[i].mapname );
		else
			msg = localizedPrintf(e, "FriendOffline", e->friends[i].name );

		chatSendToPlayer( e->db_id, msg, INFO_SVR_COM, 0 );
	}
}

