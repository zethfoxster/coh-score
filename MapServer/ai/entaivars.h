
#ifndef ENTAIVARS_H
#define ENTAIVARS_H

#include "entityRef.h"

typedef struct NavSearch			NavSearch;
typedef struct Beacon				Beacon;
typedef struct BeaconConnection		BeaconConnection;
typedef struct BeaconCurvePoint		BeaconCurvePoint;
typedef struct BeaconAvoidNode		BeaconAvoidNode;
typedef struct AIPriorityManager	AIPriorityManager;
typedef struct AIProxEntStatus		AIProxEntStatus;
typedef struct AIProxEntStatusTable	AIProxEntStatusTable;
typedef struct Entity				Entity;
typedef struct Power				Power;
typedef struct BasePower			BasePower;
typedef struct Array				Array;
typedef struct PatrolRoute			PatrolRoute;
typedef struct AIAvoidInstance		AIAvoidInstance;
typedef struct AIConfigTargetPref	AIConfigTargetPref;
typedef struct DoorAnimPoint		DoorAnimPoint;
typedef struct DoorAnimState		DoorAnimState;
typedef struct DoorEntry			DoorEntry;
typedef struct AIVarsBase			AIVarsBase;
typedef struct AITeamBase			AITeamBase;

#define AI_POINTLOG_LENGTH 100
#define AI_POINTLOGPOINT_TAG_LENGTH 50
#define AI_POS_HISTORY_LENGTH (1 << 7) // This has to be some power of 2 because that makes modding easier.
#define MAX_RUN_OUT_DOORS	4

typedef struct AIPointLogPoint {
	Vec3				pos;
	int					argb;
	char				tag[AI_POINTLOGPOINT_TAG_LENGTH + 1];
} AIPointLogPoint;

typedef struct AIPointLog {
	int					curPoint;
	AIPointLogPoint		points[AI_POINTLOG_LENGTH];
} AIPointLog;

#define AI_LOG_LENGTH 2000

typedef struct AILog {
	int				logTick;
	int				logTickColor;

	char			text[AI_LOG_LENGTH + 1];
	
	AIPointLog		pointLog;
} AILog;

typedef enum AIActivity {
	AI_ACTIVITY_NONE = 0,
	
	AI_ACTIVITY_COMBAT,
	AI_ACTIVITY_DO_ANIM,
	AI_ACTIVITY_FOLLOW_LEADER,
	AI_ACTIVITY_FOLLOW_ROUTE,
	AI_ACTIVITY_FROZEN_IN_PLACE,
	AI_ACTIVITY_HIDE_BEHIND_ENT,
	AI_ACTIVITY_NPC_HOSTAGE,
	AI_ACTIVITY_THANK_HERO,
	AI_ACTIVITY_PATROL,
	AI_ACTIVITY_RUN_AWAY,
	AI_ACTIVITY_RUN_INTO_DOOR,
	AI_ACTIVITY_RUN_OUT_OF_DOOR,
	AI_ACTIVITY_RUN_TO_TARGET,
	AI_ACTIVITY_STAND_STILL,
	AI_ACTIVITY_WANDER,

	AI_ACTIVITY_COUNT,
} AIActivity;

typedef struct AIPosHistoryPoint {
	Vec3	pos;
} AIPosHistoryPoint;

typedef union AIMessageParams {
	void*	ptrData;
	int		intData;
} AIMessageParams;

typedef enum{
	AI_MSG_ATTACK_TARGET_MOVED,
	AI_MSG_ATTACK_TARGET_IN_RANGE,
	AI_MSG_BAD_PATH,
	AI_MSG_FOLLOW_TARGET_REACHED,
	AI_MSG_PATH_BLOCKED,
	AI_MSG_PATH_FINISHED,
	AI_MSG_PATH_EARLY_BLOCK,

	AI_MSG_COUNT,
}AIMessage;

typedef enum AIMotionType {
	AI_MOTIONTYPE_NONE = 0,
	AI_MOTIONTYPE_STARTING,
	AI_MOTIONTYPE_TRANSITION,
	AI_MOTIONTYPE_GROUND,
	AI_MOTIONTYPE_JUMP,
	AI_MOTIONTYPE_FLY,
	AI_MOTIONTYPE_WIRE,
	AI_MOTIONTYPE_ENTERABLE,
	AI_MOTIONTYPE_COUNT,
} AIMotionType;

typedef enum AIFollowTargetType {
	AI_FOLLOWTARGET_NONE = 0,
	AI_FOLLOWTARGET_ENTITY,
	AI_FOLLOWTARGET_BEACON,
	AI_FOLLOWTARGET_POSITION,
	AI_FOLLOWTARGET_ENTERABLE,
	AI_FOLLOWTARGET_MOVING_POINT,
} AIFollowTargetType;

typedef enum AIFollowTargetReason {
	AI_FOLLOWTARGETREASON_NONE = 0,
	AI_FOLLOWTARGETREASON_ATTACK_TARGET,
	AI_FOLLOWTARGETREASON_ATTACK_TARGET_LASTPOS,
	AI_FOLLOWTARGETREASON_CIRCLECOMBAT,
	AI_FOLLOWTARGETREASON_ESCAPE,
	AI_FOLLOWTARGETREASON_SAVIOR,
	AI_FOLLOWTARGETREASON_SPAWNPOINT,
	AI_FOLLOWTARGETREASON_PATROLPOINT,
	AI_FOLLOWTARGETREASON_LEADER,
	AI_FOLLOWTARGETREASON_STUNNED,
	AI_FOLLOWTARGETREASON_AVOID,
	AI_FOLLOWTARGETREASON_ENTERABLE,
	AI_FOLLOWTARGETREASON_COWER,
	AI_FOLLOWTARGETREASON_BEHAVIOR,
	AI_FOLLOWTARGETREASON_MOVING_TO_HELP,
	AI_FOLLOWTARGETREASON_CREATOR,
} AIFollowTargetReason;

typedef struct AIFollowTargetInfo {
	AIFollowTargetType		type;
	AIFollowTargetReason	reason;
	const float*			pos;
	Vec3					storedPos;
	float					standoffDistance;
	float					jumpHeightOverride;
	Vec3					posWhenPathFound;	// Position of follow target when path was constructed.
	
	union {
		Entity*				entity;
		Beacon*				beacon;
		Vec3*				movingPoint;
	};

	U8						followTargetReached : 1;	// Signifies that the follow target was discarded because it was successfully reached
	U8						followTargetDiscarded : 1;	// Signifies that the follow target was discarded for miscellaneous reasons
} AIFollowTargetInfo;

typedef struct AIAttackerInfo {
	struct AIAttackerInfo*	next;
	struct AIAttackerInfo*	prev;
	
	Entity*					attacker;
} AIAttackerInfo;

typedef struct AIAttackerList {
	AIAttackerInfo*			head;
	AIAttackerInfo*			tail;
	
	U32						count;
} AIAttackerList;

typedef struct AIAttackTargetInfo {
	Entity*					entity;
	AIProxEntStatus*		status;				// My target's status.

	U32						hitCount;

	struct {
		U32					set;				// When I set my current attack target.
	} time;
} AIAttackTargetInfo;

typedef enum AIFeelingID {
	AI_FEELING_FEAR,
	AI_FEELING_CONFIDENCE,
	AI_FEELING_LOYALTY,
	AI_FEELING_BOTHERED,
	
	AI_FEELING_COUNT,
} AIFeelingID;

typedef struct AIFeelingModifier {
	struct AIFeelingModifier*	next;
	AIFeelingID					feelingID;
	
	struct {
		float					begin;
		float					end;
	} level;
	
	struct {
		U32						begin;
		U32						end;
	} time;
} AIFeelingModifier;

typedef struct AIFeelingLevels {
	float				level[AI_FEELING_COUNT];
} AIFeelingLevels;

typedef enum AIFeelingSet {
	AI_FEELINGSET_BASE,
	AI_FEELINGSET_PROXIMITY,
	AI_FEELINGSET_PERMANENT,
	AI_FEELINGSET_COUNT,
} AIFeelingSet;

typedef struct AIFeelingInfo {
	AIFeelingLevels		sets[AI_FEELINGSET_COUNT];
	AIFeelingLevels		total;
	AIFeelingLevels		mods;
	
	U32					modifierCheckTime;
	AIFeelingModifier*	modifiers;

	struct {
		float			meAttacked;
		float			friendAttacked;
	} scale;
	
	U32					trackFeelings : 1;
} AIFeelingInfo;

extern const char* aiFeelingName[AI_FEELING_COUNT];

typedef enum AIEventFlags {
	#define	CREATE_SET(set, lo, hi)																					\
		AI_EVENT_##set##_LO_BIT				= lo,																	\
		AI_EVENT_##set##_HI_BIT				= hi,																	\
		AI_EVENT_##set##_MASK				= ((1 << (AI_EVENT_##set##_HI_BIT - AI_EVENT_##set##_LO_BIT + 1)) - 1) << AI_EVENT_##set##_LO_BIT,
	
	#define CREATE_VALUE(set, value) value << AI_EVENT_##set##_LO_BIT
	
	// Type.
	
	CREATE_SET(TYPE, 0, 7)
	AI_EVENT_TYPE_ATTACKED				= CREATE_VALUE(TYPE, 0),
	AI_EVENT_TYPE_HEALED				= CREATE_VALUE(TYPE, 1),
	AI_EVENT_TYPE_MISSED				= CREATE_VALUE(TYPE, 2),
	AI_EVENT_TYPE_JOINED_MY_FIGHT		= CREATE_VALUE(TYPE, 3),

	// Target.
	
	CREATE_SET(TARGET, 8, 11)
	AI_EVENT_TARGET_ME					= CREATE_VALUE(TARGET, 0),
	AI_EVENT_TARGET_FRIEND				= CREATE_VALUE(TARGET, 1),
	AI_EVENT_TARGET_ENEMY				= CREATE_VALUE(TARGET, 2),
	
	// Source.
	
	CREATE_SET(SOURCE, 12, 15)
	AI_EVENT_SOURCE_ME					= CREATE_VALUE(SOURCE, 0),
	AI_EVENT_SOURCE_FRIEND				= CREATE_VALUE(SOURCE, 1),
	AI_EVENT_SOURCE_ENEMY				= CREATE_VALUE(SOURCE, 2),
	
	#undef CREATE_SET
	#undef CREATE_VALUE
} AIEventFlags;

#define AI_EVENT_SET_FLAG(flags, set, value) (flags) = ((flags) & (~AI_EVENT_##set##_MASK)) | AI_EVENT_##set##_##value
#define AI_EVENT_GET_FLAG(flags, set) ((flags) & AI_EVENT_##set##_MASK)

typedef struct AIEvent {
	struct AIEvent*	next;
	U32				flags;
	U32				timeStamp;
	
	// This is where the actual data is stored, specific to the AIEventId.
	union {
		struct {
			float	hp;
		} attack;
		
		struct {
			float	hp;
		} heal;
	};
} AIEvent;

#define AI_EVENT_HISTORY_SET_TIME		5			// Seconds between sets of events.
#define AI_EVENT_HISTORY_SET_COUNT		(1 << 4)	// # of sets of events to be stored, power of 2.
#define AI_EVENT_HISTORY_TOTAL_SECONDS	(AI_EVENT_HISTORY_SET_TIME * AI_EVENT_HISTORY_SET_COUNT)

typedef struct AIEventSet {
	int					eventCount;
	AIEvent*			events;
	
	#define DEFINE_DATA_SET(name,type) struct { type me, myFriend, enemy; } name

	DEFINE_DATA_SET(attackedCount, int);
	DEFINE_DATA_SET(wasMissedCount, int);
	DEFINE_DATA_SET(damageTaken, float);
	
	#undef DEFINE_DATA_SET
} AIEventSet;

typedef struct AIEventHistory {
	AIEventSet			eventSet[AI_EVENT_HISTORY_SET_COUNT];
	U32					curEventSet;
	U32					curEventSetStartTime;
	U32					lastWasDamagedTime;
	U32					lastDidDamageTime;
} AIEventHistory;

typedef struct AIBasePowerInfo {
	struct AIBasePowerInfo*	prev;
	struct AIBasePowerInfo*	next;
	
	U32						refCount;
	U32						inuseCount;	// How many teammates are currently using the power if it's a buff (should only be 0 or 1; only guaranteed to be correct if power.toggle.curBuff/curDebuff are used)

	U32						usedCount;
	
	const BasePower*				basePower;

	struct {
		U32					lastUsed;
		U32					lastCurrent;
		U32					lastFail;
		U32					lastStoppedUsing; // For toggles/buffs, when their usage stopped
	} time;
} AIBasePowerInfo;

typedef struct AIPowerInfo {
	struct AIPowerInfo*		next;

	Power*					power;
	
	AIBasePowerInfo*		teamInfo;
	
	U32						groupFlags;
	char**					allowedTargets;
	
	float					sourceRadius;
	
	U32						stanceID;
	
	U32						usedCount;
	
	struct {
		U32					lastUsed;
		U32					lastCurrent;
		U32					lastFail;
	} time;
	
	struct {
		U32					duration;
		U32					startTime;
	} doNotChoose;
	
	F32						critterOverridePreferenceMultiplier;
	F32						debugTargetCount;
	F32						debugPreference;

	U32						isAttack			: 1;
	U32						isAuto				: 1;
	U32						isToggle			: 1;
	U32						isNearGround		: 1; // This power can only be cast near the ground.
	U32						isSpecialPetPower	: 1; // If user is a pet, don't use this power unless your master tells you to.
	U32						neverChoose			: 1; // I never choose this power, a script or something has to make me use it.
	U32						allowMultipleBuffs	: 1; // Get around the "same team doesn't use same buff at same time" code.
	U32						doNotToggleOff		: 1; // If this is a toggle, don't toggle it off after 20 sec
} AIPowerInfo;

typedef enum AITeamRole {
	AI_TEAM_ROLE_NONE = 0,
	AI_TEAM_ROLE_ATTACKER_MELEE,
	AI_TEAM_ROLE_ATTACKER_RANGED,
	AI_TEAM_ROLE_HEALER,
	
	AI_TEAM_ROLE_COUNT,
} AITeamRole;

typedef struct AITeam AITeam;
typedef struct AITeamMemberInfo AITeamMemberInfo;

typedef struct AITeamMemberInfo {
	AITeamMemberInfo*			next;
	AITeamMemberInfo*			prev;
	AITeam*						team;
	Entity*						entity;
	
	struct {
		AITeamRole				role;
		AIProxEntStatus*		targetStatus;
	} orders;

	DoorAnimPoint*				myDoor;

	F32							distanceToCurTargetSQR;
	U8							assignTargetPriority;
	U32							runOutTime;
	
	// Bitfields.
	
	U32							alerted						: 1;
	U32							currentlyRunningOutOfDoor	: 1;
	U32							doNotShareInfoWithMembers	: 1;
	U32							doNotAssignToMelee			: 1;
	U32							doNotAssignToRanged			: 1;
	U32							underGriefedHPRatio			: 1;
	U32							waiting						: 1;
	U32							wantToRunOutOfDoor			: 1;
} AITeamMemberInfo;

typedef struct AITeam {
	AITeamBase*					base;
	struct {
		AITeamMemberInfo*		list;
		U32						totalCount;
		U32						aliveCount;
		U32						killedCount;
		U32						underGriefedHPCount;
	} members;

	F32							totalMaxHP;

	Vec3						spawnCenter;

	AIProxEntStatusTable*		proxEntStatusTable;
	
	struct {
		AIBasePowerInfo*		list;
	} power;
	
	struct {
		U32						lastThink;
		U32						lastSetRoles;
		U32						didAttack;
		U32						wasAttacked;
		U32						firstAttacked;
	} time;

	DoorAnimPoint*				runOutDoors[MAX_RUN_OUT_DOORS];
	U32							numDoors;

	PatrolRoute*				teamPatrolRoute;				// this is used to restart the patrol if the team switches routes... obselete if behavior team data ever gets finished
	Mat4						followPoint;					// to be moved when team behavior stuff is more set up
	F32							patrolSpeed;
	U32							numPatrollers;

	int							curPatrolPoint;
	S32							patrolDirection;				// current direction for PATROL_PINGPONG
	U32							finishedPatrol			: 1;	// finished the current patrol route (or went through at least one iteration
	U32							patrolTypeWire			: 1;	// path finding switch... wire ignores everything and goes in a straight line
	U32							patrolTypeWireComplex	: 1;	// a wire path that has curves
	U32							patrolTeamLeaderBlocked	: 1;
	
	U32							teamid					: 16;	// corresponds with MAX_AITEAM_ID
	U32							wasAttacked				: 1;
	U32							teamMemberWantsToRunOut	: 1;
} AITeam;

typedef enum AIBrainType {
	// Default brain doesn't do any custom behavior.
	
	AI_BRAIN_DEFAULT,

	// Custom but generic

	AI_BRAIN_CUSTOM_HEAL_ALWAYS,				// this brain will take every opportunity to heal a teammate

	// 5thColumn.
	
	AI_BRAIN_5THCOLUMN_FOG,
	AI_BRAIN_5THCOLUMN_FURY,
	AI_BRAIN_5THCOLUMN_NIGHT,
	AI_BRAIN_5THCOLUMN_VAMPYR,
	
	// Banished Pantheon

	AI_BRAIN_BANISHED_PANTHEON,	
	AI_BRAIN_BANISHED_PANTHEON_SORROW,
	AI_BRAIN_BANISHED_PANTHEON_LT,

	// Clockwork.
	
	AI_BRAIN_CLOCKWORK,

	// Circle of Thorns

	AI_BRAIN_COT_THE_HIGH,
	AI_BRAIN_COT_BEHEMOTH,
	AI_BRAIN_COT_CASTER,
	AI_BRAIN_COT_SPECTRES,

	// Devouring Earth

	AI_BRAIN_DEVOURING_EARTH_MINION,
	AI_BRAIN_DEVOURING_EARTH_LT,
	AI_BRAIN_DEVOURING_EARTH_BOSS,
	
	// Freakshow.
	
	AI_BRAIN_FREAKSHOW,
	AI_BRAIN_FREAKSHOW_TANK,
	
	// Hellions.
	
	AI_BRAIN_HELLIONS,
	
	// Hydra.
	
	AI_BRAIN_HYDRA_HEAD,
	AI_BRAIN_HYDRA_FORCEFIELD_GENERATOR,
	
	// Malta.
	
	AI_BRAIN_MALTA_HERCULES_TITAN,

	// Octopus.
	
	AI_BRAIN_OCTOPUS_HEAD,
	AI_BRAIN_OCTOPUS_TENTACLE,

	// Outcasts.
	
	AI_BRAIN_OUTCASTS,

	// Prisoners.
	
	AI_BRAIN_PRISONERS,
	
	// Rikti.
	
	AI_BRAIN_RIKTI_SUMMONER,
	AI_BRAIN_RIKTI_UXB,
	
	// Skulls.
	
	AI_BRAIN_SKULLS,
	
	// SkyRaiders.
	
	AI_BRAIN_SKYRAIDERS_ENGINEER,
	AI_BRAIN_SKYRAIDERS_PORTER,

	// Synapse,

	AI_BRAIN_SWITCH_TARGETS,
	
	// TheFamily.
	
	AI_BRAIN_THEFAMILY,
	
	// Thugs.
	
	AI_BRAIN_THUGS,
	
	// Trolls.
	
	AI_BRAIN_TROLLS,

	// Tsoo.

	AI_BRAIN_TSOO_MINION,
	AI_BRAIN_TSOO_HEALER,

	// Vahzilok.

	AI_BRAIN_VAHZILOK_ZOMBIE,
	AI_BRAIN_VAHZILOK_REAPER,
	AI_BRAIN_VAHZILOK_BOSS,
	
	// Warriors.
	
	AI_BRAIN_WARRIORS,

	// Arachnos

	AI_BRAIN_ARACHNOS_WEB_TOWER,
	AI_BRAIN_ARACHNOS_REPAIRMAN,
	AI_BRAIN_ARACHNOS_DR_AEON,
	AI_BRAIN_ARACHNOS_MAKO,
	AI_BRAIN_ARACHNOS_GHOST_WIDOW,
	AI_BRAIN_ARACHNOS_BLACK_SCORPION,
	AI_BRAIN_ARACHNOS_SCIROCCO,
	AI_BRAIN_ARACHNOS_LORD_RECLUSE_STF,

	AI_BRAIN_ARCHITECT_ALLY,

	// End with the count.
	
	AI_BRAIN_COUNT,
} AIBrainType;

typedef enum {
	DES_NONE,
	DES_MOVING_TO_TARGET,
	DES_WAITING_FOR_PET,
	DES_HAS_PET,
} DevouringEarthState;

extern const char* aiBrainTypeName[AI_BRAIN_COUNT];

typedef struct AIVars{
	AIVarsBase*	base;
	struct{
		U32				waitForThink				: 1;// Don't do anything until I can think again.
		U32				firstThinkTick				: 1;// This gets set to 0 at the end of aiCritterThink, so can be used to specifically change behavior for the first tick.

		U32				doNotRun					: 1;
		U32				doNotMove					: 1;
		
		U32				magicallyKnowTargetPos		: 1;// This is set if I've seen my target in the last x seconds.

		U32				canSeeTarget				: 1;
		U32				canHitTarget				: 1;

		U32				waitingForTargetToMove		: 1;// Flag indicating whether to wait for movement of the target.
		U32				targetWasInRange			: 1;// Flag indicating whether my target was recently in range.
		U32				shortcutWasTaken			: 1;// Says whether the path was short-cutted
		U32				preventShortcut				: 1;// Prevents short-circuiting if we got stuck when attempting it.
		U32				sendEarlyBlock				: 1;// Sends an ai msg whenever a failed tick move occurs

		U32				useEnterable				: 1;// Let pathfinding use cluster connections to reach followtarget.
		U32				pathLimited					: 1;// Flag to signify whether this critter should be limited to only beacons/sub-blocks with pathLimitedAllowed set when pathfinding.
		U32				startedPathLimited			: 1;// Flag to override the AIConfig on whether to turn on PathLimited so that Josh doesn't have to go back and duplicate all of his spawndefs. Make sure this gets turned off when the "out of my volume" code gets triggered.
		
		U32				findTargetInProximity		: 1;// Checked when looking at nearby entities.
		
		U32				isScared					: 1;// Flag for NPCs who are scared of critters.
		U32				wasScared					: 1;// Flag to initialized scared state.
		U32				facingScarer				: 1;// Flag for facing the guy who scared me.

		U32				canFly						: 1;// Flag for if I can fly (set if attrCur.fFly > 0).
		U32				alwaysFly					: 1;// Flag for if I should always fly.
		U32				isFlying					: 1;// Flag for if I'm currently flying.
		U32				forceGetUp					: 1;// Flag for forcing APEX+AIR after being knocked back.

		U32				setForwardBit				: 1;// Flag for setting the forward animation bit.
		
		U32				isAfraidFromAI				: 1;// If I'm currently afraid because the AI thinks I should be.
		U32				isAfraidFromPowers			: 1;// If I'm currently afraid because the powers syste says I am.
		U32				isTerrorized				: 1;// If I'm currently terrorized.
		U32				isStunned					: 1;// If I'm currently stunned.

		U32				isGriefed					: 1;// Flag for when I'm being griefed.
		
		U32				doNotAggroCritters			: 1;// Flag for keeping critters from attacking this entity
		
		U32				doNotUsePowers				: 1;// DEBUG: Flag for disabling attack.  Rest of AI stays the same.
		U32				doNotChangePowers			: 1;// DEBUG: Flag for disabling power-changing.
		
		U32				doNotChangeAttackTarget		: 1;// Prevent changing targets.
		U32				stickyTarget				: 1;// Make it harder for to switch targets a lot when you don't have a team.
		
		U32				isResurrectable				: 1;// Allow others to resurrect me.
		
		U32				preferMelee					: 1;
		U32				preferRanged				: 1;
		U32				preferGroundCombat			: 1;// Prefer to land before fighting
		
		U32				preferUntargeted			: 1;
		
		U32				ignoreFriendsAttacked		: 1;
		U32				ignoreTargetStealthRadius	: 1;
		U32				relentless					: 1;
		U32				considerEnemyLevel			: 1;

		U32				keepSpawnPYR				: 1;
		U32				turnWhileIdle				: 1;
		U32				spawnPosIsSelf				: 1;
		
		U32				doAttackAI					: 1;
		U32				wasAttacked					: 1;
		U32				alwaysAttack				: 1; // Ignore the bothered feeling stuff and always attack

		U32				hasNextPatrolPoint			: 1;

		U32				doNotGoToSleep				: 1;
		
		U32				amPatrolLeader				: 1;
		U32				pitchWhenTurning			: 1; // Let aiTurnBody also change the pitch

		U32				doorEntryOnly				: 1;	// For running out of doors... only uses doors with DoorEntries so the doors will be usable for running into also
		U32				attackTargetsNotAttackedByJustAnyone : 1; // Allowed to attack ents with flag "notAttackedByJustAnyone"

		U32				isCamera					: 1;
		U32				wasToldToUseSpecialPetPower : 1; // I'm a pet and I have been told to use my special pet power, so it's OK to choose it

		U32				alwaysReturnToSpawn			: 1;	//	If true, always go home; if false, once combat ends, just stop

		U32				amBodyGuard					: 1; // This pet is in the defensive/passive stance so he'll take some of his owner's damage
		
		U32				perceiveAttacksWithoutDamage	: 1; // Sends notifies to an entity even if the magnitude of the power is 0
	};

	struct
	{
		U32				doNotRunIsSet				: 1;
		U32				doNotRun					: 1;
		U32				flyingIsSet					: 1;
		U32				flying						: 1;

		F32				limitedSpeed;
		F32				boostSpeed;				//Percent increase to base speed
		F32				standoffDistance;
	} inputs;
	
	void*				activityData;
	U32					activityDataAllocated;
	void*				msgHandler;

	U32					eventMemoryDuration;
	F32					targetedSightAddRange;
	F32					preferMeleeScale;
	F32					preferRangedScale;
	F32					returnToSpawnDistance;
	F32					leashToSpawnDistance;
	bool				vulnerableWhileLeashing;
	F32					protectSpawnRadius;
	F32					protectSpawnOutsideRadius;
	U32					protectSpawnDuration;
	F32					walkToSpawnRadius;
	
	struct {
		F32				angle;
		F32				angleCosine;
	} fov;
	
	struct {
		AIBrainType		type;
		const char*		configName;

		// AI-specific stuff.
		
		union {
			struct {
				int				weaponGroup;
				AIPowerInfo*	attackWeapon;
			} fifthColumn;

			struct {
				U32	retries;
				U32	clockwise:1;
			} banishedpantheon;

			struct {
				AIProxEntStatus* statusPrimary;
				U32	countWithoutPrimary;
			} cotbehemoth;

			struct {
				struct {
					U32 ranAwayRoot;
				} time;
				AIPowerInfo* curRoot;
			} cotcaster;

			struct {
				struct {
					U32 nextThinkAboutPet;
				} time;
				F32 savedHealth; // To start new plant at the same HP
				EntityRef pet;
				Vec3 target;
				DevouringEarthState state;
				AIPowerInfo *curPlant;
			} devouringearth;
			
			struct {
				AIPowerInfo*	forceField;
				struct {
					U32	foundGenerators;
				} time;
				U32				forceFieldOn : 1;
				U32				useTeam : 1;
			} hydraHead;

			struct {
				AIPowerInfo*	forceField;
				U32				forceFieldOn : 1;
			} octopusHead;

			struct {
				U32				lastTargetSwitch;
				EntityRef		target;

				F32				baseTime;
				F32				minDistance;

				U32				teleport : 1;
			} switchTargets;

			struct {
				F32				perceptionDistance;
			} makoPatron;

			struct {
				U32				deathTime;

				U32				initialized : 1;
				U32				fullHealth : 1;
			} rikti;
		};
	} brain;

	AIConfigTargetPref**	targetPref;

	// My share of the total timeslice.
	
	U32					timeslice;
	
	// Where was I spawned.

	struct {
		Vec3			pos;
		Vec3			pyr;
	} spawn;

	// Movement stuff.
	
	struct {
		AIMotionType	type;
		Vec3			source;
		Vec3			target;
		F32				minHeight;
		F32				maxHeight;
		
		struct {
			Vec3		lastTickDestPos;			// Position I want to be in on the next frame.
			F32			expectedTickMoveDist;		// How far I was from lastTickDestPos on previous frame.
			U32			notOnPath : 1;
			U32			waitingForDoorAnim : 1;		// Waiting for my running out of door anim to finish
		} ground;	

		struct {
			U32			notOnPath : 1;
		} fly;
		
		struct {
			F32			time;
			F32			superJumpSpeedMultiplier;
		} jump;

		U32				failedPathWhileFalling : 1;
		U32				superJumping : 1;
		
	} motion;

	AIFollowTargetInfo	followTarget;
	F32					curSpeed;						// This is the current speed.  It will accelerate up to max speed.
	F32					FollowDist;						// for spacing the wire following...
	F32					turnSpeed;						// for limiting turn speed

	int					failedMoveCount;				// Count of how many times I failed to reach my frame destination.
	
	F32					collisionRadius;				// Radius of my collision volume.

	DoorAnimState*		doorAnim;						// state for path finding door animation
	DoorEntry*			disappearInDoor;				// place to store the door that critters want to run into before path creation

	union{
		// Car stuff.
		struct{
			BeaconCurvePoint*	curveArray;
			int					curvePoint;				// The point in the current curve that the car is headed towards.
			F32					pitch;
			F32					distanceFromSourceToTarget;
			int					beaconTypeNumber;
		} car;
		
		// NPC stuff.
		struct{
			Beacon*				lastWireBeacon;			// The last beacon I was headed for in wire movement mode.
			Beacon*				antiIslandBeacon;		// An arbitrary beacon that I passed through used for checking if I'm connected to a big enough cluster of NPC beacons
			Vec3				wireTargetPos;			// The point I'm making my wire path to
			F32					distanceToTarget;		// Distance until I hit the target I'm headed towards.
			F32					groundOffset;			// Distance to be above the ground.
			S32					scaredType;				// This is the way an NPC will react to nearby critters.
			U8					antiIslandTimer;		// A count of how many beacons I've passed through since I set my antiIslandBeacon
			U8					antiIslandCounter;		// A count of how many times I've passed through my antiIslandBeacon
			U8					wireWanderMode	: 1;
			U8					runningBehavior	: 1;	// This is a flag to tell NPCs to skip some of their normal code because they're running a behavior
			U8					doNotCheckForIsland : 1;// This flag tells NPCs to skip the island checks because they've already checked and were in a valid place
		} npc;
		
		// Door stuff.
		struct{
			U32					timeOpened;
			U32					open : 1;
			U32					autoOpen : 1;
			U32					walkThrough : 1;
			U32					notDisappearInDoor : 1; // Flag to show this door has been "run into" and can't be used for running away
		} door;
	};

	F32					accThink;						// Think accumulator.
	
	int					timerTryingToSleep;				// Think ticks until I should fall asleep.

	struct{
		U32				foundPath;						// When I last found a path.
		U32				checkedForShortcut;				// When I checked for a shortcut.
		U32				checkedLOS;						// When I checked line of sight.
		U32				checkedProximity;				// When I checked my proximity.
		U32				alertedNearbyHelpers;			// When I last alerted nearby helpers.
		U32				beginCanSeeState;				// When my can-see state changed.
		U32				chosePower;						// When I chose a power.
		U32				setCurrentStance;				// When I set my current power stance.
		U32				setCurrentPower;				// When I armed my current power.
		U32				lastInRange;					// When I was last in attack range.
		U32				lookedForTeam;					// When I last looked for a team.
		U32				didAttack;						// When I last attacked.
		U32				wasAttacked;					// When I was last attacked.
		U32				wasAttackedMelee;				// When I was last attacked within melee range (<9ft)
		U32				friendWasAttacked;				// When either me or my friends were last attacked
		U32				wasDamaged;						// When I was last damaged.
		U32				setActivity;					// When I set my current activity.
		U32				firstAttacked;					// When I was first attacked.
		U32				fixedStuckPosition;				// When I last freed myself from a stuck position.
		U32				assignedPatrolPoint;			// When I was assigned my most recent patrol point
		U32				lastHadTarget;					// When I last had an attack target
		
		struct {
			U32			afraidState;					// When I started my current afraid state.
			U32			stunnedState;					// When I started my current stunned state.
			U32			griefedState;					// When I started my current griefed state.
		} begin;
		
		struct{
			U32			begin;							// When I started wandering on current wandering path.
		} wander;
	} time;
	
	struct 
	{
		F32 fSingleTargetPreference;
		F32 fAOEPreference;
	} AOEUsage;

	NavSearch*			navSearch;						// Path searcher.
	Beacon*				dbgLastClusterConnectSource;	// Keep track of your last cluster connection for debugging purposes.
	Beacon*				dbgLastClusterConnectTarget;

	int					nearbyPlayerCount;
	
	// Proximity Entity Status Stuff.
	
	AIProxEntStatusTable*		proxEntStatusTable;
	AIProxEntStatus*			proxEntStatusList;

	F32					overridePerceptionRadius;		// Perception radius override for use by priority lists
	
	Vec3				pyr;							// Entity body orientation in PYR
	
	Vec3				anchorPos;						// Position used for wandering around and for running away to a certain distance.
	F32					anchorRadius;					// Radius I should maintain from my anchor position.
	
	Entity*				whoScaredMe;					// Entity that scared me.

	EntityRef			protectEnt;						// Entref for the script to specify what entity to use for behaviors
	F32					HBEsearchdist;					// HideBehindEnt range controlled by the script

	// Feelings, nothing but feelings.
	
	AIFeelingInfo		feeling;
	
	// Event history.
	
	AIEventHistory		eventHistory;
	
	// Team.
	
	AITeamMemberInfo	teamMemberInfo;
	
	// Attack target stuff.

	AIAttackTargetInfo	attackTarget;
	
	Entity*				helpTarget;

	AIAttackerInfo		attackerInfo;
	AIAttackerList		attackerList;

	// Shout stuff.

	F32					shoutRadius;
	int					shoutChance;
	
	// Power stuff.

	struct {
		AIPowerInfo*		list;
		AIPowerInfo*		lastInfo;
		U32					count;

		AIPowerInfo*		current;
		AIPowerInfo*		preferred;
		
		U32					nextAvailableStanceID;
		U32					lastStanceID;

		// General use stuff, interpret as you may.

		struct {
			AIPowerInfo*	curBuff;
			AIPowerInfo*	curDebuff;
			AIPowerInfo*	curPaced;				// For Early/Mid/EndBattle flagged toggle powers
			int				curPacedPriority;		// Priority of current toggle (Early=low, End=high)
		} toggle;
		
		struct {
			AIPowerInfo*	curBuff;
			AIPowerInfo*	curDebuff;
		} click;
		
		struct {
			U32				buff;
			U32				debuff;
		} time;
		
		int groupFlags;								// Set of all groupFlags on all possessed powers
		U32 dirty					: 1;			// groupflags etc need to be reinitialized
		U32 useRangeAsMaximum		: 1;			// Specified power ranges are maximum ranges
		U32 resurrectArchvillains	: 1;			// Allows picking of archvillains as resurrect targets

		int numPets;								// The number of pets this guy has... used to limit pet powers
		int maxPets;								// The maximum number of pets this guy is allowed to have and still cast pgPet powers
	} power;

	int						originalAllyID;			// The AllyID I started with when I spawned

	struct {
		Vec3				stayPos;

		U32					useStayPos : 1;
	} pet;
	
	// Avoid stuff.
	
	struct {
		AIAvoidInstance*	list;
		F32					maxRadius;
		
		struct {
			Vec3				pos;
			F32					maxRadius;
			BeaconAvoidNode*	nodes;
		} placed;
	} avoid;		
	
	// Griefed variables.
	
	struct {
		struct {
			U32				begin;
			U32				end;
		} time;
		
		F32					hpRatio;
	} griefed;
	
	// Log variables.

	AILog*				log;

	// IAmA:/IReactTo: lists

	char**				IAmA;
	char**				IReactTo;
} AIVars;

#endif
