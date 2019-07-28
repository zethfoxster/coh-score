/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_PET_H__
#define CHARACTER_PET_H__

typedef struct AttribMod AttribMod;
void character_MakePetsUnaggressive(Character *p);
void character_CreatePet(Character *p, AttribMod *pmod);
void character_CreatePseudoPet(Character *p, AttribMod *pmod);
void character_DestroyPet(Character *p, AttribMod *pmod);
void character_DestroyAllPets(Character *p); // doesn't destroy ALL pets, only ones that have EntCreate attrib mods on the character...
void character_DestroyAllPetsEvenWithoutAttribMod(Character *p);
void character_DestroyAllPetsAtDeath(Character *p);
void character_DestroyAllFollowerPets(Character *p);
void character_DestroyAllPendingPets(Character *p);
void character_PetClearParentMods(Character *p);
Character *character_GetOwner(Character *p);
Entity * character_CreateArenaPet( Entity * eCreator, char * petVillainDef, int level );
void character_DestroyPetByEnt(Entity *eCreator, Entity *ePet);
void character_DestroyAllArenaPets( Character * pchar );
void character_PetSlotPower(Character *p, Power *ppow);
void character_BecomePet(Character *p, Entity *eNewOwner, AttribMod *pmod);
int character_IsAFollowerPet(Character *p, Entity * pet );

void character_AllPetsRunOutOfDoor(Character *p, Vec3 position, int xfer);
void character_AllPetsRunIntoDoor(Character *p);
void character_UpdateAllPetAllyGang(Character *p);
void character_PetsUpgraded(Character *p, int upgrade);
void character_ApplyPetUpgrade(Character *p);

void PlayerAddPet(Entity * eCreator, Entity *pet);
void PlayerRemovePet(Entity * eCreator, Entity *pet);

#endif /* #ifndef CHARACTER_PET_H__ */

/* End of File */

