/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef INVENTION_H
#define INVENTION_H

#include "stdtypes.h"


typedef struct Character Character;
typedef struct RecipeItem RecipeItem; 
typedef struct Entity Entity;
typedef struct ConceptItem ConceptItem;
typedef struct Packet Packet;
typedef struct PowerVar PowerVar;
 
//------------------------------------------------------------
//  These states are set by the client and the server as a means
// of communication.
// See: inventionstate_Str if update this
//----------------------------------------------------------
typedef enum InventionState
{
	kInventionState_None,      // no inventing going on.
	kInventionState_Started,  // when starting but no level chosen yet (so nothing on server)
	kInventionState_Invent, // server sets this state and sends it back to client
	kInventionState_Updating,  // client status changes being sent to server. response: Invent
	kInventionState_Finalizing,// client asks server to create the boost. response: Finished
	kInventionState_Finished,  // server sets this and sends it back to client. Identical to None except with final info about the invented item.
	kInventionState_Count
} InventionState;

bool inventionstate_Valid(InventionState state);
char *inventionstate_Str(InventionState s);
 
//------------------------------------------------------------
//  the state of an invention slot
// See: slotstate_Str if change this
//----------------------------------------------------------
typedef enum SlotState
{
	kSlotState_Empty,
	kSlotState_Slotted,       
	kSlotState_HardenSuccess, 
	kSlotState_HardenFailure,
	kSlotState_Count
} SlotState;
bool slotstate_Valid(SlotState state);
char *slotstate_Str(SlotState state);


typedef struct InventionSlot
{
	SlotState state;
 	ConceptItem **concepts; // the first of these has to be slottble, the rest affect the first.
} InventionSlot;
bool invention_SlotHardened(InventionSlot *s);



#define INVENTION_SLOTS_MAX_COUNT 10

typedef struct Invention
{
	InventionState state;
	int boostInvIdx;
	RecipeItem *recipe;
	InventionSlot slots[INVENTION_SLOTS_MAX_COUNT];

#if SERVER
	bool statusChange;
#endif // SERVER
} Invention;

// ----------------------------------------
// invention methods

void invention_ValsForVar(Invention *inv, const PowerVar *var, F32 *resActual, F32 *resPotential);
F32 invention_HardenChanceAdj(int charLevel, int hardenLevel);

#if CLIENT
bool character_InventingCanUpdate(Character *pchar);
bool character_InventingStart(Character *p, U32 recipeinv_idx);
bool character_InventingSelectLevel(Character *p, U32 inventionLevel);
bool character_InventingSlotConcept(Character *pchar, int iSlot, int iConceptInv );
bool character_InventingHardenConcept(Character *pchar, int iSlot);
bool character_InventingUnSlotConcept(Character *pchar, int iSlot);
bool character_InventingCancel(Character *p);
#endif // CLIENT

bool character_IsInventing(Character const *p);

// ----------------------------------------
// net

#if SERVER
void character_InventReceiveRecipeSelection(Character *p, Packet *pak);
void character_InventReceiveSlotConcept(Character *p, Packet *pak);
void character_InventReceiveUnSlot(Character *p, Packet *pak);
void character_InventReceiveHardenSlot(Character *p, Packet *pak);
void character_InventReceiveFinalize(Character * p, Packet *pak);
void character_InventReceiveCancel(Character *p, Packet *pak);
#endif // SERVER

#define INVENTION_RECIPEINVIDX_PACKBITS 1
#define INVENTION_INVENTIONLEVEL_PACKBITS 1
#define INVENTION_SLOTIDX_PACKBITS 1
#define INVENTION_CONCEPTINVIDX_PACKBITS 1


#if SERVER
void entity_SendInventionUpdate( Entity *e, Packet *p );
#elif CLIENT 
void entity_ReceiveInventionUpdate( Entity *e, Packet *p );
#endif // CLIENT


#endif //INVENTION_H
