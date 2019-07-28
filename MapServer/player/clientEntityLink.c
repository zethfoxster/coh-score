#include "wininclude.h"
#include <stdio.h>
#include "assert.h"
#include "entserver.h"
#include "clientEntityLink.h"
#include "entVarUpdate.h"
#include "sendToClient.h"
#include "position.h"
#include "comm_game.h"
#include "clientEntityLink.h"
#include "svr_base.h"
#include "entity.h"

//
// MS: ClientLink functions for accessing fields of ClientLink.
//

void clientSetEntity(ClientLink* client, Entity* ent)
{
	assert(client && (!client->entity || client->entity == ent));

	client->entity = ent;
	
	if(ent){
		if (ent->client!=NULL && ent->client!=client) {
			assert(!"Eek!  This entity still has another client attached!");
			// JE: perhaps disconnect the old client?  I don't think this case has ever happened though
		}
		ent->client = client;

		if (ent->db_flags & DBFLAG_NOCOLL)
			client->controls.nocoll = 2;
		
		entUpdatePosInstantaneous(ent, ENTPOS(ent));

		if(clientPacketLogIsEnabled(ent->db_id)){
			client->link->packetSentCallback = clientPacketSentCallback;
		}		
	}
}

void clientSetSlave(ClientLink* client, Entity* ent, int control)
{
	assert(client && (!client->slave || !ent || client->slave == ent));

	client->slave = ent;
	client->slaveControl = control;
}

Entity* clientGetViewedEntity(ClientLink* client)
{
	assert(client);
	
	// If I have a slave, then I'm viewing the slave, otherwise I'm viewing myself.
	
	return client->slave ? client->slave : client->entity;
}

Entity* clientGetControlledEntity(ClientLink* client)
{
	assert(client);
	
	if(client->slave){
		// I have a slave, so see if I control him.  If I don't control him then I don't control anyone.
		
		return client->slaveControl ? client->slave : NULL;
	}else{
		if(client->entity && client->entity->myMaster && client->entity->myMaster->slaveControl){
			// I have a master, and he's controlling me, so I control nothing.
			
			return NULL;
		}else{
			// I don't have a master, or I do but he's not controlling me.

			return client->entity;
		}
	}
}

int clientIsSlave(ClientLink* client)
{
	assert(client && client->entity);
	
	return client->entity->myMaster != NULL;
}

Entity* clientGetSlave(ClientLink* client)
{
	assert(client);
	
	return client->slave;
}

ClientLink* clientGetMaster(ClientLink* client)
{
	assert(client && client->entity);
	
	return client->entity->myMaster;
}

void freePlayerControl( ClientLink* client )
{
	Entity* slave = clientGetSlave(client);
	
	if(slave){
		START_PACKET( pak, client->entity, SERVER_CONTROL_PLAYER );
		
		// Send a bit indicating that player control should be disabled.
		
		pktSendBits( pak, 1, 0 );	// 0 = master
		pktSendBits( pak, 1, 0 );	// 0 = disable

		END_PACKET

		// Send a packet to the slave telling him he can move again.
		
		START_PACKET( pak, slave, SERVER_CONTROL_PLAYER );
		
		pktSendBits( pak, 1, 1 );	// 1 = slave
		pktSendBits( pak, 1, 0 );	// 0 = disable
	
		END_PACKET

		slave->myMaster = NULL;

		clientSetSlave(client, NULL, 0);
	}
}

void getPlayerControl( ClientLink* client, int control )
{
	int slaveId = client->controls.selected_ent_server_index;
	Entity* slave = validEntFromId(slaveId);
	
	if(!slave || ENTTYPE(slave) != ENTTYPE_PLAYER){
		conPrintf(client, "ERROR: No player selected.\n");
	}
	else if(slave->client && slave->client->slave){
		// Slave doesn't need to have a client.  Can be in Quitting count-down mode.
		
		conPrintf(client, "ERROR: Selected player is currently modding another player.\n");
	}
	else if(client->entity == slave){
		conPrintf(client, "ERROR: You are already modding selected player.\n");
	}
	else if(client->entity->myMaster){
		conPrintf(client, "ERROR: You are being modded.\n");
	}
	else if(slave->myMaster){
		conPrintf(client, "ERROR: Selected player is being modded.\n");
	}
	else{
		freePlayerControl(client);
	
		clientSetSlave(client, slave, control);

		slave->myMaster = client;
		
		// Send a packet to the controlling player telling him that he is controlling the slave now.
		
		START_PACKET( pak, client->entity, SERVER_CONTROL_PLAYER );
		
		pktSendBits( pak, 1, 0 );				// 0 = master
		pktSendBits( pak, 1, 1 );				// 1 = enable
		pktSendBits( pak, 1, control ? 1 : 0 );	// 1 = full control, 0 = just viewing
		
		pktSendBitsPack( pak, 1, slaveId );
		
		sendCharacterToClient( pak, slave );
		
		END_PACKET

		if(control){
			// Send a packet to the slave telling him he has lost control of his character.
			
			START_PACKET( pak, slave, SERVER_CONTROL_PLAYER );
			
			pktSendBits( pak, 1, 1 );	// 1 = slave
			pktSendBits( pak, 1, 1 );	// 1 = enable
		
			END_PACKET
			
			// Update the pos_id for slave.
			
			entUpdatePosInstantaneous(slave, ENTPOS(slave));
		}
	}
}

