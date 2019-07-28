
#ifndef PLAYERSTICKY_H
#define PLAYERSTICKY_H

#include "stdtypes.h"

typedef struct Entity Entity;
typedef struct GroupDef GroupDef;
typedef struct Packet Packet;

void playerToggleFollowMode(void);
void playerStartForcedFollow(Packet *pak);
void playerSetFollowMode(Entity* entity, Mat4 matTarget, GroupDef *defTarget, int follow, int activateTarget);
void playerStopFollowMode(void);
void playerDoFollowMode(void);
int playerIsFollowing(Entity *entity);

#endif