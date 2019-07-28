#include "chatter.h"
#include "clientcomm.h"
#include "stdio.h"
#include "dooranimclient.h"
#include "textClientInclude.h"

TCStats stats;

int randInt2(int max);


#include "entity.h"
#include "netio.h"
#include "player.h"
#include "entVarUpdate.h"
#include "uiCursor.h"

void CodeChatNPC(int svr_idx)
{
	START_INPUT_PACKET(pak, CLIENTINP_TOUCH_NPC );
	pktSendBitsPack(pak, PKT_BITS_TO_REP_SVR_IDX, svr_idx);
	END_INPUT_PACKET
}

int testChatNPC(void)
{
	int		i;
	Entity	*e;
	float	dist;
	int		ret=0;

	for( i=1; i<=entities_max; i++)
	{
		if (!(entity_state[i] & ENTITY_IN_USE ) )
			continue;

		e = entities[i];

		dist = distance3Squared( ENTPOS(playerPtr()), ENTPOS(e) );
		if ( dist < 15*15 ) 
		{
			CodeChatNPC(e->svr_idx);
			ret++;
			if( e && (ENTTYPE(e) == ENTTYPE_DOOR) )
			{
				enterDoorClient( ENTPOS(e), 0 );
			}
		}
	}
	return ret;
}

/*void enterDoor( Vec3 pos )
{
	setWaitForDoorMode();	
	pktSendBitsPack( input_pak, 1, CLIENTINP_ENTER_DOOR );
	pktSendF32( input_pak, pos[0] );
	pktSendF32( input_pak, pos[1] );
	pktSendF32( input_pak, pos[2] );
}
*/
