
#ifndef PETCOMMON_H
#define PETCOMMON_H

typedef enum PetStance
{
	kPetStance_Unknown, // Only for uninitialized stances, please don't use this!
	kPetStance_Passive,
	kPetStance_Defensive,
	kPetStance_Aggressive,
	kPetStance_Count,
} PetStance;

#define PET_STANCE_BITS	2

typedef enum PetAction
{
	kPetAction_Attack,
	kPetAction_Goto,
	kPetAction_Follow,
	kPetAction_Stay,
	kPetAction_Special,
	kPetAction_Dismiss,
	kPetAction_Count,
}PetAction;

#define PET_ACTION_BITS 3

#endif