/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_ANIMFX_CLIENT_H__
#define CHARACTER_ANIMFX_CLIENT_H__

#include "character_animfx_common.h"
#include "Color.h"

void character_SetAnimClientBits(struct Character *p, int *piList);
void character_SetAnimClientStanceBits(struct Character *p, int *piList);
unsigned int entity_PlayClientsideMaintainedFX(Entity *eSrc, Entity *eTarget, char *pchName, ColorPair uiTint, float fDelay, unsigned int iAttached, float fTimeOut, int iFlags);
void entity_KillClientsideMaintainedFX(Entity *eSrc, Entity *eTarget, char *pchName);


#endif /* #ifndef CHARACTER_ANIMFX_CLIENT_H__ */

/* End of File */

