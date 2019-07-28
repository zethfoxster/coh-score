/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_WORKSHOP_H
#define CHARACTER_WORKSHOP_H

#include "stdtypes.h"

typedef struct Character Character;
typedef struct RecipeItem RecipeItem; 
typedef struct Entity Entity;
typedef struct ConceptItem ConceptItem;
typedef struct Packet Packet;
typedef struct PowerVar PowerVar;

// ------------------------------------------------------------
// defines

#define DETAILRECIPE_ID_PACKBITS 1

#define DETAILRECIPE_USE_RECIPE_LEVEL 255

//------------------------------------------------------------
//  The state that the player is in regards to workshop
//----------------------------------------------------------
typedef enum WorkshopState
{
	kWorkshopState_None,
	kWorkshopState_Invent,
	kWorkshopState_Updating,
	kWorkshopState_Error,
	// count
	kWorkshopState_Count	
} WorkshopState;

//------------------------------------------------------------
//  The type of workshop that the player is interacting with (static maps)
//----------------------------------------------------------
typedef enum WorkshopType
{
	kWorkshopType_None,
	kWorkshopType_Invention_Basic,
	kWorkshopType_Invention_Arcane,
	kWorkshopType_Invention_Tech,
	// count
	kWorkshopType_Count	

} WorkshopType; 


bool workshopstate_Valid( WorkshopState e );
char const *workshopstate_ToStr( WorkshopState s );


typedef struct Workshop
{
 	WorkshopState state;
	int iInvCreatedDetail; // where the last item went after finishing
	char *error; // EString
#if SERVER
	bool changed;
#endif
} Workshop;

//int character_WorkshopWorkshopTypeByName(char *name);
//char *character_WorkshopWorkshopNameByType(int type);

#if CLIENT
/*bool character_IsWorkshopping(Character *p);
		// is the character in workshop mode
bool character_WorkshopCanSendCmd(Character *p);
		// can the character execute a workshop command (if they are workshopping)

void character_WorkshopStart(Character *p);*/
bool character_WorkshopSendDetailRecipeBuild(Character *p, char const *nameDetailRecipe, int iLevel, int bUseCoupon);
bool character_WorkshopSendDetailRecipeBuy(Character *p, char const *nameDetailRecipe);
//void character_WorkshopStop(Character *p);
//void character_ReceiveWorkshopUpdate( Character *p, Packet *pak );
#endif // CLIENT

// ----------------------------------------
// NET

#if SERVER
//void character_WorkshopReceiveStart(Character *p, Packet *pak);
void character_WorkshopReceiveDetailRecipeBuild(Character *p, Packet *pak);
//void character_WorkshopReceiveStop(Character *p, Packet *pak);
//void character_SendWorkshopUpdate(Character *p, Packet *pak );
void character_WorkshopInterrupt(Entity *e);
#endif // SERVER

#endif //CHARACTER_WORKSHOP_H
