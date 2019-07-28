#ifndef _COMBAT_H
#define _COMBAT_H

typedef struct Entity Entity;

typedef enum
{
	SELF,
	FRIEND,
	FOE,
}FriendFoeOrSelf;

FriendFoeOrSelf friendFoeOrSelf( Entity *me, Entity *other );

#endif