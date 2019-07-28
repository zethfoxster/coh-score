/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_TICK_H__
#define CHARACTER_TICK_H__

typedef struct Character Character;
typedef struct CharacterAttributes CharacterAttributes;
typedef struct Power Power;

void character_TickPhaseOne(Character *p, float fRate);
void character_HandleDeathAndResurrection(Character *p, CharacterAttributes *attrLast);
void character_TickPhaseTwo(Character *p, float fRate);
void character_OffTick(Character *p, float fRate);

int character_NamedTeleport(Character *p, Power *originatingPower, const char *pDest);

#endif /* #ifndef CHARACTER_TICK_H__ */

/* End of File */

