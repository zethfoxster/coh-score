#ifndef ENTAIBEHAVIORSTRUCT_H
#define ENTAIBEHAVIORSTRUCT_H

#include "stdtypes.h"
#include "entityRef.h"
#include "entaivars.h"	// for AIActivity enum
#include "petCommon.h"
#include "scriptconstants.h"

typedef struct AIBehavior				AIBehavior;
typedef struct AIBParsedString			AIBParsedString;
typedef struct AIPowerInfo				AIPowerInfo;
typedef struct DoorAnimPoint			DoorAnimPoint;
typedef struct PatrolRoute				PatrolRoute;
typedef struct StructFunctionCall		StructFunctionCall;
#define TokenizerFunctionCall StructFunctionCall

// Data Structs
typedef struct AIBDAttackTarget
{
	EntityRef target;
	PRIORITY priority;
}AIBDAttackTarget;

typedef struct AIBDAttackableVolume
{
	Vec3 pos;
	char *name;
} AIBDAttackableVolume;

typedef struct AIBDAttackVolumes
{
	AIBDAttackableVolume **targetVolumes;
	F32 fSecondaryTimer, fAttackRecharge, fSlowScale;
}AIBDAttackVolumes;

typedef struct AIBDCallForHelp
{
	F32 radius;
	const char* target_name;
}AIBDCallForHelp;

typedef struct AIBDCameraScan
{
	U8 initted;
	bool sweepDir;
	int flags;
	F32 arc;
	F32 speed;
	Vec3 initialPyr;
}AIBDCameraScan;

typedef struct AIBDCombat
{
	U32 fromOverride		: 1;
	U32 neverForgetTargets	: 1;
}AIBDCombat;

typedef struct AIBDDisappearInStandardDoor
{
	EntityRef door;
	U32 timeOpenedDoor;
	U32 failedPathFindCount;

	U32 waitForDeath		: 1;
}AIBDDisappearInStandardDoor;

typedef struct AIBDDoNothing
{
	U8 turn	: 1;
}AIBDDoNothing;

typedef struct AIBDHideBehindEnt
{
	EntityRef			protectEnt;
	U32					HBEsearchdist;

	U32					HBEcheckedProximity;
	Vec3				enemyGeoAvg;
	bool				amScared;
}AIBDHideBehindEnt;

typedef enum AIBHostageScaredState{
	AIB_HOSTAGE_SCARED_INIT,
	AIB_HOSTAGE_SCARED_COWER,
	AIB_HOSTAGE_SCARED_RUN,
}AIBHostageState;

typedef struct AIBDHostageScared
{
	int					state;
	Entity*				whoScaredMe;

	struct {
		U32				beginState;
		U32				scared;
		U32				checkedProximity;
	} time;
}AIBDHostageScared;

typedef struct AIBDLoop
{
	char* estr;
	U32 times;
	TokenizerFunctionCall** stringFlags;
}AIBDLoop;

typedef struct AIBDMoveToPos
{
	Vec3 pos;
	F32 target_radius;
	F32	jumpSpeedMultiplier;
}AIBDMoveToPos;

typedef struct AIBDMoveToRandPoint
{
	Vec3 patrolPoint;

	U32 patrolPointFoundTime;
	U32 patrolPointValid	: 1;

	U32 groupMode			: 1;
}AIBDMoveToRandPoint;

typedef struct AIBDReturnToSpawn
{
	F32 target_radius;
	F32	jumpSpeedMultiplier;
	U8 turn					: 1;
	U8 atspawn				: 1;
}AIBDReturnToSpawn;

typedef struct AIBDPlayFX
{
	char* estr;
}AIBDPlayFX;

typedef struct AIBDPatrol
{
	PatrolRoute* route;
	Vec3* followPoint;
	U32 curPoint;
	F32 mySpeed;
	int targetType;
	U32 lastStuck;
	F32	turnRadius;
	int owner_id;
}AIBDPatrol;

typedef enum AIBPetState
{
	PET_STATE_ATTACK_LEASH,
	PET_STATE_ATTACK_FREELY,
	PET_STATE_BE_IN_AREA,
	PET_STATE_RESUMMON,
	PET_STATE_FOLLOW,
	PET_STATE_NONE,
	PET_STATE_USE_POWER,
}AIBPetState;

typedef struct AIBDPet
{
	AIBPetState state;
	AIBPetState oldState;

	PetAction action;
	PetStance stance;

	Vec3 gotoPos;
	EntityRef attackTarget;
	U32 specialPowerCount;
	char* specialPetPower;

	F32 followDistance;

	U32	lastCommandTime;

	U32 noTeleport : 1;
}AIBDPet;

typedef struct AIBDPriorityList
{
	AIActivity activity;
	AIPriorityManager*	manager;
	AIBehavior* scopeVar;
	char* mypl;

	unsigned int restartPL : 1;
	struct {
		unsigned int AlwaysGetHit				: 1;
		unsigned int DoNotFaceTarget			: 1;
		unsigned int DoNotGoToSleep				: 1;
		unsigned int DoNotMove					: 1;
		unsigned int FeelingFear				: 1;
		unsigned int FeelingConfidence			: 1;
		unsigned int FeelingLoyalty				: 1;
		unsigned int FeelingBothered			: 1;
		unsigned int FindTarget					: 1;
		unsigned int Flying						: 1;
		unsigned int Invincible					: 1;
		unsigned int Invisible					: 1;
		unsigned int LimitedSpeed				: 1;
		unsigned int OverridePerceptionRadius	: 1;
		unsigned int SpawnPosIsSelf				: 1;
		unsigned int TargetInstigator			: 1;
		unsigned int TeamHero					: 1;
		unsigned int TeamMonster				: 1;
		unsigned int TeamVillain				: 1;
		unsigned int Untargetable				: 1;
		unsigned int HasPatrolPoint				: 1;
	}sets;
	struct {
		int AlwaysGetHit;
		int DoNotFaceTarget;
		int DoNotGoToSleep;
		int DoNotMove;
		int FeelingFear;
		int FeelingConfidence;
		int FeelingLoyalty;
		int FeelingBothered;
		int FindTarget;
		int Flying;
		int Invincible;
		int Invisible;
		int LimitedSpeed;
		int OverridePerceptionRadius;
		int SpawnPosIsSelf;
		int TargetInstigator;
		int TeamHero;
		int TeamMonster;
		int TeamVillain;
		int Untargetable;
		int HasPatrolPoint;
	}values;
}AIBDPriorityList;

typedef struct AIBDRunAwayFar
{
	F32 radius;
}AIBDRunAwayFar;

typedef struct AIBDRunIntoDoor
{
	U32 timeLastFailedPathFind;
	U32 timeOfLastDoorFind;
}AIBDRunIntoDoor;

typedef struct AIBDRunThroughDoor
{
	DoorEntry* doorIn;
	DoorEntry* doorOut;
	EntityRef leaderRef;
	Vec3 centerpoint;
	U32 tries;
}AIBDRunThroughDoor;

typedef struct AIBDSay
{
	char* estr;
	int priority;
}AIBDSay;

typedef struct AIBDSetHealth
{
	F32 health;
}AIBDSetHealth;

typedef struct AIBDSetObjective
{
	char* estr;
	int failed;
}AIBDSetObjective;

typedef struct AIBDTest
{
	int test;
}AIBDTest;

typedef struct AIBTDTest
{
	int teamTest;
}AIBTDTest;

typedef enum AIBRunOutOfDoorState{
	AIB_RUN_OUT_WAITING_FOR_TEAM,
	AIB_RUN_OUT_WAITING_FOR_MY_TIME,
	AIB_RUN_OUT_WAITING_FOR_ANIMATION,
}AIBRunOutOfDoorState;

typedef struct AIBDRunOutOfDoor
{
	AIBRunOutOfDoorState state;
}AIBDRunOutOfDoor;

typedef enum AIBThankHeroState{
	AIB_THANK_HERO_GO_TO_HERO,
	AIB_THANK_HERO_THANK_HERO,
}AIBThankHeroState;

typedef struct AIBDThankHero
{
	int				state;
	EntityRef		hero;
	Entity*			door;
	Vec3			dest;
	U32				storedTime;
	char			thankStringHasEmotes;
}AIBDThankHero;

typedef struct AIBDUsePower
{
	AIPowerInfo* power;
	U32 timesUsedAtSetPower;
	U32 timesUsedIsSet	: 1;
	U32 toggleOff		: 1;
}AIBDUsePower;

typedef struct AIBDWanderAround
{
	F32 radius;
}AIBDWanderAround;

#endif
