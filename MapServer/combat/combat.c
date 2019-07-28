#include "map.h"                    // for combatVsType()
#include "combat.h"

FriendFoeOrSelf friendFoeOrSelf( Entity *me, Entity *other )
{
	if( me == other )
		return SELF;

	switch( combatVsType() )
	{
		case PLAYER_VS_CRITTER:
			if( ENTTYPE(me) == ENTTYPE_PLAYER && ENTTYPE(other) == ENTTYPE_CRITTER )
				return FOE;
			else if( ENTTYPE(me) == ENTTYPE_CRITTER && ENTTYPE(other) != ENTTYPE_CRITTER )
				return FOE;
			else
				return FRIEND;
		break;

		// Both of these kinds are for arena use only.
		case PLAYER_VS_PLAYER:
			return FOE;
		break;

		case TEAM_VS_TEAM:
			//if( entOnMyTeam( me, other ) )
			//	return FRIEND;
			//else
				return FOE;
		break;
	}

	return FOE;
}
