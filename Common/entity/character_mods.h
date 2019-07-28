/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_MODS_H__
#define CHARACTER_MODS_H__

typedef struct Character Character;
typedef struct Power Power;

void character_AccrueMods(Character *p, float fRate, bool *pbDisallowDefeat);
void character_AccrueBoosts(Character *p, Power *ppow, float fRate);
void character_RemoveSleepMods(Character *p);
void character_HandleConditionalFXMods(Character *p);

#endif /* #ifndef CHARACTER_MODS_H__ */

/* End of File */

