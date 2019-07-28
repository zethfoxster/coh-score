#ifndef UIPET_H
#define UIPET_H

#include "uiInclude.h"
#include "stdtypes.h"
#include "petCommon.h"

#define MAX_PETNAME_LENGTH 15

int petWindow(void);

void pet_Add( int dbid );
void pet_Remove( int dbid );
void pets_Clear(void);

void pet_select( int idx );
void pet_selectName( char * name );

bool entIsPet( Entity * e );

enum
{
	kPetCommandType_Current,
	kPetCommandType_Name,
	kPetCommandType_Power,
	kPetCommandType_All,
};

void pet_update(int svr_idx,PetStance stance, PetAction action);
void pet_command( char * cmd, char * target, int type );
void pet_selectCommand( int stance, int action, char * target, int command_type, Vec3 vec );
void pet_say( char * msg, char * target_Str, int command_type );
void pet_rename( char * name, char * target_Str, int command_type );

void pet_inspireByName(char * pchInsp, char *pchName);
void pet_inspireTarget(char * pchInsp);

void petoption_open();

typedef struct ContextMenu ContextMenu;
ContextMenu * interactPetMenu( void );
ContextMenu * interactSimplePetMenu( void );


bool playerIsMasterMind();
bool entIsMyHenchman(Entity *e);
bool entIsMyPet(Entity *e);
bool entIsPlayerPet(Entity *e, bool strict);
bool entIsMyTeamPet(Entity *e, bool strict);
bool entIsMyLeaguePet(Entity *e, bool strict);


#endif
