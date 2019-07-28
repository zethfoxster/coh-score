#ifndef __INCARNATE_SERVER_H
#define __INCARNATE_SERVER_H

#include "incarnate.h"
#include "entity.h"
#include "Proficiency.h"
///////////////////////////////////////////////////////////
// Incarnate Points
///////////////////////////////////////////////////////////
typedef struct IncarnateSlotInfo
{
	int slotUnlocked;
	char *badgeStat;
	char *clientMessage;
}IncarnateSlotInfo;
typedef struct IncarnateExpInfo
{
	char *incarnateXPType;
	IncarnateSlotInfo **incarnateSlotInfo;
}IncarnateExpInfo;

typedef struct IncarnateModifier
{
	float *incarnateExpModifier;
}IncarnateModifier;

typedef struct IncarnateMods
{
	IncarnateExpInfo **incarnateExpList;
	IncarnateModifier incarnateModifierList[kProfOriginType_Count];
}IncarnateMods;

extern SHARED_MEMORY IncarnateMods g_IncarnateMods;

void Incarnate_RewardPoints(Entity *e, int subtype, int pointCount);

typedef enum IncarnateEquipResult
{
	kIncarnateEquipResult_Success = 0,
	kIncarnateEquipResult_Failure = 1,
	kIncarnateEquipResult_FailureSlotCooldown = 2,
	kIncarnateEquipResult_FailurePowerRecharging = 3,
	kIncarnateEquipResult_FailureInCombat = 4,
} IncarnateEquipResult;

///////////////////////////////////////////////////////////
// Incarnate Slots
///////////////////////////////////////////////////////////

IncarnateSlot Incarnate_slotFromPower(const BasePower *ppow);
bool IncarnateSlot_isRecharging(Entity *e, IncarnateSlot slot);

int IncarnateSlot_activate(Entity *e, IncarnateSlot slot);
void IncarnateSlot_activateAll(Entity *e);

int IncarnateSlot_deactivate(Entity *e, IncarnateSlot slot);
void IncarnateSlot_deactivateAll(Entity *e);

void IncarnateSlot_debugPrint(Entity *e, ClientLink *client, IncarnateSlot slot);
void IncarnateSlot_debugPrintAll(Entity *e, ClientLink *client);

///////////////////////////////////////////////////////////
// Incarnate Abilities
///////////////////////////////////////////////////////////

int Incarnate_grant(Entity *e, IncarnateSlot slot, char *name);
void Incarnate_grantAll(Entity *e);
void Incarnate_grantAllBySlot(Entity *e, IncarnateSlot slot);

int Incarnate_revoke(Entity *e, IncarnateSlot slot, char *name);
void Incarnate_revokeAll(Entity *e);
void Incarnate_revokeAllBySlot(Entity *e, IncarnateSlot slot);

IncarnateEquipResult Incarnate_equip(Entity *e, IncarnateSlot slot, char *name);
IncarnateEquipResult Incarnate_equipForced(Entity *e, IncarnateSlot slot, char *name);

IncarnateEquipResult Incarnate_unequip(Entity *e, IncarnateSlot slot, char *name);
IncarnateEquipResult Incarnate_unequipForced(Entity *e, IncarnateSlot slot, char *name);
IncarnateEquipResult Incarnate_unequipBySlot(Entity *e, IncarnateSlot slot);
IncarnateEquipResult Incarnate_unequipBySlotForced(Entity *e, IncarnateSlot slot);
void Incarnate_unequipAll(Entity *e);

void SetCombatModShift(Entity *e, int levelShift);

#endif //__INCARNATE_SERVER_H