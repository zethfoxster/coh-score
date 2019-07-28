#ifndef __INCARNATE_H
#define __INCARNATE_H

#include "entity.h"
#include "powers.h"

///////////////////////////////////////////////////////////
// Incarnate Slots
///////////////////////////////////////////////////////////

typedef enum IncarnateSlot
{
	kIncarnateSlot_Alpha,
	kIncarnateSlot_Destiny,
	kIncarnateSlot_Genesis,
	kIncarnateSlot_Hybrid,
	kIncarnateSlot_Interface,
	kIncarnateSlot_Judgement,
	kIncarnateSlot_Lore,
	kIncarnateSlot_Mind,
	kIncarnateSlot_Vitae,
	kIncarnateSlot_Omega,
	kIncarnateSlot_Count,
} IncarnateSlot;

extern StaticDefineInt ParseIncarnateSlotNames[];

static char *incarnateEnableBadgeNames[] =
{
	"IncarnateAlphaSlot",
	"IncarnateDestinySlot",
	"IncarnateGenesisSlot",
	"IncarnateHybridSlot",
	"IncarnateInterfaceSlot",
	"IncarnateJudgementSlot",
	"IncarnateLoreSlot",
	"IncarnateMindSlot",
	"IncarnateVitaeSlot",
	"IncarnateOmegaSlot"
};

IncarnateSlot IncarnateSlot_getByInternalName(const char *name);

float IncarnateSlot_getActivatePercent(Entity *e, IncarnateSlot slot);
bool IncarnateSlot_isActivated(Entity *e, IncarnateSlot slot);

bool IsEntityIncarnate(Entity* e);

const BasePowerSet* IncarnateSlot_getBasePowerSet(IncarnateSlot slot);

const BasePower* Incarnate_getBasePower(IncarnateSlot slot, const char *name);
Power* Incarnate_getPower(Entity *e, IncarnateSlot slot, const char *name);

bool Incarnate_hasInInventory(Entity *e, IncarnateSlot slot, const char *name);

bool Incarnate_isEquipped(Entity *e, IncarnateSlot slot, const char *name);
Power* Incarnate_getEquipped(Entity *e, IncarnateSlot slot);

bool Incarnate_isEquippedByBasePower(Entity *e, const BasePower *pPow);

#endif //__INCARNATE_H